#include <QDebug>

#include "openvpnprotocol.h"
#include "vpnconnection.h"

VpnConnection::VpnConnection(QObject* parent) : QObject(parent)
{

}

VpnConnection::~VpnConnection()
{

}

void VpnConnection::onBytesChanged(quint64 receivedBytes, quint64 sentBytes)
{
    emit bytesChanged(receivedBytes, sentBytes);
}

void VpnConnection::onConnectionStateChanged(VpnProtocol::ConnectionState state)
{
    emit connectionStateChanged(state);
}

void VpnConnection::connectToVpn(Protocol protocol)
{
    qDebug() << "Connect to VPN";

    switch (protocol) {
    case Protocol::OpenVpn:
        m_vpnProtocol.reset(new OpenVpnProtocol());
        break;
        ;
    default:
        // TODO, add later
        return;
        ;
    }

    connect(m_vpnProtocol.data(), SIGNAL(connectionStateChanged(VpnProtocol::ConnectionState)), this, SLOT(onConnectionStateChanged(VpnProtocol::ConnectionState)));
    connect(m_vpnProtocol.data(), SIGNAL(bytesChanged(quint64, quint64)), this, SLOT(onBytesChanged(quint64, quint64)));

    m_vpnProtocol.data()->start();
}

QString VpnConnection::bytesToText(quint64 bytes)
{
    return QString("%1 %2").arg((bytes * 8) / 1024).arg(tr("Mbps"));
}

void VpnConnection::disconnectFromVpn()
{
    qDebug() << "Disconnect from VPN";

    if (!m_vpnProtocol.data()) {
        return;
    }
    m_vpnProtocol.data()->stop();
}

bool VpnConnection::connected() const
{
    if (!m_vpnProtocol.data()) {
        return false;
    }

    return m_vpnProtocol.data()->connected();
}

bool VpnConnection::disconnected() const
{
    if (!m_vpnProtocol.data()) {
        return true;
    }

    return m_vpnProtocol.data()->disconnected();
}
