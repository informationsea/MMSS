#include "screensharingconnectionmanager.h"
#include <QHostAddress>
#include <QDebug>
#include "common.h"

#include <QFileDialog>
#include <QNetworkInterface>
#include "sheetmessagebox.h"

ScreenSharingConnectionManager::ScreenSharingConnectionManager(ScreenManager *screenManager, FileSharingManger *fileManager, QObject *parent) :
    QObject(parent), m_server(NULL), m_udpSocket(NULL), m_screenManager(screenManager),
    m_fileSharingManager(fileManager), m_uuid(g_settings->value(SETTINGS_UUID).toString()), m_timer(this)
{
    m_udpSocket = new QUdpSocket(this);
    if (!m_udpSocket->bind(QHostAddress(QHostAddress::Any), CONNECT_PORT, QAbstractSocket::ShareAddress))
        qDebug() << "Cannot bind broadcast reciver";
    connect(m_udpSocket, SIGNAL(readyRead()), SLOT(readyDiagram()));

    m_timer.setInterval(10000);
    m_timer.setSingleShot(false);
    connect(&m_timer, SIGNAL(timeout()), SLOT(advertisingBroadcast()));
    //m_timer.start();
}

ScreenSharingConnectionManager::~ScreenSharingConnectionManager()
{
    cleanupConnections();
    qDebug() << "Close UDP socket";
    m_udpSocket->close();
    delete m_udpSocket;

    if (m_server)
        delete m_server;
}

void ScreenSharingConnectionManager::addConnection(ScreenSharingConnection *connection)
{
    cleanupConnections();
    m_connectionList.append(connection);
    connection->setScreenManager(m_screenManager);
    connection->setFileSharingManager(m_fileSharingManager);
    connect(connection, SIGNAL(connectionClosing()), SLOT(connectionClosingSlot()));
    connect(connection, SIGNAL(receivedPushedFileContent(QString,QByteArray)), SLOT(onPushFileContentReceived(QString,QByteArray)));
    emit connectionAdded(connection);
}

const QList<ScreenSharingConnection *>* ScreenSharingConnectionManager::connectionList()
{
    cleanupConnections();
    return &m_connectionList;
}

void ScreenSharingConnectionManager::setScreenManager(ScreenManager *manager)
{
    m_screenManager = manager;
}

void ScreenSharingConnectionManager::pushFileToAll(const QString &filename, const QByteArray &data)
{
    foreach (ScreenSharingConnection *conn, m_connectionList) {
        conn->pushFileContent(filename, data);
    }
}

bool ScreenSharingConnectionManager::startServer()
{
    if (m_server) {
        m_server->resumeAccepting();
        return true;
    }

    m_server = new QTcpServer(this);
    connect(m_server, SIGNAL(newConnection()), SLOT(newConnection()));
    if (m_server->listen(QHostAddress::Any, CONNECT_PORT)) {
        advertisingBroadcast();
        m_timer.start();
        return true;
    } else {
        delete m_server;
        m_server = NULL;
        return false;
    }
}

void ScreenSharingConnectionManager::stopServer()
{
    m_server->close();
    delete m_server;
    m_server = NULL;
    m_timer.stop();
}

bool ScreenSharingConnectionManager::isServerRunning()
{
    return m_server && m_server->isListening();
}

void ScreenSharingConnectionManager::cleanupConnections()
{
    foreach (ScreenSharingConnection *con, m_connectionListToDelete) {
        delete con;
    }
    m_connectionListToDelete.clear();
}

void ScreenSharingConnectionManager::closeAllConnections()
{
    cleanupConnections();
    foreach(ScreenSharingConnection *conn, m_connectionList) {
        conn->close();
    }
}

void ScreenSharingConnectionManager::newConnection()
{
    cleanupConnections();
    QTcpSocket *newsocket = m_server->nextPendingConnection();

    bool anymatch = false;

    foreach(QString subnetstr, g_settings->value(SETTINGS_ALLOW_IPS).toStringList()) {
        QPair<QHostAddress, int> subnet = QHostAddress::parseSubnet(subnetstr);
        if (newsocket->peerAddress().isInSubnet(subnet))
            anymatch = true;
    }

    if (anymatch) {
        addConnection(new ScreenSharingConnection(newsocket, this));
    } else {
        newsocket->close();
    }
}

void ScreenSharingConnectionManager::connectionClosingSlot()
{
    cleanupConnections();
    ScreenSharingConnection *sender = static_cast<ScreenSharingConnection *>(QObject::sender());
    m_connectionList.removeOne(sender);
    emit connectionClosing(sender);
    m_connectionListToDelete.append(sender);
}

void ScreenSharingConnectionManager::readyDiagram()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(m_udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        m_udpSocket->readDatagram(data.data(), data.size(),
                                  &sender, &senderPort);

        QUuid uuid = QUuid::fromRfc4122(data.left(16));
        if (uuid == m_uuid) // own UUID
            continue;
        foreach (ScreenSharingConnection *conn, m_connectionList) {
            if (conn->remoteUuid() == uuid)
                goto next;
        }

        qDebug() << "received datagram" << uuid << sender << senderPort;

        {
            QTcpSocket *conn = new QTcpSocket(this);
            connect(conn, SIGNAL(connected()), SLOT(autoconnectionConnected()));
            connect(conn, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(autocconectionError(QAbstractSocket::SocketError)));
            conn->connectToHost(sender, CONNECT_PORT);
        }

next:
        continue;
    }
}

void ScreenSharingConnectionManager::advertisingBroadcast()
{
    if (!m_server) return;
    //qDebug() << "send datagram";
    QByteArray data = m_uuid.toRfc4122();
    QUdpSocket udp;
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    // send advertising packet to all networks
    foreach (QNetworkInterface oneif, interfaces) {
        if (!oneif.isValid())
            continue;
        QList<QNetworkAddressEntry> entries = oneif.addressEntries();
        foreach (QNetworkAddressEntry oneentry, entries) {
            if (oneentry.ip().isLoopback())
                continue;
            if (oneentry.broadcast().isNull())
                continue;
            //qDebug() << "Send to " << oneentry.broadcast();
            udp.writeDatagram(data, oneentry.broadcast(), CONNECT_PORT);
        }
    }
}

void ScreenSharingConnectionManager::autoconnectionConnected()
{
    QTcpSocket *conn = (QTcpSocket *)sender();

    addConnection(new ScreenSharingConnection(conn, this));
}

void ScreenSharingConnectionManager::autocconectionError(QAbstractSocket::SocketError socketError)
{
    QTcpSocket *conn = (QTcpSocket *)sender();

    qDebug() << conn->errorString() << "auto connection refused" << socketError;
}

void ScreenSharingConnectionManager::onPushFileContentReceived(const QString &filename, const QByteArray &content)
{
    QString path = QFileDialog::getSaveFileName(NULL, QString("Save %1").arg(filename), QDir::home().absoluteFilePath(filename));
    if (path.isEmpty())
        return;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        SheetMessageBox::critical(NULL, tr("Cannot save file"), file.errorString());
        return;
    }
    file.write(content);
    file.close();
}
