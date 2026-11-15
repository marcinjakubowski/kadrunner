#include "HistoryModel.h"
#include "RunnerBridge.h"
#include "SingleInstance.h"

#include <QAction>
#include <QDebug>
#include <QGuiApplication>
#include <QIcon>
#include <QKeySequence>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>

#include <KConfigGroup>
#include <KGlobalAccel>
#include <KRunner/ResultsModel>
#include <KSharedConfig>

int main(int argc, char *argv[])
{
    QStringList args;
    args.reserve(argc);
    for (int i = 0; i < argc; ++i) {
        args << QString::fromLocal8Bit(argv[i]);
    }

    const bool quitFlag = args.contains(QStringLiteral("--quit"));
    SingleInstance ipc(QStringLiteral("kadrunner-ipc"));
    if (ipc.sendToExisting(quitFlag) || quitFlag) {
        return 0;
    }

    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("kadrunner"));
    app.setApplicationDisplayName(QStringLiteral("KadRunner"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("kadrunner")));
    app.setQuitOnLastWindowClosed(false);
    QCoreApplication::setQuitLockEnabled(false);

    const auto krunnerCfg = KSharedConfig::openConfig(QStringLiteral("krunnerrc"));
    auto *resultsModel = new KRunner::ResultsModel(
        krunnerCfg->group(QStringLiteral("General")),
        krunnerCfg->group(QStringLiteral("State")),
        &app);
    resultsModel->setLimit(20);
    const QStringList favoriteIds = krunnerCfg->group(QStringLiteral("Plugins"))
                                        .group(QStringLiteral("Favorites"))
                                        .readEntry(QStringLiteral("plugins"), QStringList());
    if (!favoriteIds.isEmpty()) {
        resultsModel->setFavoriteIds(favoriteIds);
    }

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

    auto *toggleAction = new QAction(QStringLiteral("Toggle KadRunner"), &app);
    toggleAction->setObjectName(QStringLiteral("toggle"));
    KGlobalAccel::setGlobalShortcut(toggleAction, QList<QKeySequence>{
        QKeySequence(QStringLiteral("Meta")),
        QKeySequence(QStringLiteral("Meta+/")),
    });
    QObject::connect(toggleAction, &QAction::triggered, bridge, &RunnerBridge::toggle);

    QObject::connect(&ipc, &SingleInstance::toggleRequested, bridge, &RunnerBridge::toggle);
    QObject::connect(&ipc, &SingleInstance::quitRequested, &app, &QCoreApplication::quit);
    ipc.listen();

    if (!args.contains(QStringLiteral("--daemon"))) {
        bridge->show();
    }

    return app.exec();
}
