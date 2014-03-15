#ifndef CONNECTIONLISTADAPTER_H
#define CONNECTIONLISTADAPTER_H

#include <QAbstractListModel>
#include <QModelIndex>
#include "screensharingconnectionmanager.h"

class ConnectionListAdapter : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ConnectionListAdapter(ScreenSharingConnectionManager* manager, QObject *parent = 0);

    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;

signals:

public slots:

private slots:
    void updated();

private:
    ScreenSharingConnectionManager *m_manager;

};

#endif // CONNECTIONLISTADAPTER_H
