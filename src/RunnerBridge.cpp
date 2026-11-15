#include "RunnerBridge.h"
#include "HistoryModel.h"

#include <QMargins>
#include <QModelIndex>
#include <QQmlComponent>
#include <QQuickWindow>

#include <KRunner/QueryMatch>
#include <KRunner/ResultsModel>
#include <KRunner/RunnerManager>
#include <LayerShellQt/Window>

#include <memory>

RunnerBridge::RunnerBridge(KRunner::ResultsModel *model,
                           HistoryModel *history,
                           QQmlComponent *windowComponent,
                           QObject *parent)
    : QObject(parent)
    , m_model(model)
    , m_history(history)
    , m_windowComponent(windowComponent)
{
}

bool RunnerBridge::runRow(int row)
{
    if (m_closing || row < 0 || row >= m_model->rowCount()) {
        return false;
    }
    const QModelIndex idx = m_model->index(row, 0);
    const KRunner::QueryMatch match = m_model->getQueryMatch(idx);
    m_history->recordMatch(match, m_model->queryString());
    const bool ok = m_model->run(idx);
    hide();
    return ok;
}

bool RunnerBridge::runHistoryRow(int row)
{
    if (m_closing) {
        return false;
    }
    const HistoryEntry *entry = m_history->entryAt(row);
    if (!entry) {
        return false;
    }
    const HistoryEntry target = *entry;
    m_history->incrementEntry(row);
    hide();

    auto *mgr = m_model->runnerManager();
    if (!mgr) {
        return false;
    }

    const QString queryTerm = target.query.isEmpty() ? target.title : target.query;
    auto matchConn = std::make_shared<QMetaObject::Connection>();
    auto finishConn = std::make_shared<QMetaObject::Connection>();

    *matchConn = connect(mgr, &KRunner::RunnerManager::matchesChanged, this,
                         [mgr, target, matchConn, finishConn](const QList<KRunner::QueryMatch> &matches) {
        for (const auto &m : matches) {
            if (m.id() == target.id) {
                QObject::disconnect(*matchConn);
                QObject::disconnect(*finishConn);
                mgr->run(m);
                return;
            }
        }
    });

    *finishConn = connect(mgr, &KRunner::RunnerManager::queryFinished, this,
                          [mgr, target, matchConn, finishConn]() {
        QObject::disconnect(*matchConn);
        QObject::disconnect(*finishConn);
        for (const auto &m : mgr->matches()) {
            if (m.id() == target.id || m.text() == target.title) {
                mgr->run(m);
                break;
            }
        }
    });

    mgr->launchQuery(queryTerm, target.runnerId);
    return true;
}

void RunnerBridge::hide()
{
    if (m_closing || !m_window) {
        return;
    }
    m_closing = true;
    QQuickWindow *win = m_window;
    m_window = nullptr;
    win->close();
    win->deleteLater();
    m_model->setQueryString(QString());
    m_model->clear();
    m_closing = false;
}

void RunnerBridge::show()
{
    if (m_window) {
        m_window->raise();
        m_window->requestActivate();
        emit shown();
        return;
    }

    QObject *obj = m_windowComponent->create();
    auto *win = qobject_cast<QQuickWindow *>(obj);
    if (!win) {
        delete obj;
        return;
    }

    if (auto *layerWin = LayerShellQt::Window::get(win)) {
        layerWin->setWantsToBeOnActiveScreen(true);
        layerWin->setLayer(LayerShellQt::Window::LayerOverlay);
        layerWin->setAnchors(LayerShellQt::Window::AnchorTop);
        layerWin->setMargins(QMargins(0, 220, 0, 0));
        layerWin->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityExclusive);
        layerWin->setExclusiveZone(-1);
        layerWin->setScope(QStringLiteral("kadrunner"));
    }

    m_window = win;
    m_window->show();
    m_window->raise();
    m_window->requestActivate();
    emit shown();
}

void RunnerBridge::toggle()
{
    if (m_window && m_window->isVisible()) {
        hide();
    } else {
        show();
    }
}
