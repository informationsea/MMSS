#include "screensharingconnection.h"

#include <QDebug>
#include <QHostAddress>
#include <QTextStream>

#include "common.h"

/*
 * Protocol
 *
 * # Handshake (20 bytes)
 * |"KSS1" (ascii)|UUID|
 * 1: version in ascii code
 * UUID : local UUID
 *
 * # Header (6 bytes)
 * |0x48|Packet Type|Packet Size without Header 4bytes|
 *
 * ## Packet Type
 * 0x00: Request Configuration (Length 0)
 * 0x01: Request Screen Update (Length 0)
 * 0x02: Request Cursor Position Update (Length 0)
 * 0x03: Request File List (Length 0)
 * 0x04: Request File Content (Length N)
 *       |File Name (n bytes)|
 * 0x10: Configuration Response (Length 5+)
 *       |Screen width (2 bytes)|Screen Height (2 bytes)|Name Length(1 byte)|Name (n bytes)
 * 0x11: Screen Update (Length: image size)
 *       |Image data|
 *       Image data can be empty if no updates
 * 0x12: Cursor Position Response (Length 4)
 *       |X (2 bytes)|Y (2 bytes)|
 * 0x13: File list
 *       |Null separated string list)
 * 0x14: File Content
 *       |Name Length (2 bytes)|File Name (n bytes)|Content (remained bytes)
 * 0x25: Push File Content
 *       |Name Length (2 bytes)|File Name (n bytes)|Content (remained bytes)
 *
 * ### Acceptable Image Format:
 * JPEG, PNG
 * */

static const size_t HANDSHAKE_SIZE = 20;
static const size_t HEADER_SIZE = 6;
static const char HEADER_MAGIC = 0x48;

static const char TYPE_REQUEST_CONFIGURATION = 0x00;
static const char TYPE_REQUEST_SCREEN_UPDATE = 0x01;
static const char TYPE_REQUEST_CURSOR_UPDATE = 0x02;
static const char TYPE_REQUEST_FILE_LIST     = 0x03;
static const char TYPE_REQUEST_FILE_CONTENT  = 0x04;

static const char TYPE_RESPONSE_CONFIGURATION = 0x10;
static const char TYPE_RESPONSE_SCREEN_UPDATE = 0x11;
static const char TYPE_RESPONSE_CURSOR_UPDATE = 0x12;
static const char TYPE_RESPONSE_FILE_LIST     = 0x13;
static const char TYPE_RESPONSE_FILE_CONTENT  = 0x14;

static const char TYPE_PUSH_FILE_CONTENT  = 0x25;


static QByteArray int2data(int value, size_t length)
{
    QByteArray data;
    for (size_t i = 0; i < length; i++) {
        char ch = value >> ((length -i -1)*8);
        data.append(ch);
    }
    return data;
}


ScreenSharingConnection::ScreenSharingConnection(QTcpSocket *connection, QObject *parent) :
    QObject(parent), m_expectedBytes(HANDSHAKE_SIZE), m_socket(connection), m_screenManager(NULL),
    m_fileSharingManager(NULL), m_state(NOT_CONNECTED), m_closed(false), m_localScreenIndex(0)
{
    qDebug() << connection->peerAddress().toString() << connection->peerPort() << connection->localAddress().toString() << connection->localPort();
    connect(connection, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(stateChanged(QAbstractSocket::SocketState)));
    connect(connection, SIGNAL(readyRead()), SLOT(readyRead()));
    connect(this, SIGNAL(writeOnMainThread(QByteArray)), SLOT(writeData(QByteArray)), Qt::QueuedConnection);

    emit writeOnMainThread(QByteArray("KSS1", 4));
    emit writeOnMainThread(QUuid(g_settings->value(SETTINGS_UUID).toString()).toRfc4122());
}

void ScreenSharingConnection::setScreenManager(ScreenManager *manager)
{
    m_screenManager = manager;
}

void ScreenSharingConnection::setFileSharingManager(FileSharingManger *manager)
{
    m_fileSharingManager = manager;
    connect(m_fileSharingManager, SIGNAL(fileListUpdated(QStringList)), SLOT(onFileListUpdated(QStringList)));
}

