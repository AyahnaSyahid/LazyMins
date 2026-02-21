#include "dashboardmodel.h"

DashboardModel::DashboardModel(QObject *p) : 
  m_dashboardItems {}, 
  QAbstractListModel (p)
{}

int DashboardModel::rowCount(const QModelIndex& parent) const
{
  if(parent.isValid()) return 0;
  return m_dashboardItems.count();
}

QVariant DashboardModel::data(const QModelIndex &ix, int role) const
{
  if (!ix.isValid() || ix.row() >= m_dashboardItems.count())
    return {};
  
  const DashboardItem &i = m_dashboardItems.at(ix.row());
  
  switch (role) {
    case DashboardItem::Roles::TypeRole:           return QVariant::fromValue(role);
    case DashboardItem::Roles::TitleRole:          return i.title;
    case DashboardItem::Roles::ValueRole:          return i.value;
    case DashboardItem::Roles::SubTextRole:        return i.subText;
    case Qt::DisplayRole:                          return i.value;
    default :                                      return {};
  }
  return QVariant();
}

QHash<int, QByteArray> DashboardModel::roleNames () const
{
  return {
    {DashboardItem::Roles::TypeRole,            "itemType"  },
    {DashboardItem::Roles::TitleRole,           "title"     },
    {DashboardItem::Roles::ValueRole,           "value"     },
    {DashboardItem::Roles::SubTextRole,         "subText"   }
    };
}

void DashboardModel::addItem(const DashboardItem &d)
{
  beginInsertRows(QModelIndex(), m_dashboardItems.count(), m_dashboardItems.count());
  m_dashboardItems << d;
  endInsertRows();
}

void DashboardModel::setItems(const QList<DashboardItem>& ldi)
{
  beginResetModel();
  m_dashboardItems = ldi;
  endResetModel();
}

void DashboardModel::updateItem(int row, const DashboardItem &di)
{
  if (row < 0 || row >= m_dashboardItems.count()) return;
  m_dashboardItems[row] = di;
  emit dataChanged(index(row), index(row));
}

const DashboardItem& DashboardModel::itemAt(int row) const
{
  return m_dashboardItems.at(row);
}