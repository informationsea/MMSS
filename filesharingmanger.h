#ifndef FILESHARINGMANGER_H
#define FILESHARINGMANGER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTimer>
#include <QStringList>

class FileSharingManger : public QObject
{
    Q_OBJECT
public:
    explicit FileSharingManger(QObject *parent = 0);
    void setSharingPoint(QDir path);

    const QStringList& fileList() const {return m_fileList;}
    QByteArray fileContent(const QString &filename);

signals:
    void fileListUpdated(const QStringList &filelist);

public slots:

private slots:
    void onTimerEvent();

private:
    void recursiveScanner(QDir rootdir, QDir parent, QStringList *list);

    QTimer m_refreshTimer;
    QDir m_sharingPoint;
    QStringList m_fileList;
    QStringList m_nameFilter;
};

#endif // FILESHARINGMANGER_H
