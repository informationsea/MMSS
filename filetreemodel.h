#ifndef FILETREEMODEL_H
#define FILETREEMODEL_H

#include <QAbstractItemModel>
#include <QJsonDocument>

class FileTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit FileTreeModel(QObject *parent = 0);

signals:

public slots:

public:
    static QJsonDocument

};

#endif // FILETREEMODEL_H
