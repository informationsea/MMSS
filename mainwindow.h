#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QScreen>
#include <QList>
#include <QItemSelectionModel>
#include <QPoint>
#include <QSystemTrayIcon>
#include <QStringListModel>

#include "screensharingconnectionmanager.h"
#include "screenmanager.h"
#include "filesharingmanger.h"
#include "sourcecodehighlighter.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onClosingActiveConnetion();
    void onImageUpdate(const QPixmap &image);
    void onImageUpdateNoChange();
    void onCursorUpdate(QPoint point);
    void onFileListUpdate(const QStringList &fileList);
    void onFileContentReceived(const QString &filename, const QByteArray &content);
    void onTimerEvent();
    void onActiveSelectionChanged(QItemSelection current, QItemSelection last);

    void on_actionConnect_triggered();

    void on_actionDisconnect_triggered();

    void on_actionAbout_triggered();

    void on_actionAbout_Qt_triggered();

    void on_actionGo_to_informationsea_triggered();

    void on_actionDisplay_connection_list_triggered(bool checked);

    void on_actionToggle_Toolbar_triggered(bool checked);

    void on_actionStart_Server_triggered(bool checked);

    void on_actionPause_Screen_Update_triggered(bool checked);

    void on_actionPause_Local_Screen_Update_triggered(bool checked);

    void on_actionBlackout_Local_Screen_triggered(bool checked);

    void on_dockWidget_visibilityChanged(bool visible);

    void on_actionShow_File_List_triggered(bool checked);

    void on_dockWidgetFileList_visibilityChanged(bool visible);

    void on_actionPreferences_triggered();

    void on_shareButton_clicked();

    void on_actionQuit_triggered();

    void on_fileListView_doubleClicked(const QModelIndex &index);

    void on_actionPush_File_triggered();

    void on_actionPush_File_To_All_triggered();

private:
    Ui::MainWindow *ui;
    int m_timerCounter;
    QTimer m_timer;
    bool m_lastRequestRecieved;

    SourceCodeHighlighter *m_highlighter;
    QSystemTrayIcon m_systray;
    ScreenManager m_screenManager;
    FileSharingManger m_fileSharingManger;
    QStringListModel m_fileListModel;
    ScreenSharingConnectionManager m_connectionManager;
    ScreenSharingConnection *m_activeConnection;
};

#endif // MAINWINDOW_H
