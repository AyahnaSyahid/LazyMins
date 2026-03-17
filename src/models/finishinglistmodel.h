#pragma once

#include "src/models/ordermodel.h"
#include <QAbstractListModel>


class FinishingListModel : public QAbstractListModel
{
  Q_OBJECT

public:
  explicit FinishingListModel(QObject *p=nullptr);
  ~FinishingListModel();
  
  void setItems(QList<FinishingItem> *finishings);
  
  int rowCount(const QModelIndex& par=QModelIndex()) const override;
  QVariant data(const QModelIndex &ix, int role) const override;
  bool setData(const QModelIndex &ix, const QVariant& va, int role);
  
  bool addItem(const FinishingItem& fi);
  bool removeItem(int a);

  int total() const;

  const QList<FinishingItem> getItems() const { return *m_items; }
  
private:
  QList<FinishingItem> *m_items;
};