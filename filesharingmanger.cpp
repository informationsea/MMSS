#include "filesharingmanger.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>

FileSharingManger::FileSharingManger(QObject *parent) :
    QObject(parent), m_refreshTimer(this)
{
    m_refreshTimer.setInterval(5000);
    m_refreshTimer.setSingleShot(false);
    connect(&m_refreshTimer, SIGNAL(timeout()), SLOT(onTimerEvent()));

    m_nameFilter << ".h" << ".cpp" << ".c" << ".ui" << ".pro"
                 << ".mk" << ".m" << ".cs" << ".py" << ".rb"
                 << ".txt" << ".hex" << ".pl" << ".am" << ".ac"
                 << ".html" << ".md" << ".rst";
}

void FileSharingManger::setSharingPoint(QDir path)
{
    m_sharingPoint = path;
    if (!m_refreshTimer.isActive())
        m_refreshTimer.start();
    onTimerEvent();
}

QByteArray FileSharingManger::fileContent(const QString &filename)
{
    QFileInfo requestFile(m_sharingPoint, filename);
    if (!requestFile.canonicalFilePath().startsWith(m_sharingPoint.absolutePath()))
        return QByteArray();

    QFile file(requestFile.absoluteFilePath());
    //qDebug() << "OK" << file.isReadable() << requestFile.isFile();
    if (!file.open(QIODevice::ReadOnly)) {
        //qDebug() << "Error" << file.errorString();
        return QByteArray();
    }

    //qDebug() << "OK2";

    QByteArray data(file.readAll());
    file.close();
    return data;
}

void FileSharingManger::onTimerEvent()
{
    QStringList templist;
    recursiveScanner(QDir(m_sharingPoint.absolutePath()), QDir(m_sharingPoint.absolutePath()), &templist);
    if (m_fileList != templist) {
        qDebug() << "file list updated";
        m_fileList = templist;
        emit fileListUpdated(m_fileList);
    }
}

void FileSharingManger::recursiveScanner(QDir rootdir, QDir parent, QStringList *list)
{
    QFileInfoList fileList = parent.entryInfoList();
    foreach(QFileInfo onefile, fileList) {
        if (onefile.fileName() == "." || onefile.fileName() == "..")
            continue;
        //qDebug() << onefile.absoluteFilePath() << onefile.fileName();
        if (onefile.isDir()) {
            recursiveScanner(rootdir, QDir(onefile.absoluteFilePath()), list);
        } else {
            foreach(QString suffix, m_nameFilter) {
                //qDebug() << onefile.fileName() << suffix << onefile.fileName().endsWith(suffix);
                if (onefile.fileName().endsWith(suffix) || onefile.fileName() == "Makefile") {
                    *list << rootdir.relativeFilePath(onefile.absoluteFilePath());
                    break;
                }
            }
        }
    }
}
