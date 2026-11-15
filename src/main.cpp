#include <QAbstractListModel>
#include <QAction>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeySequence>
#include <QLocalServer>
#include <QLocalSocket>
#include <QModelIndex>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickWindow>
#include <QStandardPaths>
#include <QTimer>

#include <KConfigGroup>
#include <KGlobalAccel>
#include <KRunner/AbstractRunner>
#include <KRunner/QueryMatch>
#include <KRunner/ResultsModel>
#include <KRunner/RunnerManager>
#include <KSharedConfig>
#include <LayerShellQt/Window>

#include <algorithm>

namespace {
const QString kSocketName = QStringLiteral("kadrunner-ipc");

struct HistoryEntry {
    QString id;
    QString title;
    QString subtext;
    QString category;
    QString iconName;
    QString runnerId;
    QString query;
    int count = 0;
    qint64 lastUsed = 0;
};

class HistoryModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString mode READ mode WRITE setMode NOTIFY settingsChanged)
    Q_PROPERTY(int maxItems READ maxItems WRITE setMaxItems NOTIFY settingsChanged)

public:
    enum Roles {
        DecorationRole = Qt::DecorationRole,
        DisplayRole = Qt::DisplayRole,
        SubtextRole = Qt::UserRole + 1,
        CategoryRole = Qt::UserRole + 2,
    };

    explicit HistoryModel(QObject *parent = nullptr)
        : QAbstractListModel(parent)
    {
        const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataDir);
        m_dbPath = dataDir + QStringLiteral("/history.json");

        const QString cfgDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
        QDir().mkpath(cfgDir);
        m_cfgPath = cfgDir + QStringLiteral("/settings.json");

        loadSettings();
        loadHistory();
    }

    QString mode() const { return m_mode; }
    void setMode(const QString &mode)
    {
        if (m_mode == mode) {
            return;
        }
        m_mode = mode;
        beginResetModel();
        sortEntries();
        endResetModel();
        saveSettings();
        emit settingsChanged();
    }

    int maxItems() const { return m_maxItems; }
    void setMaxItems(int maxItems)
    {
        const int clamped = std::clamp(maxItems, 1, 10);
        if (m_maxItems == clamped) {
            return;
        }
        m_maxItems = clamped;
        beginResetModel();
        endResetModel();
        saveSettings();
        emit settingsChanged();
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid() || m_mode == QLatin1String("off")) {
            return 0;
        }
        return std::min(static_cast<int>(m_entries.size()), m_maxItems);
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!index.isValid() || index.row() < 0 || index.row() >= rowCount()) {
            return {};
        }
        const auto &e = m_entries[index.row()];
        switch (role) {
        case DisplayRole:
            return e.title;
        case DecorationRole:
            return e.iconName.isEmpty() ? QStringLiteral("application-x-executable") : e.iconName;
        case SubtextRole:
            return e.subtext;
        case CategoryRole:
            return e.category;
        default:
            return {};
        }
    }

    QHash<int, QByteArray> roleNames() const override
    {
        return {
            {DisplayRole, "display"},
            {DecorationRole, "decoration"},
            {SubtextRole, "subtext"},
            {CategoryRole, "category"},
        };
    }

    const HistoryEntry *entryAt(int row) const
    {
        if (row < 0 || row >= rowCount()) {
            return nullptr;
        }
        return &m_entries[row];
    }

    void recordMatch(const KRunner::QueryMatch &match, const QString &query)
    {
        const QString id = match.id();
        if (id.isEmpty()) {
            return;
        }

        const qint64 now = QDateTime::currentSecsSinceEpoch();
        auto it = std::find_if(m_entries.begin(), m_entries.end(), [&](const HistoryEntry &e) {
            return e.id == id;
        });

        if (it != m_entries.end()) {
            it->count += 1;
            it->lastUsed = now;
            it->title = match.text();
            it->subtext = match.subtext();
            it->category = match.matchCategory();
            if (!match.iconName().isEmpty()) {
                it->iconName = match.iconName();
            }
            if (match.runner()) {
                it->runnerId = match.runner()->id();
            }
            if (!query.isEmpty()) {
                it->query = query;
            }
        } else {
            HistoryEntry entry;
            entry.id = id;
            entry.title = match.text();
            entry.subtext = match.subtext();
            entry.category = match.matchCategory();
            entry.iconName = match.iconName();
            entry.runnerId = match.runner() ? match.runner()->id() : QString();
            entry.query = query;
            entry.count = 1;
            entry.lastUsed = now;
            m_entries.append(entry);
        }

        sortAndSave();
    }

    void incrementEntry(int row)
    {
        if (row < 0 || row >= m_entries.size()) {
            return;
        }
        m_entries[row].count += 1;
        m_entries[row].lastUsed = QDateTime::currentSecsSinceEpoch();
        sortAndSave();
    }

