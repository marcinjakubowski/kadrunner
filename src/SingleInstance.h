#pragma once

#include <QLocalServer>
#include <QObject>
#include <QString>

/**
 * Handles single-instance IPC over a local socket so secondary invocations
 * can toggle or quit the running daemon.
 */
class SingleInstance : public QObject
{
    Q_OBJECT

public:
    explicit SingleInstance(QString socketName, QObject *parent = nullptr);

    /// Returns true if an existing instance was contacted and handled the command.
    bool sendToExisting(bool quitRequested) const;

    /// Starts listening for incoming toggle/quit commands.
    bool listen();

Q_SIGNALS:
    void toggleRequested();
    void quitRequested();

private:
    QString m_socketName;
    QLocalServer m_server;
};