const QTcpSocket *ScreenSharingConnection::socket()
{
    return m_socket;
}

QString ScreenSharingConnection::toString() const
{
    return QString("<%1> %2[%3] -> %4[%5]").arg(m_peerName,
                                                m_socket->localAddress().toString(), QString::number(m_socket->localPort()),
                                                m_socket->peerAddress().toString(), QString::number(m_socket->peerPort()));
}

void ScreenSharingConnection::close()
{
    if (!m_closed) {
        m_closed = true;
        qDebug() << toString() << "closing";
        if (m_socket->state() ==  QAbstractSocket::ConnectedState)
            m_socket->close();
        emit connectionClosing();
    }
}

void ScreenSharingConnection::requestFile(const QString &filename)
{
    sendData(TYPE_REQUEST_FILE_CONTENT, filename.toUtf8());
    m_requestedFileName = filename;
}

void ScreenSharingConnection::requestScreenUpdate()
{
    if (m_state == HEADER_RECIVING || m_state == CONTENT_RECIVING) {
        sendData(TYPE_REQUEST_SCREEN_UPDATE);
        // emit writeOnMainThread(QByteArray(PACKET_REQUEST_SCREEN_UPDATE, sizeof(PACKET_REQUEST_SCREEN_UPDATE)));
    }
}

void ScreenSharingConnection::requestCursorUpdate()
{
    if (m_state == HEADER_RECIVING || m_state == CONTENT_RECIVING) {
        sendData(TYPE_REQUEST_CURSOR_UPDATE);
        //emit writeOnMainThread(QByteArray(PACKET_REQUEST_CURSOR_UPDATE, sizeof(PACKET_REQUEST_CURSOR_UPDATE)));
    }
}

void ScreenSharingConnection::requestFileListUpdate()
{
    if (m_state == HEADER_RECIVING || m_state == CONTENT_RECIVING) {
        sendData(TYPE_REQUEST_FILE_LIST);
        //emit writeOnMainThread(QByteArray(PACKET_REQUEST_CURSOR_UPDATE, sizeof(PACKET_REQUEST_CURSOR_UPDATE)));
    }
}

void ScreenSharingConnection::pushFileContent(const QString &name, const QByteArray &data)
{
    QByteArray nameBytes = name.toUtf8();
    QByteArray content;
    content.append(int2data(nameBytes.length(), 2));
    content.append(nameBytes);
    content.append(data);
    sendData(TYPE_PUSH_FILE_CONTENT, content);
}

void ScreenSharingConnection::printData(const QByteArray &data)
{
    QByteArray ba;
    QTextStream ts(&ba);
    foreach (char ch, data) {
        ts.setPadChar('0');
        ts.setFieldWidth(2);
        ts << hex << (int)(((int)ch)&0xff);
        ts.setFieldWidth(1);
        ts << ' ';
    }
    ts.flush();
    qDebug() << toString() << ba;
}


static int data2int(QByteArray data, size_t length)
{
    int value = 0;
    for (size_t i = 0; i < length; i++) {
        value <<= 8;
        value |= (((int)data.at(i)) & 0xff);
        //qDebug() << "data2int" << value;
    }
    return value;
}

