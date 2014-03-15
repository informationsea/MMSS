#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopWidget>
#include <QDebug>
#include <QWidget>
#include <QPixmap>
#include <QScreen>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QModelIndex>
#include <QModelIndexList>
#include <QAction>
#include <QDesktopServices>
#include <QUrl>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QFileDialog>
#include "sheetmessagebox.h"
#include "newconnectiondialog.h"
#include "connectionlistadapter.h"
#include "preferencedialog.h"
#include "common.h"
#include <QFontDatabase>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), m_timerCounter(0), m_timer(this), m_lastRequestRecieved(true),
    m_systray(this), m_screenManager(this), m_fileSharingManger(this), m_fileListModel(this),
    m_connectionManager(&m_screenManager, &m_fileSharingManger, this), m_activeConnection(NULL)
{
    ui->setupUi(this);
    ui->actionAbout->setMenuRole(QAction::AboutRole);
    ui->actionAbout_Qt->setMenuRole(QAction::AboutQtRole);
    ui->actionPreferences->setMenuRole(QAction::PreferencesRole);
    ui->actionPreferences->setShortcut(QKeySequence::Preferences);
    ui->actionQuit->setMenuRole(QAction::QuitRole);
    ui->actionQuit->setShortcut(QKeySequence::Quit);
    connect(ui->actionScale_Mode, SIGNAL(triggered(bool)), ui->widget, SLOT(setScaleMode(bool)));

    // setup remote source viewer
    ui->dockWidgetSource->setVisible(false);
    m_highlighter = new SourceCodeHighlighter(ui->sourceTextView->document());
    ui->sourceTextView->document()->setDefaultFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    // setup status bar
    QString status;
    QList<QHostAddress> localaddress = QNetworkInterface::allAddresses();
    foreach(QHostAddress one, localaddress) {
        if (one.isLoopback())
            continue;
        if (one.protocol() != QAbstractSocket::IPv4Protocol)
            continue;
        status += one.toString();
        status += "  ";
    }
    statusBar()->showMessage(status);

    // setup remote file list
    ui->listView->setModel(new ConnectionListAdapter(&m_connectionManager));
    connect(ui->listView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(onActiveSelectionChanged(QItemSelection,QItemSelection)));
    ui->fileListView->setModel(&m_fileListModel);

    m_timer.setInterval(50);
    m_timer.setSingleShot(false);
    connect(&m_timer, SIGNAL(timeout()), SLOT(onTimerEvent()));
    m_timer.start();

    m_systray.setIcon(QIcon(":/images/trayicon.png"));
    QMenu *traymenu = new QMenu(this);
    traymenu->addAction(ui->actionPause_Local_Screen_Update);
    traymenu->addAction(ui->actionBlackout_Local_Screen);
    traymenu->addAction(ui->actionQuit);
    m_systray.setContextMenu(traymenu);
    m_systray.setVisible(true);
}

MainWindow::~MainWindow()
{
    m_connectionManager.closeAllConnections();
    delete ui;
    delete m_highlighter;
}

void MainWindow::onClosingActiveConnetion()
{
    disconnect(m_activeConnection);
    ui->widget->setCursorPosition(QPoint(0, 0));
    QImage nullimage;
    ui->widget->setImage(nullimage);
    m_fileListModel.setStringList(QStringList());
    m_activeConnection = NULL;
}

void MainWindow::onImageUpdate(const QPixmap &image)
{
    ui->widget->setImage(image);
    m_lastRequestRecieved = true;
}

void MainWindow::onImageUpdateNoChange()
{
    m_lastRequestRecieved = true;
}

void MainWindow::onCursorUpdate(QPoint point)
{
    ui->widget->setCursorPosition(point);
    m_lastRequestRecieved = true;
}

void MainWindow::onFileListUpdate(const QStringList &fileList)
{
    m_fileListModel.setStringList(fileList);
}

void MainWindow::onFileContentReceived(const QString &filename, const QByteArray &content)
{
    // m_fileView.close();
    // m_fileView.setFileName(filename);
    // m_fileView.setContent(content);
    // m_fileView.show();
    ui->sourceTextView->setPlainText(content);
    ui->dockWidgetSource->setWindowTitle(filename);
    ui->dockWidgetSource->setVisible(true);
}

void MainWindow::onTimerEvent()
{
    //qDebug() << m_activeConnection << m_lastRequestRecieved;
    if (m_activeConnection && m_lastRequestRecieved && !isMinimized()) {
        if (m_timerCounter >= 5) {
            m_activeConnection->requestScreenUpdate();
            m_lastRequestRecieved = false;
            m_timerCounter = 0;
        } else {
            m_activeConnection->requestCursorUpdate();
            m_timerCounter++;
        }
    }
}