Q_SIGNALS:
    void settingsChanged();

private:
    void sortEntries()
    {
        std::sort(m_entries.begin(), m_entries.end(), [this](const HistoryEntry &a, const HistoryEntry &b) {
            if (m_mode == QLatin1String("recent")) {
                if (a.lastUsed != b.lastUsed) {
                    return a.lastUsed > b.lastUsed;
                }
                return a.count > b.count;
            }
            if (a.count != b.count) {
                return a.count > b.count;
            }
            return a.lastUsed > b.lastUsed;
        });
    }

    void loadSettings()
    {
        QFile f(m_cfgPath);
        if (!f.open(QIODevice::ReadOnly)) {
            return;
        }
        const QJsonObject obj = QJsonDocument::fromJson(f.readAll()).object();
        const QString m = obj.value(QStringLiteral("mode")).toString(QStringLiteral("often"));
        if (m == QLatin1String("often") || m == QLatin1String("recent") || m == QLatin1String("off")) {
            m_mode = m;
        }
        m_maxItems = std::clamp(obj.value(QStringLiteral("maxItems")).toInt(5), 1, 10);
    }

    void saveSettings() const
    {
        QJsonObject obj;
        obj.insert(QStringLiteral("mode"), m_mode);
        obj.insert(QStringLiteral("maxItems"), m_maxItems);
        QFile f(m_cfgPath);
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            f.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
        }
    }

    void loadHistory()
    {
        QFile f(m_dbPath);
        if (!f.open(QIODevice::ReadOnly)) {
            return;
        }
        const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
        if (!doc.isArray()) {
            return;
        }
        for (const QJsonValue &v : doc.array()) {
            const QJsonObject o = v.toObject();
            HistoryEntry e;
            e.id = o.value(QStringLiteral("id")).toString();
            e.title = o.value(QStringLiteral("title")).toString();
            e.subtext = o.value(QStringLiteral("subtext")).toString();
            e.category = o.value(QStringLiteral("category")).toString();
            e.iconName = o.value(QStringLiteral("iconName")).toString();
            e.runnerId = o.value(QStringLiteral("runnerId")).toString();
            e.query = o.value(QStringLiteral("query")).toString();
            e.count = o.value(QStringLiteral("count")).toInt(1);
            e.lastUsed = o.value(QStringLiteral("lastUsed")).toInteger();
            if (!e.id.isEmpty() && !e.title.isEmpty()) {
                m_entries.append(e);
            }
        }
        sortEntries();
    }

    void sortAndSave()
    {
        beginResetModel();
        sortEntries();
        endResetModel();

        QJsonArray arr;
        for (const auto &e : m_entries) {
            QJsonObject o;
            o.insert(QStringLiteral("id"), e.id);
            o.insert(QStringLiteral("title"), e.title);
            o.insert(QStringLiteral("subtext"), e.subtext);
            o.insert(QStringLiteral("category"), e.category);
            o.insert(QStringLiteral("iconName"), e.iconName);
            o.insert(QStringLiteral("runnerId"), e.runnerId);
            o.insert(QStringLiteral("query"), e.query);
            o.insert(QStringLiteral("count"), e.count);
            o.insert(QStringLiteral("lastUsed"), e.lastUsed);
            arr.append(o);
        }

        QFile f(m_dbPath);
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            f.write(QJsonDocument(arr).toJson(QJsonDocument::Compact));
        }
    }

    QString m_dbPath;
    QString m_cfgPath;
    QString m_mode = QStringLiteral("often");
    int m_maxItems = 5;
    QList<HistoryEntry> m_entries;
};

class RunnerBridge : public QObject
{
    Q_OBJECT
    Q_PROPERTY(KRunner::ResultsModel *model READ model CONSTANT)
    Q_PROPERTY(HistoryModel *historyModel READ historyModel CONSTANT)

public:
    explicit RunnerBridge(KRunner::ResultsModel *model, HistoryModel *history, QQmlComponent *windowComponent, QObject *parent = nullptr)
        : QObject(parent)
        , m_model(model)
        , m_history(history)
        , m_windowComponent(windowComponent)
    {
    }

    KRunner::ResultsModel *model() const { return m_model; }
    HistoryModel *historyModel() const { return m_history; }

    Q_INVOKABLE bool runRow(int row)
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

