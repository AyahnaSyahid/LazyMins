#include "finishinglistmodel.h"

FinishingListModel::FinishingListModel(QObject *p) :
  m_items {} , QAbstractListModel(p) {}

FinishingListModel::~FinishingListModel() {}

int FinishingListModel::rowCount(const QModelIndex& par) const {
  if(par.isValid()) return 0;
  return m_items.count();
}

QVariant FinishingListModel::data(const QModelIndex &ix, int role) const {
  if(!ix.isValid()) return QVariant();
  auto item = (*m_items)[ix.row()];
  switch (role) {
    case Qt::UserRole + 1: {
      return item.id;
    }
    case Qt::UserRole + 2: {
      return item.order_item_id;
    }
    case Qt::UserRole + 3: {
      return item.finishing_id;
    }
    case Qt::UserRole + 4: {
      return item.finishing_name;
    }
    case Qt::UserRole + 5: {
      return item.quantity;
    }
    case Qt::UserRole + 6: {
      return item.finishing_price;
    }
    case Qt::UserRole + 7: {
      return item.subtotal();
    }
    default:
      return item.finishing_name;
  }
}

void FinishingListModel::setList(QList<FinishingItem> *list) {
  beginResetModel();
  m_items = list;
  endResetModel();
}

bool FinishingListModel::setData(const QModelIndex &ix, const QVariant& va, int role) {
  if(!ix.isValid()) return false;
  auto &item = (*m_items)[ix.row()];
  auto meta = va.metaType();
  switch (role) {
    case Qt::UserRole + 1: {
      if(meta.id() == QMetaType::Int || meta.id() == QMetaType::LongLong || meta.id() == QMetaType::Long) {
        item.id = va.toInt();
        break;
      }
      return false;
    }
    case Qt::UserRole + 2: {
      if(meta.id() == QMetaType::Int || meta.id() == QMetaType::LongLong || meta.id() == QMetaType::Long) {
        item.order_item_id = va.toInt();
        break;
      }
      return false;
    }
    case Qt::UserRole + 3: {
      if(meta.id() == QMetaType::Int || meta.id() == QMetaType::LongLong || meta.id() == QMetaType::Long) {
        item.finishing_id = va.toInt();
        break;
      }
      return false;
    }
    case Qt::UserRole + 4: {
      if(meta.id() == QMetaType::QString) {
        item.finishing_name = va.toString();
        break;
      }
      return false;
    }
    case Qt::UserRole + 5: {
      if(meta.id() == QMetaType::Int || meta.id() == QMetaType::LongLong || meta.id() == QMetaType::Long) {
        item.quantity = va.toInt();
        break;
      }
      return false;
    }
    case Qt::UserRole + 6: {
      if(meta.id() == QMetaType::Int || meta.id() == QMetaType::LongLong || meta.id() == QMetaType::Long) {
        item.finishing_price = va.toInt();
        break;
      }
      return false;
    }
    default:
      return false;
  }
  emit dataChanged(ix, ix, {role});
  return true;
}

int FinishingListModel::total() const {
  if(!m_items) return 0;
  if(m_items.count() < 1) return 0;
  int t = 0;
  for(auto const& item : m_items) {
    t += item.subtotal();
  }
  return t;
}

bool FinishingListModel::addItem(const FinishingItem &fi)
{
  if (!m_items) return false;
  beginInsertRows(QModelIndex(), rowCount(), rowCount() + 1);
  m_items->push_back(fi);
  endInsertRows();
  return true;
}

bool FinishingListModel::removeItem(int at) {
  if (!m_items) return false;
  if(at >=0 && at < rowCount()) {
    beginRemoveRows(QModelIndex(), at, at);
    m_items->remove(at, 1);
    endRemoveRows();
  }
  return true;
}