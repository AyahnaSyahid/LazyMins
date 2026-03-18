#pragma once

#include "src/models/ordermodel.h"
#include <QAbstractListModel>


class FinishingListModel : public QAbstractListModel
{
  Q_OBJECT

public:
  explicit FinishingListModel(QObject *p=nullptr);
  ~FinishingListModel();
  
  int rowCount(const QModelIndex& par=QModelIndex()) const override;
  int columnCount(const QModelIndex& par=QModelIndex()) const override { return 1; }
  QVariant data(const QModelIndex &ix, int role) const override;
  bool setData(const QModelIndex &ix, const QVariant& va, int role);
  
  void setList(QList<FinishingItem> *item);
  
  bool addItem(const FinishingItem& fi);
  bool removeItem(int a);

  int total() const;

  const QList<FinishingItem> *getItems() const { return m_items; }

private:
  QList<FinishingItem> *m_items;
};