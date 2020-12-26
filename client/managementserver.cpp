#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>

#include "managementserver.h"

ManagementServer::ManagementServer(QObject *parent) : QObject(parent),
    m_tcpServer(nullptr)
{

}

ManagementServer::~ManagementServer()
{

}

bool ManagementServer::isOpen() const
{
    return (m_socket && m_socket->isOpen());
}

void ManagementServer::stop()
{
    m_tcpServer->close();
}

void ManagementServer::onAcceptError(QAbstractSocket::SocketError socketError)
{
    qDebug().noquote() << QString("Accept error: %1").arg(socketError);
}

qint64 ManagementServer::writeCommand(const QString& message)
{
    if (!isOpen()) {
        return 0;
    }

    const QString command = message + "\n";
    return m_socket->write(command.toStdString().c_str());
}

void ManagementServer::onNewConnection()
{
    qDebug() << "New incoming connection";

    m_socket = m_tcpServer->nextPendingConnection();
    m_tcpServer->close();

    QObject::connect(m_socket, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));
    QObject::connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
    QObject::connect(m_socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

void ManagementServer::onSocketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);

    qDebug().noquote() << QString("Mananement server error: %1").arg(m_socket->errorString());
}

void ManagementServer::onSocketDisconnected()
{
    m_socket->deleteLater();
}

QTcpSocket* ManagementServer::socket() const
{
    if (!isOpen()) {
        return nullptr;
    }
    return m_socket;
}

void ManagementServer::onReadyRead()
{
    emit readyRead();
}

bool ManagementServer::start(const QString& host, unsigned int port)
{
    if (m_tcpServer) {
        delete m_tcpServer;
    }

    m_tcpServer = new QTcpServer(this);
    m_tcpServer->setMaxPendingConnections(1);

    connect(m_tcpServer, SIGNAL(acceptError(QAbstractSocket::SocketError)), this, SLOT(onAcceptError(QAbstractSocket::SocketError)));
    connect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));

    if (m_tcpServer->listen(QHostAddress(host), port)) {
        emit serverStarted();
        return true;
    }

    qDebug().noquote() << QString("Can't start TCP server, %1,%2")
                      .arg(m_tcpServer->serverError())
                      .arg(m_tcpServer->errorString());
    return false;
}

QString ManagementServer::readLine()
{
    if (!isOpen()) {
        qDebug() << "Socket is not opened";
        return QString();
    }

    return m_socket->readLine();
}
