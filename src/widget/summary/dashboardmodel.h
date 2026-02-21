#pragma once

#include "dashboarditem.h"
#include <QAbstractListModel>

class DashboardModel : public QAbstractListModel
{
  Q_OBJECT
  
  public:
    DashboardModel(QObject *p=nullptr);

    int rowCount (const QModelIndex& ix = QModelIndex()) const override;
    QVariant data(const QModelIndex& ix, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames () const override;
    
    void addItem(const DashboardItem& i);
    void setItems(const QList<DashboardItem> &iss);
    void clear();
    
    void updateItem(int row, const DashboardItem &di);
    const DashboardItem& itemAt(int row) const;
  
  private :
    QList<DashboardItem> m_dashboardItems;
};