void MainWindow::onActiveSelectionChanged(QItemSelection current, QItemSelection /*last*/)
{
    QModelIndexList list = current.indexes();
    if (list.count()) {
        m_activeConnection = m_connectionManager.connectionList()->at(list[0].row());
        qDebug() << "active" << m_activeConnection->toString();
    } else {
        m_activeConnection = NULL;
        qDebug() << "active" << m_activeConnection;
    }
    m_lastRequestRecieved = true;
    connect(m_activeConnection, SIGNAL(connectionClosing()), SLOT(onClosingActiveConnetion()));
    connect(m_activeConnection, SIGNAL(updateImage(QPixmap)), SLOT(onImageUpdate(QPixmap)));
    connect(m_activeConnection, SIGNAL(updateImageNoChange()), SLOT(onImageUpdateNoChange()));
    connect(m_activeConnection, SIGNAL(updateCursorPoint(QPoint)), SLOT(onCursorUpdate(QPoint)));
    connect(m_activeConnection, SIGNAL(updateFileList(QStringList)), SLOT(onFileListUpdate(QStringList)));
    connect(m_activeConnection, SIGNAL(receivedFileContent(QString,QByteArray)), SLOT(onFileContentReceived(QString,QByteArray)));
    //connect(m_activeConnection, SIGNAL(receivedPushedFileContent(QString,QByteArray)), SLOT(onPushFileContentReceived(QString,QByteArray)));
    ui->widget->setImage(m_activeConnection->currentRemotePixmap());
    m_fileListModel.setStringList(m_activeConnection->remoteFileList());
    m_timerCounter = 100;
}

void MainWindow::on_actionConnect_triggered()
{
    m_connectionManager.cleanupConnections();
    NewConnectionDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        m_connectionManager.addConnection(new ScreenSharingConnection(dialog.socket()));
    }
}

void MainWindow::on_actionDisconnect_triggered()
{
    if (m_activeConnection) {
        m_activeConnection->close();
    }
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("Multiple Mutual Screen Sharing"), tr("Multiple Mutual Screen Sharing\nVersion: 0.2\nCopyright (C) 2014 Yasunobu OKAMURA All Rights Reserved."));
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::on_actionGo_to_informationsea_triggered()
{
    QDesktopServices::openUrl(QUrl("http://informationsea.info/"));
}

void MainWindow::on_actionDisplay_connection_list_triggered(bool checked)
{
    ui->dockWidget->setVisible(checked);
}

void MainWindow::on_actionToggle_Toolbar_triggered(bool checked)
{
    ui->mainToolBar->setVisible(checked);
}

void MainWindow::on_actionStart_Server_triggered(bool checked)
{
    if (!checked && m_connectionManager.isServerRunning()) {
        m_connectionManager.stopServer();
        //ui->actionStart_Server->setText(tr("Start Server"));
    } else if (checked && !m_connectionManager.isServerRunning()) {
        if (m_connectionManager.startServer()) {
            //ui->actionStart_Server->setText(tr("Stop Server"));
        } else {
            SheetMessageBox::critical(this, tr("Cannot start server"), tr("Failed to start server"));
            ui->actionStart_Server->setChecked(false);
        }
    }
}

void MainWindow::on_actionPause_Screen_Update_triggered(bool checked)
{
    if (checked) {
        m_timer.stop();
        //ui->actionPause_Screen_Update->setText(tr("Resume Remote Screen Update"));
    } else {
        m_timer.start();
        //ui->actionPause_Screen_Update->setText(tr("Pause Remote Screen Update"));
    }
}

void MainWindow::on_actionPause_Local_Screen_Update_triggered(bool checked)
{
    m_screenManager.runTimer(!checked);
}


void MainWindow::on_actionBlackout_Local_Screen_triggered(bool checked)
{
    m_screenManager.setBlackout(checked);
}

void MainWindow::on_dockWidget_visibilityChanged(bool visible)
{
    ui->actionDisplay_connection_list->setChecked(visible);
}

void MainWindow::on_actionShow_File_List_triggered(bool checked)
{
    ui->dockWidgetFileList->setVisible(checked);
}

void MainWindow::on_dockWidgetFileList_visibilityChanged(bool visible)
{
    ui->actionShow_File_List->setChecked(visible);
}

void MainWindow::on_actionPreferences_triggered()
{
    PreferenceDialog dialog(this);
    dialog.exec();
}

void MainWindow::on_shareButton_clicked()
{
    static const char *SETTINGS_SHAREPOINT = "SETTINGS_SHAREPOINT";
    QString dirpath = QFileDialog::getExistingDirectory(this, tr("Select Share Directory"),
                                                        g_settings->value(SETTINGS_SHAREPOINT, QDir::home().canonicalPath()).toString());
    if (dirpath.isNull())
        return;
    g_settings->setValue(SETTINGS_SHAREPOINT, dirpath);
    QDir dir(dirpath);
    ui->sharePathLabel->setText(dir.dirName());
    m_fileSharingManger.setSharingPoint(dir);
}

void MainWindow::on_actionQuit_triggered()
{
    close();
}
void MainWindow::on_fileListView_doubleClicked(const QModelIndex &index)
{
    if (m_activeConnection)
        m_activeConnection->requestFile(m_fileListModel.stringList()[index.row()]);
}

void MainWindow::on_actionPush_File_triggered()
{
    if (!m_activeConnection)
        return;
    QString path = QFileDialog::getOpenFileName(this, tr("Select push file"), QDir::homePath());
    if (path.isEmpty())
        return;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        SheetMessageBox::critical(this, tr("Cannot open file"), file.errorString());
        return;
    }
    QByteArray data(file.readAll());
    m_activeConnection->pushFileContent(QFileInfo(path).fileName(), data);
}

void MainWindow::on_actionPush_File_To_All_triggered()
{
    if (!m_activeConnection)
        return;
    QString path = QFileDialog::getOpenFileName(this, tr("Select push file"), QDir::homePath());
    if (path.isEmpty())
        return;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        SheetMessageBox::critical(this, tr("Cannot open file"), file.errorString());
        return;
    }
    QByteArray data(file.readAll());
    m_connectionManager.pushFileToAll(QFileInfo(path).fileName(), data);
}
