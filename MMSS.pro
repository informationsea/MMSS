#-------------------------------------------------
#
# Project created by QtCreator 2014-01-22T00:39:22
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MMSS
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    imagewidget.cpp \
    screensharingconnectionmanager.cpp \
    screensharingconnection.cpp \
    sheetmessagebox.cpp \
    newconnectiondialog.cpp \
    connectionlistadapter.cpp \
    screenmanager.cpp \
    preferencedialog.cpp \
    filesharingmanger.cpp \
    sourcecodehighlighter.cpp

HEADERS  += mainwindow.h \
    imagewidget.h \
    screensharingconnectionmanager.h \
    screensharingconnection.h \
    sheetmessagebox.h \
    newconnectiondialog.h \
    connectionlistadapter.h \
    screenmanager.h \
    common.h \
    preferencedialog.h \
    filesharingmanger.h \
    sourcecodehighlighter.h

FORMS    += mainwindow.ui \
    newconnectiondialog.ui \
    preferencedialog.ui

RESOURCES += \
    resources.qrc


ICON = images/icon.icns
RC_FILE = windows.rc

OTHER_FILES += \
    windows.rc
