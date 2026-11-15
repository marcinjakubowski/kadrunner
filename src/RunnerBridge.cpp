#include "RunnerBridge.h"
#include "HistoryModel.h"

#include <QGuiApplication>
#include <QMargins>
#include <QModelIndex>
#include <QQmlComponent>
#include <QQuickWindow>
#include <QScreen>

#include <KRunner/QueryMatch>
#include <KRunner/ResultsModel>
#include <KRunner/RunnerManager>
#include <LayerShellQt/Window>

#include <algorithm>
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

    m_window = win;
    applyLayerGeometry();
    m_window->show();
    m_window->raise();
    m_window->requestActivate();
    emit shown();
}

void RunnerBridge::applyLayerGeometry()
{
    if (!m_window) {
        return;
    }
    auto *layerWin = LayerShellQt::Window::get(m_window);
    if (!layerWin) {
        return;
    }
    layerWin->setWantsToBeOnActiveScreen(true);
    layerWin->setLayer(LayerShellQt::Window::LayerOverlay);
    layerWin->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityExclusive);
    layerWin->setExclusiveZone(-1);
    layerWin->setScope(QStringLiteral("kadrunner"));

    if (m_history->posX() < 0) {
        layerWin->setAnchors(LayerShellQt::Window::AnchorTop);
        layerWin->setMargins(QMargins(0, m_history->posY(), 0, 0));
    } else {
        layerWin->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorTop | LayerShellQt::Window::AnchorLeft));
        layerWin->setMargins(QMargins(m_history->posX(), m_history->posY(), 0, 0));
    }
}

void RunnerBridge::moveBy(int dx, int dy)
{
    if (!m_window || (dx == 0 && dy == 0)) {
        return;
    }
    QScreen *scr = m_window->screen();
    if (!scr) {
        scr = QGuiApplication::primaryScreen();
    }
    const QRect bounds = scr ? scr->geometry() : QRect(0, 0, 2560, 1440);

    int currentX = m_history->posX();
    if (currentX < 0) {
        currentX = std::max(0, (bounds.width() - m_window->width()) / 2);
    }
    const int maxX = std::max(0, bounds.width() - m_window->width());
    const int maxY = std::max(0, bounds.height() - m_window->height());

    m_history->setPosX(std::clamp(currentX + dx, 0, maxX));
    m_history->setPosY(std::clamp(m_history->posY() + dy, 0, maxY));
    applyLayerGeometry();
}

void RunnerBridge::resetPosition()
{
    m_history->setPosX(-1);
    m_history->setPosY(220);
    if (m_window) {
        applyLayerGeometry();
    }
}

int RunnerBridge::activeScreenWidth() const
{
    QScreen *scr = m_window ? m_window->screen() : QGuiApplication::primaryScreen();
    if (!scr) {
        scr = QGuiApplication::primaryScreen();
    }
    return scr ? scr->geometry().width() : 2560;
}

int RunnerBridge::activeScreenHeight() const
{
    QScreen *scr = m_window ? m_window->screen() : QGuiApplication::primaryScreen();
    if (!scr) {
        scr = QGuiApplication::primaryScreen();
    }
    return scr ? scr->geometry().height() : 1440;
}

void RunnerBridge::setDesignMode(bool enabled)
{
    if (!m_window) {
        return;
    }
    auto *layerWin = LayerShellQt::Window::get(m_window);
    if (!layerWin) {
        return;
    }

    if (enabled) {
        if (m_history->posX() < 0) {
            const int centeredX = std::max(0, (activeScreenWidth() - m_history->launcherWidth()) / 2);
            m_history->setPosX(centeredX);
        }
        layerWin->setAnchors(LayerShellQt::Window::Anchors(
            LayerShellQt::Window::AnchorTop |
            LayerShellQt::Window::AnchorBottom |
            LayerShellQt::Window::AnchorLeft |
            LayerShellQt::Window::AnchorRight));
        layerWin->setMargins(QMargins(0, 0, 0, 0));
    } else {
        applyLayerGeometry();
    }
}

void RunnerBridge::toggle()
{
    if (m_window && m_window->isVisible()) {
        hide();
    } else {
        show();
    }
}
