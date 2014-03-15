#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include <QObject>
#include <QWidget>
#include <QScreen>
#include <QList>
#include <QImage>
#include <QSize>
#include <QPixmap>
#include <QTimer>
#include <QPoint>
#include <QReadWriteLock>

class ScreenManager : public QObject
{
    Q_OBJECT
public:
    explicit ScreenManager(QObject *parent = 0);

    const QByteArray &currentData(int *screenIndex);
    QSize screenSize() const;
    QPoint cursorPosition();

    void runTimer(bool state);
    void setBlackout(bool state);


signals:

public slots:

private slots:
    void onTimerEvent();
    void onTimer2Event();

private:
    bool m_blackout;
    QTimer m_timer;
    QTimer m_timer2;
    QList<QScreen *> m_screenList;
    QWidget *m_primaryScreen;
    QSize m_primaryScreenSize;
    qint64 m_lastShotTime;
    QImage m_lastImage;

    QByteArray m_blackImage;

    QReadWriteLock m_scrrenLock;
    QByteArray m_screenData;
    int m_screenDataIndex;

    QPoint m_cursor;
    QReadWriteLock m_cursorLock;

    void grabScreen(const QByteArray &type);
};

#endif // SCREENMANAGER_H
