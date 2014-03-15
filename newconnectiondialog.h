#ifndef NEWCONNECTIONDIALOG_H
#define NEWCONNECTIONDIALOG_H

#include <QDialog>
#include <QTcpSocket>

namespace Ui {
class NewConnectionDialog;
}

class NewConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewConnectionDialog(QWidget *parent = 0);
    ~NewConnectionDialog();
    QTcpSocket *socket();

public slots:
    void accept();

    void connected();
    void error(QAbstractSocket::SocketError socketError);

private slots:


private:
    Ui::NewConnectionDialog *ui;
    QTcpSocket *m_socket;
};

#endif // NEWCONNECTIONDIALOG_H
