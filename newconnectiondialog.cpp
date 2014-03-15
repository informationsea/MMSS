#include "newconnectiondialog.h"
#include "ui_newconnectiondialog.h"

#include <QTcpSocket>
#include "sheetmessagebox.h"
#include <QDebug>

#include "common.h"

#define CONNECT_HOST "CONNECT_HOST"

NewConnectionDialog::NewConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewConnectionDialog), m_socket(NULL)
{
    ui->setupUi(this);
    setWindowModality(Qt::WindowModal);
    ui->lineEdit->setText(g_settings->value(CONNECT_HOST, "").toString());
}

NewConnectionDialog::~NewConnectionDialog()
{
    delete ui;
}

QTcpSocket *NewConnectionDialog::socket()
{
    return m_socket;
}

void NewConnectionDialog::accept()
{
    QTcpSocket *sock = new QTcpSocket();
    sock->connectToHost(ui->lineEdit->text(), CONNECT_PORT);
    connect(sock, SIGNAL(connected()), SLOT(connected()));
    connect(sock, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(error(QAbstractSocket::SocketError)));
    m_socket = sock;
    ui->buttonBox->setEnabled(false);
}

void NewConnectionDialog::connected()
{
    qDebug() << "accepted" << m_socket->state();
    g_settings->setValue(CONNECT_HOST, ui->lineEdit->text());
    QDialog::accept();
}

void NewConnectionDialog::error(QAbstractSocket::SocketError socketError)
{
    qDebug() << m_socket->errorString() << "refused" << socketError;
    SheetMessageBox::critical(this, tr("Cannot connect"), m_socket->errorString());
    delete m_socket;
    m_socket = NULL;
    ui->buttonBox->setEnabled(true);
}
