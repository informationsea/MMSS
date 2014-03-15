#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QUuid>

#include "common.h"

QSettings *g_settings;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("MMSS");
    a.setOrganizationDomain("info.informationsea.qt");
    a.setOrganizationName("Informationsea");

    //qDebug() << a.platformName() << a.applicationFilePath() << a.libraryPaths();

    g_settings = new QSettings(&a);

    if (!g_settings->contains(SETTINGS_ALLOW_IPS)) {
        QStringList iplist;
        iplist << "127.0.0.1/8" << "::1" << "10.0.0.0/8" << "172.16.0.0/12" << "192.168.0.0/16" << "169.254.0.0/16" << "fe80::/10";
        g_settings->setValue(SETTINGS_ALLOW_IPS, iplist);
    }

    if (!g_settings->contains(SETTINGS_UUID)) {
        g_settings->setValue(SETTINGS_UUID, QUuid::createUuid().toString());
    }

    if (!g_settings->contains(SETTINGS_PRESHAREDKEY)) {
        g_settings->setValue(SETTINGS_PRESHAREDKEY, "DEFAULT_SHARED_KEY");
    }

    if (!g_settings->contains(SETTINGS_NAME)) {
        g_settings->setValue(SETTINGS_NAME, "Participant");
    }

    //QUuid uuid(g_settings->value(SETTINGS_UUID).toString());
    //qDebug() << uuid.toByteArray() << uuid.toRfc4122().length();

    MainWindow w;
    w.show();

    return a.exec();
}
