#include "connectionlistadapter.h"

#include <QTcpSocket>

ConnectionListAdapter::ConnectionListAdapter(ScreenSharingConnectionManager *manager, QObject *parent) :
    QAbstractListModel(parent)
{
    m_manager = manager;
    connect(manager, SIGNAL(connectionAdded(ScreenSharingConnection*)), SLOT(updated()));
    connect(manager, SIGNAL(connectionClosing(ScreenSharingConnection*)), SLOT(updated()));
}

int ConnectionListAdapter::columnCount(const QModelIndex & /*parent*/) const
{
    return 1;
}

QVariant ConnectionListAdapter::data(const QModelIndex &index, int role) const
{
    const QTcpSocket *socket = m_manager->connectionList()->at(index.row())->socket();
    switch(role) {
    case Qt::DisplayRole:
        return QString("%5 %1[%2] <- %3[%4]").arg(socket->peerAddress().toString(), QString::number(socket->peerPort()),
                                                  socket->localAddress().toString(), QString::number(socket->localPort()),
                                                  m_manager->connectionList()->at(index.row())->peerName());
    }
    return QVariant();
}

void ConnectionListAdapter::updated()
{
    beginResetModel();
    endResetModel();
}

int ConnectionListAdapter::rowCount(const QModelIndex &/*parent*/) const
{
    return m_manager->connectionList()->count();
}