    Q_INVOKABLE bool runHistoryRow(int row)
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

        *matchConn = connect(mgr, &KRunner::RunnerManager::matchesChanged, this, [mgr, target, matchConn, finishConn](const QList<KRunner::QueryMatch> &matches) {
            for (const auto &m : matches) {
                if (m.id() == target.id) {
                    QObject::disconnect(*matchConn);
                    QObject::disconnect(*finishConn);
                    mgr->run(m);
                    return;
                }
            }
        });

        *finishConn = connect(mgr, &KRunner::RunnerManager::queryFinished, this, [mgr, target, matchConn, finishConn]() {
            QObject::disconnect(*matchConn);
            QObject::disconnect(*finishConn);
            const auto matches = mgr->matches();
            for (const auto &m : matches) {
                if (m.id() == target.id || m.text() == target.title) {
                    mgr->run(m);
                    break;
                }
            }
        });

        mgr->launchQuery(queryTerm, target.runnerId);
        return true;
    }

    Q_INVOKABLE void hide()
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

    Q_INVOKABLE void show()
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

    void toggle()
    {
        if (m_window && m_window->isVisible()) {
            hide();
        } else {
            show();
        }
    }

Q_SIGNALS:
    void shown();

private:
    KRunner::ResultsModel *m_model = nullptr;
    HistoryModel *m_history = nullptr;
    QQmlComponent *m_windowComponent = nullptr;
    QQuickWindow *m_window = nullptr;
    bool m_closing = false;
};
} // namespace

int main(int argc, char *argv[])
{
    const QStringList args = [&]() {
        QStringList list;
        for (int i = 0; i < argc; ++i) {
            list << QString::fromLocal8Bit(argv[i]);
        }
        return list;
    }();

    // Single-instance IPC: if daemon is already running, send toggle/quit and exit
    {
        QLocalSocket probe;
        probe.connectToServer(kSocketName);
        if (probe.waitForConnected(150)) {
            const QByteArray msg = args.contains(QStringLiteral("--quit")) ? "quit" : "toggle";
            probe.write(msg);
            probe.flush();
            probe.waitForBytesWritten(150);
            return 0;
        }
    }

    if (args.contains(QStringLiteral("--quit"))) {
        return 0;
    }

    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("kadrunner"));
    app.setApplicationDisplayName(QStringLiteral("KadRunner"));
    app.setQuitOnLastWindowClosed(false);

    const auto krunnerCfg = KSharedConfig::openConfig(QStringLiteral("krunnerrc"));
    const KConfigGroup cfgGroup = krunnerCfg->group(QStringLiteral("General"));
    const KConfigGroup stateGroup = krunnerCfg->group(QStringLiteral("State"));

    auto *resultsModel = new KRunner::ResultsModel(cfgGroup, stateGroup, &app);
    resultsModel->setLimit(20);

    auto *historyModel = new HistoryModel(&app);

    QQmlApplicationEngine engine;
    auto *windowComponent = new QQmlComponent(&engine, &app);
    auto *bridge = new RunnerBridge(resultsModel, historyModel, windowComponent, &app);
    engine.rootContext()->setContextProperty(QStringLiteral("runnerBridge"), bridge);
    windowComponent->loadFromModule(QStringLiteral("KadRunner"), QStringLiteral("Main"));

    if (windowComponent->isError()) {
        qWarning() << windowComponent->errors();
        return 1;
    }

    // Register global hotkey with KDE KGlobalAccel (works natively on Wayland)
    auto *toggleAction = new QAction(QStringLiteral("Toggle KadRunner"), &app);
    toggleAction->setObjectName(QStringLiteral("toggle"));
    KGlobalAccel::setGlobalShortcut(toggleAction, QKeySequence(QStringLiteral("Meta+/")));
    QObject::connect(toggleAction, &QAction::triggered, bridge, &RunnerBridge::toggle);

    // Start IPC server for CLI/shortcut toggling
    QLocalServer::removeServer(kSocketName);
    QLocalServer server(&app);
    server.listen(kSocketName);
    QObject::connect(&server, &QLocalServer::newConnection, &app, [&]() {
        while (QLocalSocket *client = server.nextPendingConnection()) {
            QObject::connect(client, &QLocalSocket::readyRead, client, [&, client]() {
                const QByteArray cmd = client->readAll().trimmed();
                if (cmd == "quit") {
                    app.quit();
                } else {
                    bridge->toggle();
                }
                client->deleteLater();
            });
        }
    });

    if (!args.contains(QStringLiteral("--daemon"))) {
        bridge->show();
    }

    return app.exec();
}

#include "main.moc"
