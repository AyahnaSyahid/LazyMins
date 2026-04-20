#include "invoicecomposermodel.h"
#include "src/managers/managers.h"

InvoiceComposerModel::InvoiceComposerModel(QObject *p):
m_orders(), QAbstractListModel(p)
{}

InvoiceComposerModel::~InvoiceComposerModel() {}


bool InvoiceComposerModel::loadInvoice(int invoiceId) {
  // Loading nanti saja
  return false;
}

int InvoiceComposerModel::rowCount(const QModelIndex& parent) const {
  if(parent.isValid()) return 0;
  return m_orders.size();
};

QVariant InvoiceComposerModel::data(const QModelIndex& ix, int role) const {
  if(!ix.isValid()) return QVariant();
  int row = ix.row();
  if (row >= m_orders.size()) return QVariant();
  SavedOrder savedOrder = m_orders.at(row);
  switch (role) {
    case IdRole:
      return savedOrder.id;
    case NumberRole:
      return savedOrder.order_number; 
    case SubtotalRole:
      return savedOrder.subtotal;
    case DiscountRole:
      return savedOrder.discount_amount;
    case TotalRole:
      return savedOrder.total_amount;
    default :
      return QVariant();
  }
};

bool InvoiceComposerModel::setData(const QModelIndex& ix, const QVariant& val, int role) {
  if (!ix.isValid()) return false;
  if (role == IdRole) return false;
  qWarning() << "This function isnt implemented yet";
  return false;
}


void InvoiceComposerModel::insertOrder(int order_id) {

  SavedOrder imp;
  imp.loadFromId(order_id);
  
  if(imp.id < 1) return;
  
  imp.validate();
  
  auto newRow = m_orders.size();
  beginInsertRows(QModelIndex(), newRow, newRow);
  m_orders << imp;
  endInsertRows();
}

void InvoiceComposerModel::removeOrder(int order_id) {
  if(!m_orders.size()) return;
  
  for(int r = 0; r < m_orders.size(); ++r) {
    if (m_orders.at(r).id == order_id) {
      beginRemoveRows(QModelIndex(), r, r);
      m_orders.remove(r);
      endRemoveRows();
      return ;
    }
  }
}

QList<int> InvoiceComposerModel::imported() const {
  QList<int> imp;
  for(auto const& so : m_orders) imp << so.id ;
  return imp;
}

void InvoiceComposerModel::clearOrders()
{
  beginResetModel();
  m_orders.clear();
  endResetModel();
}

int InvoiceComposerModel::subtotal() const
{
  auto ret = 0;
  for(auto const& order : m_orders) {
    ret += order.subtotal;
  }
  return ret;
}

int InvoiceComposerModel::discount() const
{
  auto ret = 0;
  for(auto const& order : m_orders) {
    ret += order.discount_amount;
  }
  return ret;
}

SavedOrder &SavedOrder::loadFromId(int sid)
{
  OrderManager orderManager;
  auto opt_ord = orderManager.getById(sid);
  if (opt_ord.has_value()) {
    id              = (*opt_ord).value("id").toInt();
    subtotal        = (*opt_ord).value("subtotal").toInt();
    total_amount    = (*opt_ord).value("total_amount").toInt();
    discount_amount = (*opt_ord).value("discount_amount").toInt();
    order_number    = (*opt_ord).value("order_number").toString();
  } else {
    *this = SavedOrder(); // reset all to the new
  }
  qDebug() << "SaveOrder : loaded id : " << id; 
  return *this;
}

void SavedOrder::validate() {
  auto calc_total = subtotal - discount_amount;
  if (total_amount != calc_total) {
    qWarning() << "Invalid total amount detected : Now Fixed";
    total_amount = calc_total;
  }
}
