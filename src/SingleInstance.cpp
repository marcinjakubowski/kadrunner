#include "SingleInstance.h"

#include <QLocalSocket>

SingleInstance::SingleInstance(QString socketName, QObject *parent)
    : QObject(parent)
    , m_socketName(std::move(socketName))
{
}

bool SingleInstance::sendToExisting(bool quitRequested) const
{
    QLocalSocket probe;
    probe.connectToServer(m_socketName);
    if (!probe.waitForConnected(150)) {
        return false;
    }
    probe.write(quitRequested ? "quit" : "toggle");
    probe.flush();
    probe.waitForBytesWritten(150);
    return true;
}

bool SingleInstance::listen()
{
    QLocalServer::removeServer(m_socketName);
    if (!m_server.listen(m_socketName)) {
        return false;
    }

    connect(&m_server, &QLocalServer::newConnection, this, [this]() {
        while (QLocalSocket *client = m_server.nextPendingConnection()) {
            connect(client, &QLocalSocket::readyRead, client, [this, client]() {
                const QByteArray cmd = client->readAll().trimmed();
                if (cmd == "quit") {
                    emit quitRequested();
                } else {
                    emit toggleRequested();
                }
                client->deleteLater();
            });
        }
    });
    return true;
}
