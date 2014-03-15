#ifndef SCREENSHARINGCONNECTIONMANAGER_H
#define SCREENSHARINGCONNECTIONMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QUdpSocket>
#include "screensharingconnection.h"
#include "screenmanager.h"
#include "filesharingmanger.h"
#include <QList>
#include <QUuid>
#include <QTimer>

class ScreenSharingConnection;

class ScreenSharingConnectionManager : public QObject
{
    Q_OBJECT
public:
    explicit ScreenSharingConnectionManager(ScreenManager *screenManager, FileSharingManger *fileManager, QObject *parent = 0);
    ~ScreenSharingConnectionManager();

    void addConnection(ScreenSharingConnection *connection);
    const QList<ScreenSharingConnection *> *connectionList();
    void setScreenManager(ScreenManager *manager);

    void pushFileToAll(const QString &filename, const QByteArray &data);

signals:
    void connectionAdded(ScreenSharingConnection *);
    void connectionClosing(ScreenSharingConnection *);

public slots:
    bool startServer();
    void stopServer();
    bool isServerRunning();
    void cleanupConnections();
    void closeAllConnections();

private slots:
    void newConnection();
    void connectionClosingSlot();
    void readyDiagram();
    void advertisingBroadcast();

    void autoconnectionConnected();
    void autocconectionError(QAbstractSocket::SocketError socketError);

    void onPushFileContentReceived(const QString &filename, const QByteArray &content);

private:
    QTcpServer *m_server;
    QUdpSocket *m_udpSocket;
    QList<ScreenSharingConnection *> m_connectionList;
    QList<ScreenSharingConnection *> m_connectionListToDelete;
    ScreenManager *m_screenManager;
    FileSharingManger *m_fileSharingManager;
    QUuid m_uuid;
    QTimer m_timer;
};

#endif // SCREENSHARINGCONNECTIONMANAGER_H