void ScreenSharingConnection::processReceivedData(const QByteArray &data)
{
    //qDebug() << toString() << "data" << m_expectedBytes << m_state;
    //printData(data);

    switch (m_state) {
    case NOT_CONNECTED:
        if (data.size() == HANDSHAKE_SIZE &&
                data[0] == 'K' &&
                data[1] == 'S' &&
                data[2] == 'S' &&
                data[3] == '1') {
            m_state = HEADER_RECIVING;
            m_expectedBytes = HEADER_SIZE;
            m_remoteUuid = QUuid::fromRfc4122(data.mid(4, 16));
            qDebug() << toString() << "connected" << m_remoteUuid.toString();
            sendData(TYPE_REQUEST_CONFIGURATION);

            QStringList fileList = m_fileSharingManager->fileList();
            onFileListUpdated(fileList);
            //emit writeOnMainThread(QByteArray(PACKET_REQUEST_CONFIGURATION, sizeof(PACKET_REQUEST_CONFIGURATION)));
        } else {
            qDebug() << "Invalid handshake" << data.size() << data.left(4);
            close();
        }
        break;
    case HEADER_RECIVING: {
        if (data[0] != 0x48) {
            qDebug() << "Invalid data" << data[0];
            close();
            return;
        }
        m_expectedBytes = data2int(data.mid(2), 4);
        m_currentHeader = data[1];
        //qDebug() << toString() << "header" << hex << m_currentHeader;

        switch(data[1]) {
        case TYPE_REQUEST_CONFIGURATION:{
            QByteArray response;
            QByteArray name = g_settings->value(SETTINGS_NAME, "").toString().toUtf8();
            QSize size = m_screenManager->screenSize();
            response.append(int2data(size.width(), 2));
            response.append(int2data(size.height(), 2));
            response.append(int2data(name.length(), 1));
            response.append(name);
            sendData(TYPE_RESPONSE_CONFIGURATION, response);
            break;
        }
        case TYPE_REQUEST_SCREEN_UPDATE: {
            QByteArray current = m_screenManager->currentData(&m_localScreenIndex);
            //qDebug() << toString() << "request screen update" << m_localScreenIndex << current.size();
            sendData(TYPE_RESPONSE_SCREEN_UPDATE, current);
            break;
        }
        case TYPE_REQUEST_CURSOR_UPDATE: {
            QByteArray response;
            QPoint point = m_screenManager->cursorPosition();
            response.append(int2data(point.x(), 2));
            response.append(int2data(point.y(), 2));
            sendData(TYPE_RESPONSE_CURSOR_UPDATE, response);
            break;
        }
        case  TYPE_REQUEST_FILE_LIST: {
            QStringList fileList = m_fileSharingManager->fileList();
            onFileListUpdated(fileList);
            break;
        }
        case TYPE_REQUEST_FILE_CONTENT:
            // This request have content
            break;
        case TYPE_RESPONSE_CONFIGURATION:
            break;
        case TYPE_RESPONSE_CURSOR_UPDATE:
            break;
        case TYPE_RESPONSE_FILE_LIST:
            break;
        case TYPE_RESPONSE_SCREEN_UPDATE:
            //qDebug() << toString() << "Screen update header recieved" << m_expectedBytes;
            if (m_expectedBytes == 0) {
                emit updateImageNoChange();
            }
            break;
        case TYPE_RESPONSE_FILE_CONTENT:
        case TYPE_PUSH_FILE_CONTENT:
            break;
        default:
            Q_ASSERT(0);
            break;
        }

        if (m_expectedBytes) {
            m_state = CONTENT_RECIVING;
        } else {
            m_expectedBytes = HEADER_SIZE;
        }

        break;
    }
    case CONTENT_RECIVING:
        // qDebug() << "reciving data" << m_currentHeader << data.size();
//        if (data.size() < 20)
//            printData(data);
//        else
//            printData(data.left(20));

        switch (m_currentHeader) {
        case TYPE_REQUEST_FILE_CONTENT: {
            qDebug() << toString() << "file request" << data;
            QByteArray content;
            content.append(int2data(data.length(), 2));
            content.append(data);
            content.append(m_fileSharingManager->fileContent(QString::fromUtf8(data)));
            sendData(TYPE_RESPONSE_FILE_CONTENT, content);
            break;
        }
        case TYPE_RESPONSE_CONFIGURATION: {
            if (data.length() < 5) {
                Q_ASSERT(0);
                break; // ignore illegal packet
            }
            m_size.setWidth(data2int(data, 2));
            m_size.setHeight(data2int(data.mid(2, 2), 2));
            int namelength = data2int(data.mid(4, 1), 1);
            m_peerName = QString::fromUtf8(data.mid(5, namelength));
            break;
        }
        case TYPE_RESPONSE_SCREEN_UPDATE:
            //qDebug() << toString() << "Screen update recieved";
            if (data.size() > 2) {
                if ((uchar)data[0] == 0xff && (uchar)data[1] == 0xd8) {
                    m_remoteImage = QImage::fromData(data, "JPEG");
                    emit updateImage(QPixmap::fromImage(m_remoteImage));
                } else if ((uchar)data[0] == 0x89 && (uchar)data[1] == 0x50) {
                    m_remoteImage = QImage::fromData(data, "PNG");
                    emit updateImage(QPixmap::fromImage(m_remoteImage));
                } else {
                    qDebug() << "Unsupported data type";
                    printData(data.left(4));
                    Q_ASSERT(0);
                }
            } else {
                qDebug() << "Too small data";
                Q_ASSERT(0);
            }
            break;
        case TYPE_RESPONSE_CURSOR_UPDATE:
            if (data.length() < 4) {
                Q_ASSERT(0);
                break; // ignore illegal packet
            }
            m_remoteCursorPoint.setX(data2int(data, 2));
            m_remoteCursorPoint.setY(data2int(data.mid(2), 2));
            emit updateCursorPoint(m_remoteCursorPoint);
            break;
        case TYPE_RESPONSE_FILE_LIST: {
            int start = 0;
            int pos = data.indexOf('\0');
            QStringList list;
            while (pos < data.length() && pos > 0) {
                list << data.mid(start, pos - start);
                start = pos+1;
                pos = data.indexOf('\0', start);
            }
            m_remoteFileList = list;
            emit updateFileList(m_remoteFileList);
            break;
        }
        case TYPE_RESPONSE_FILE_CONTENT:{
            int length = data2int(data, 2);
            QString name = QString::fromUtf8(data.mid(2, length));
            if (name == m_requestedFileName) {
                QByteArray content = data.mid(2+length);
                emit receivedFileContent(name, content);
                m_requestedFileName = "";
            } else {
                qDebug() << "invalid response; file content" << name;
            }
            break;
        }
        case TYPE_PUSH_FILE_CONTENT:{
            qDebug() << "push file recieved" << data.length();
            int length = data2int(data, 2);
            QString name = QString::fromUtf8(data.mid(2, length));
            QByteArray content = data.mid(2+length);
            emit receivedPushedFileContent(name, content);
            break;
        }
        default:
            Q_ASSERT(0);
            //assert(1);
            break;
        }

        m_expectedBytes = HEADER_SIZE;
        m_state = HEADER_RECIVING;
        break;
    default: // Error
        Q_ASSERT(0);
        qDebug() << "error" << m_socket->state() << m_state;
        close();
        break;
    }
}

