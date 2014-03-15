#include "screenmanager.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QDateTime>
#include <QImage>
#include <QBuffer>
#include <QByteArray>
#include <QDebug>
#include <QCursor>
#include <QPainter>
#include <QImage>
#include <QReadLocker>
#include <QWriteLocker>

#define SCREEN_UPDATE_INTERVAL 800
#define CURSOR_UPDATE_INTERVAL 10

ScreenManager::ScreenManager(QObject *parent) :
    QObject(parent), m_blackout(false), m_timer(this),
    m_screenList(qApp->screens()), m_primaryScreen(qApp->desktop()->screen()),
    m_screenDataIndex(0)
{
    m_timer.setInterval(SCREEN_UPDATE_INTERVAL);
    m_timer.setSingleShot(false);
    m_timer.start();
    m_timer2.setInterval(CURSOR_UPDATE_INTERVAL);
    m_timer2.setSingleShot(false);
    m_timer2.start();
    connect(&m_timer, SIGNAL(timeout()), SLOT(onTimerEvent()));
    connect(&m_timer2, SIGNAL(timeout()), SLOT(onTimer2Event()));
    m_primaryScreenSize = m_primaryScreen->size();

    QImage black(1, 1, QImage::Format_Mono);
    QPainter painter(&black);
    painter.fillRect(QRect(0, 0, 1, 1), Qt::black);
    QBuffer buffer;
    black.save(&buffer, "JPEG");
    m_blackImage = buffer.data();
}

const QByteArray &ScreenManager::currentData(int *screenIndex)
{
    static const QByteArray empty;
    if (m_blackout)
        return m_blackImage;

    QReadLocker locker(&m_scrrenLock);
    if (*screenIndex == m_screenDataIndex)
        return empty;
    *screenIndex = m_screenDataIndex;
    return m_screenData;
}

QSize ScreenManager::screenSize() const
{
    return m_primaryScreenSize;
}

QPoint ScreenManager::cursorPosition()
{
    QReadLocker locker(&m_cursorLock);
    return m_cursor;
}

void ScreenManager::runTimer(bool state)
{
    if (state) {
        m_timer.start();
        m_timer2.start();
    } else {
        m_timer.stop();
        m_timer2.stop();
        grabScreen("PNG");
    }
}

void ScreenManager::setBlackout(bool state)
{
    m_blackout = state;
}

void ScreenManager::onTimerEvent()
{
    if (!m_blackout) {
        grabScreen("JPEG");
    }
    //qDebug() << QCursor::pos() << m_primaryScreen->cursor();
}

void ScreenManager::onTimer2Event()
{
    QWriteLocker locker(&m_cursorLock);
    m_cursor = QCursor::pos(m_screenList[0]);
}

void ScreenManager::grabScreen(const QByteArray &type)
{
    QBuffer buffer;
    m_lastImage = m_screenList[0]->grabWindow(m_primaryScreen->winId()).toImage();
    m_lastImage.save(&buffer, type, 20);

    QWriteLocker locker(&m_scrrenLock);
    m_screenData = buffer.data();
    m_screenDataIndex += 1;
}
