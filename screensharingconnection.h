#ifndef SCREENSHARINGCONNECTION_H
#define SCREENSHARINGCONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QStringList>
#include <QUuid>
#include "screenmanager.h"
#include "filesharingmanger.h"

class ScreenSharingConnection : public QObject
{
    Q_OBJECT
public:
    explicit ScreenSharingConnection(QTcpSocket *connection, QObject *parent = 0);
    void setScreenManager(ScreenManager *manager);
    void setFileSharingManager(FileSharingManger *manager);
    const QTcpSocket *socket();
    QString toString() const;
    void close();
    QString peerName() const {return m_peerName;}
    QImage currentRemoteImage() const {return m_remoteImage;}
    QPixmap currentRemotePixmap() const {return QPixmap::fromImage(m_remoteImage);}
    const QStringList &remoteFileList() const {return m_remoteFileList;}
    QUuid remoteUuid() const {return m_remoteUuid;}
    void requestFile(const QString& filename);

signals:
    void connectionClosing();
    void updateImage(const QPixmap &image);
    void updateImageNoChange();
    void updateCursorPoint(QPoint point);
    void updateFileList(const QStringList &fileList);
    void receivedFileContent(const QString &filename, const QByteArray &content);
    void receivedPushedFileContent(const QString &filename, const QByteArray &content);

    void writeOnMainThread(const QByteArray &data); // should be private

public slots:
    void requestScreenUpdate();
    void requestCursorUpdate();
    void requestFileListUpdate();

    void pushFileContent(const QString &name, const QByteArray &data);

private:
    enum State {
        NOT_CONNECTED,
        HEADER_RECIVING,
        CONTENT_RECIVING
    };

    int m_expectedBytes;
    QByteArray m_buffer;

    QTcpSocket *m_socket;
    QString m_peerName;
    ScreenManager *m_screenManager;
    FileSharingManger *m_fileSharingManager;
    QImage m_remoteImage;
    QStringList m_remoteFileList;
    QUuid m_remoteUuid;
    QString m_requestedFileName;

    enum State m_state;
    int m_currentHeader;

    QSize m_size;
    QPoint m_remoteCursorPoint;

    bool m_closed;

    int m_localScreenIndex;

    void printData(const QByteArray &data);
    void processReceivedData(const QByteArray &data);
    void sendData(char type, const QByteArray &data = QByteArray());

private slots:
    void readyRead();
    void stateChanged(QAbstractSocket::SocketState newstate);
    void onError(QAbstractSocket::SocketError error);
    void onFileListUpdated(const QStringList &filelist);

    void writeData(const QByteArray &data);
};

#endif // SCREENSHARINGCONNECTION_H