void ScreenSharingConnection::sendData(char type, const QByteArray &data)
{
    QByteArray header;
    header.append(HEADER_MAGIC);
    header.append(type);
    header.append(int2data(data.length(), 4));
    header.append(data);
    emit writeOnMainThread(header);
    //emit writeOnMainThread(data);
}

void ScreenSharingConnection::readyRead()
{
//    if (m_socket->state() != QAbstractSocket::ConnectedState) {
//        qDebug() << "connectionReady but error" << m_socket->state();
//        close();
//        return;
//    }
    m_buffer.append(m_socket->readAll());
    //qDebug() << toString() << "readyRead" << m_buffer.size() << m_expectedBytes;

    while (m_buffer.size() >= m_expectedBytes) {
        int expectedBytes = m_expectedBytes;
        processReceivedData(m_buffer.left(m_expectedBytes));
        m_buffer = m_buffer.mid(expectedBytes);
    }
}

void ScreenSharingConnection::stateChanged(QAbstractSocket::SocketState newstate)
{
    qDebug() << toString() << newstate;
    switch(newstate) {
    case QAbstractSocket::ConnectedState:
        break;
    case QAbstractSocket::ClosingState:
    case QAbstractSocket::UnconnectedState:
    default:
        close();
        break;
    }
}

void ScreenSharingConnection::onError(QAbstractSocket::SocketError error)
{
    qDebug() << "Error" << error;
    close();
}

void ScreenSharingConnection::onFileListUpdated(const QStringList &filelist)
{
    QByteArray data;
    foreach(QString one, filelist) {
        data.append(one.toUtf8());
        data.append('\0');
    }
    sendData(TYPE_RESPONSE_FILE_LIST, data);
}

void ScreenSharingConnection::writeData(const QByteArray &data)
{
    m_socket->write(data);
}
