#include "ordermodel.h"

#include "src/managers/productmanager.h"
#include "src/managers/orderitemfinishingmanager.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>

// ============================================================================
// Helpers – pure in-memory recalculation
// ============================================================================

void OrderModel::recalcFinishingTotal(int row)
{
    OrderItem &item = m_items[row];
    int total = 0;
    for (const FinishingItem &fi : item.finishings)
        total += fi.subtotal();
    item.finishing_total = total;

    const QModelIndex ix = index(row);
    emit dataChanged(ix, ix, {Qt::UserRole + 14, Qt::UserRole + 16, Qt::UserRole + 17});
}

void OrderModel::recalcOrderTotal()
{
    int total = 0;
    for (const OrderItem &it : m_items)
        total += it.total();
    emit orderTotalChanged(total);
}

// ============================================================================
// OrderModel – QAbstractListModel overrides
// ============================================================================

OrderModel::~OrderModel() = default;

int OrderModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_items.size();
}

QVariant OrderModel::data(const QModelIndex &ix, int role) const
{
    if (!ix.isValid() || ix.row() < 0 || ix.row() >= m_items.size())
        return {};

    const OrderItem &item = m_items.at(ix.row());

    switch (role) {
    case Qt::DisplayRole:   return QString("%1 x %2").arg(item.product_name).arg(item.quantity);
    case Qt::UserRole + 0:  return item.id;
    case Qt::UserRole + 1:  return item.order_id;
    case Qt::UserRole + 2:  return item.product_id;
    case Qt::UserRole + 3:  return item.product_name;
    case Qt::UserRole + 4:  return item.sku;
    case Qt::UserRole + 5:  return item.quantity;
    case Qt::UserRole + 6:  return item.unit;
    case Qt::UserRole + 7:  return item.size_width;
    case Qt::UserRole + 8:  return item.size_height;
    case Qt::UserRole + 9:  return item.use_area;
    case Qt::UserRole + 10: return item.sale_price;
    case Qt::UserRole + 11: return item.base_price;
    case Qt::UserRole + 12: return item.discount_percentage;
    case Qt::UserRole + 13: return item.discount_amount;
    case Qt::UserRole + 14: return item.finishing_total;
    case Qt::UserRole + 15: return item.notes;
    case Qt::UserRole + 16: return item.subtotal();
    case Qt::UserRole + 17: return item.total();
    case Qt::UserRole + 18: return m_dirtyRows.contains(ix.row());
    default: break;
    }
    return {};
}

bool OrderModel::setData(const QModelIndex &ix, const QVariant &va, int role)
{
    if (!ix.isValid() || ix.row() < 0 || ix.row() >= m_items.size())
        return false;

    OrderItem &item = m_items[ix.row()];

    switch (role) {
    case Qt::UserRole + 3:  item.product_name        = va.toString(); break;
    case Qt::UserRole + 4:  item.sku                 = va.toString(); break;
    case Qt::UserRole + 5:  item.quantity            = va.toInt();    break;
    case Qt::UserRole + 6:  item.unit                = va.toString(); break;
    case Qt::UserRole + 7:  item.size_width          = va.toReal();   break;
    case Qt::UserRole + 8:  item.size_height         = va.toReal();   break;
    case Qt::UserRole + 9:  item.use_area            = va.toBool();   break;
    case Qt::UserRole + 10: item.sale_price          = va.toInt();    break;
    case Qt::UserRole + 11: item.base_price          = va.toInt();    break;
    case Qt::UserRole + 12: item.discount_percentage = va.toReal();   break;
    case Qt::UserRole + 13: item.discount_amount     = va.toInt();    break;
    // UserRole+14 (finishing_total) is derived – use addFinishing/removeFinishing.
    case Qt::UserRole + 15: item.notes               = va.toString(); break;
    default:
        return false;
    }

    m_dirtyRows.insert(ix.row());
    emit dataChanged(ix, ix, {role, Qt::UserRole + 16, Qt::UserRole + 17, Qt::UserRole + 18});
    recalcOrderTotal();
    return true;
}

// ============================================================================
// OrderModel – header
// ============================================================================

void OrderModel::setHeader(const OrderHeader &header)
{
    m_header      = header;
    m_headerDirty = true;
    // A new order has no DB id yet – ensure this is reflected.
    m_orderId     = -1;
    m_header.id   = -1;
}

void OrderModel::setHeaderField(const OrderHeader &header)
{
    m_header      = header;
    m_headerDirty = true;
}

// ============================================================================
// OrderModel – load
// ============================================================================

bool OrderModel::loadOrder(int orderId, QSqlDatabase &db)
{
    // Load the order header first.
    QSqlQuery hq(db);
    hq.prepare(R"(
        SELECT id, order_number,
               admin_id, customer_id, customer_name, customer_phone,
               price_level_id,
               discount_amount, discount_percentage,
               status, priority,
               order_date, deadline_date, completion_date,
               notes, internal_notes
        FROM   orders
        WHERE  id = :id
    )");
    hq.bindValue(":id", orderId);
    if (!hq.exec() || !hq.next()) {
        qWarning() << "OrderModel::loadOrder – header query failed:" << hq.lastError().text();
        return false;
    }

    // Load items.
    QSqlQuery iq(db);
    iq.prepare("SELECT id FROM order_items WHERE order_id = :oid ORDER BY id");
    iq.bindValue(":oid", orderId);
    if (!iq.exec()) {
        qWarning() << "OrderModel::loadOrder – items query failed:" << iq.lastError().text();
        return false;
    }

    beginResetModel();

    m_header.id                 = hq.value("id").toInt();
    m_header.order_number       = hq.value("order_number").toString();
    m_header.admin_id           = hq.value("admin_id").toInt();
    m_header.customer_id        = hq.value("customer_id").isNull() ? -1 : hq.value("customer_id").toInt();
    m_header.customer_name      = hq.value("customer_name").toString();
    m_header.customer_phone     = hq.value("customer_phone").toString();
    m_header.price_level_id     = hq.value("price_level_id").toInt();
    m_header.discount_amount    = hq.value("discount_amount").toInt();
    m_header.discount_percentage= hq.value("discount_percentage").toInt();
    // m_header.tax_amount         = hq.value("tax_amount").toInt();
    // m_header.status             = hq.value("status").toString();
    m_header.priority           = hq.value("priority").toString();
    // m_header.payment_status     = hq.value("payment_status").toString();
    m_header.order_date         = hq.value("order_date").toDateTime();
    m_header.deadline_date      = hq.value("deadline_date").toDateTime();
    m_header.completion_date    = hq.value("completion_date").toDateTime();
    m_header.notes              = hq.value("notes").toString();
    m_header.internal_notes     = hq.value("internal_notes").toString();

    m_items.clear();
    m_dirtyRows.clear();
    m_deletedItemIds.clear();
    m_deletedFinishingIds.clear();
    m_headerDirty = false;
    m_orderId     = orderId;

    while (iq.next()) {
        OrderItem item;
        item.loadFromId(iq.value(0).toInt());
        m_items.append(std::move(item));
    }

    endResetModel();
    recalcOrderTotal();
    return true;
}

// ============================================================================
// OrderModel – item mutation (in-memory only)
// ============================================================================

QModelIndex OrderModel::addItem(OrderItem item)
{
    // Allow adding items even before the order has a DB id –
    // commit() will create the order row first.
    item.order_id = m_orderId;   // may still be -1; commit() will fix it up

    const int newRow = m_items.size();
    beginInsertRows(QModelIndex(), newRow, newRow);
    m_items.append(std::move(item));
    endInsertRows();

    m_dirtyRows.insert(newRow);
    recalcOrderTotal();
    return index(newRow);
}

bool OrderModel::removeItem(int row)
{
    if (row < 0 || row >= m_items.size()) {
        qWarning() << "OrderModel::removeItem – row" << row << "out of range";
        return false;
    }

    const int itemId = m_items.at(row).id;
    if (itemId != -1)
        m_deletedItemIds.append(itemId);

    m_dirtyRows.remove(row);
    m_deletedFinishingIds.remove(row);

    // Re-index tracking sets: rows above `row` shift down by one.
    QSet<int> adjustedDirty;
    for (int r : m_dirtyRows)
        adjustedDirty.insert(r > row ? r - 1 : r);
    m_dirtyRows = adjustedDirty;

    QMap<int, QList<int>> adjustedFinDel;
    for (auto it = m_deletedFinishingIds.begin(); it != m_deletedFinishingIds.end(); ++it)
        adjustedFinDel.insert(it.key() > row ? it.key() - 1 : it.key(), it.value());
    m_deletedFinishingIds = adjustedFinDel;

    beginRemoveRows(QModelIndex(), row, row);
    m_items.removeAt(row);
    endRemoveRows();

    recalcOrderTotal();
    return true;
}

// ============================================================================
// OrderModel – finishing mutation (in-memory only)
// ============================================================================

bool OrderModel::addFinishing(int row, FinishingItem finishing)
{
    if (row < 0 || row >= m_items.size()) {
        qWarning() << "OrderModel::addFinishing – row" << row << "out of range";
        return false;
    }
    finishing.id = -1;
    m_items[row].finishings.append(finishing);
    m_dirtyRows.insert(row);
    recalcFinishingTotal(row);
    recalcOrderTotal();
    return true;
}

bool OrderModel::updateFinishing(int row, int finishingIndex, const FinishingItem &finishing)
{
    if (row < 0 || row >= m_items.size()) {
        qWarning() << "OrderModel::updateFinishing – row" << row << "out of range";
        return false;
    }
    QList<FinishingItem> &list = m_items[row].finishings;
    if (finishingIndex < 0 || finishingIndex >= list.size()) {
        qWarning() << "OrderModel::updateFinishing – finishingIndex" << finishingIndex << "out of range";
        return false;
    }
    FinishingItem &fi = list[finishingIndex];
    fi.finishing_id    = finishing.finishing_id;
    fi.finishing_name  = finishing.finishing_name;
    fi.quantity        = finishing.quantity;
    fi.finishing_price = finishing.finishing_price;
    m_dirtyRows.insert(row);
    recalcFinishingTotal(row);
    recalcOrderTotal();
    return true;
}

bool OrderModel::removeFinishing(int row, int finishingIndex)
{
    if (row < 0 || row >= m_items.size()) {
        qWarning() << "OrderModel::removeFinishing – row" << row << "out of range";
        return false;
    }
    QList<FinishingItem> &list = m_items[row].finishings;
    if (finishingIndex < 0 || finishingIndex >= list.size()) {
        qWarning() << "OrderModel::removeFinishing – finishingIndex" << finishingIndex << "out of range";
        return false;
    }
    const int fiId = list.at(finishingIndex).id;
    if (fiId != -1)
        m_deletedFinishingIds[row].append(fiId);
    list.removeAt(finishingIndex);
    m_dirtyRows.insert(row);
    recalcFinishingTotal(row);
    recalcOrderTotal();
    return true;
}

// ============================================================================
// OrderModel – unit-of-work
// ============================================================================

bool OrderModel::isDirty() const
{
    return m_headerDirty
        || !m_dirtyRows.isEmpty()
        || !m_deletedItemIds.isEmpty()
        || !m_deletedFinishingIds.isEmpty();
}

bool OrderModel::commit(QSqlDatabase &db)
{
    if (!isDirty())
        return true;

    // Validate before touching the DB at all.
    if (!m_header.isValid()) {
        qWarning() << "OrderModel::commit – header is invalid"
                      " (admin_id or customer_name missing)";
        return false;
    }

    if (!db.transaction()) {
        qWarning() << "OrderModel::commit – could not begin transaction:"
                   << db.lastError().text();
        return false;
    }

    // ── Step 1: INSERT or UPDATE the orders row ───────────────────────────────

    // Calculate current subtotal from in-memory items.
    int subtotal = 0;
    for (const OrderItem &it : m_items)
        subtotal += it.total();
      
    OrderManager oman;
    if (m_orderId == -1) {
        QVariantMap createParams {
          {"admin_id",           m_header.admin_id},
          {"customer_id",        m_header.customer_id == -1 ? QVariant() : QVariant(m_header.customer_id)},
          {"customer_name",      m_header.customer_name},
          {"customer_phone",     m_header.customer_phone.isEmpty() ? QVariant() : QVariant(m_header.customer_phone)},
          {"price_level_id",     m_header.price_level_id},
          {"subtotal",           subtotal},
          {"discount_amount",    m_header.discount_amount},
          {"discount_percentage",m_header.discount_percentage},
          // {"tax_amount",         m_header.tax_amount},
          // {"status",             m_header.status},
          {"priority",           m_header.priority},
          // {"payment_status",     m_header.payment_status},
          {"order_date",         m_header.order_date.isNull() ? QVariant() : QVariant(m_header.order_date)},
          {"deadline_date",      m_header.deadline_date.isNull() ? QVariant() : QVariant(m_header.deadline_date)},
          {"notes",              m_header.notes.isEmpty() ? QVariant() : QVariant(m_header.notes)},
          {"internal_notes",     m_header.internal_notes.isEmpty() ? QVariant() : QVariant(m_header.internal_notes)},
          {"order_number",       m_header.order_number}
        };
        
        auto opt_order = oman.create(createParams);
        if (!opt_order.has_value()) {
            qWarning() << "OrderModel::commit – INSERT orders failed:"
                       << oman.errorString();
            db.rollback();
            return false;
        }
        
        m_orderId    = (*opt_order).value("id").toInt();
        m_header.id  = m_orderId;
    } else {
        // Existing order – UPDATE header fields and subtotal together.
        QVariantMap updateParams {
          {"admin_id",           m_header.admin_id},
          {"customer_id",        m_header.customer_id == -1 ? QVariant() : QVariant(m_header.customer_id)},
          {"customer_name",      m_header.customer_name},
          {"customer_phone",     m_header.customer_phone.isEmpty() ? QVariant() : QVariant(m_header.customer_phone)},
          {"price_level_id",     m_header.price_level_id},
          {"subtotal",           subtotal},
          {"discount_amount",    m_header.discount_amount},
          {"discount_percentage",m_header.discount_percentage},
          // {"tax_amount",         m_header.tax_amount},
          // {"status",             m_header.status},
          {"priority",           m_header.priority},
          // {"payment_status",     m_header.payment_status},
          {"order_date",         m_header.order_date.isNull() ? QVariant() : QVariant(m_header.order_date)},
          {"deadline_date",      m_header.deadline_date.isNull() ? QVariant() : QVariant(m_header.deadline_date)},
          {"notes",              m_header.notes.isEmpty() ? QVariant() : QVariant(m_header.notes)},
          {"internal_notes",     m_header.internal_notes.isEmpty() ? QVariant() : QVariant(m_header.internal_notes)},
          {"order_number",       m_header.order_number}
        };
        
        bool updateOk = oman.update(m_orderId, updateParams);
        if (!updateOk) {
            qWarning() << "OrderModel::commit – UPDATE orders failed:"
                       << oman.errorString();
            db.rollback();
            return false;
        }
    }

    // ── Step 2: DELETE removed order_items ───────────────────────────────────
    // CASCADE removes their finishings automatically.
    OrderItemManager oim;
    StockMovementManager smm;
    ProductManager prm;
    
    for (int itemId : m_deletedItemIds) {
      auto opt_item = oim.getById(itemId);
      
      if (!opt_item.has_value()) {
        qWarning() << "OrderModel::commit – DELETE order_item:"
                   << "unable to get item";
        db.rollback();
        return false;
      }
      
      auto r_item = *opt_item;
      auto opt_pro = prm.getById(r_item.value("product_id").toInt());
      
      if (!opt_pro.has_value()) {
        qWarning() << "OrderModel::commit – DELETE order_item:"
                   << "unable to get product";
        db.rollback();
        return false;
      }
      
      auto r_pro = *opt_pro;
      auto calcqty = r_pro.value("use_area").toBool() ? r_item.value("quantity").toDouble() *
                                                                         r_item.value("size_width").toDouble() *
                                                                         r_item.value("size_height").toDouble()
                                                                       : r_item.value("quantity").toDouble();
      bool stock_update = prm.adjustStock(r_pro.value("id").toInt(), 
                                          calcqty);
      if(!stock_update) {
        qWarning() << "OrderModel::commit – DELETE order_item:"
                   << "update stock failed";
        db.rollback();
        return false;
      }
      
      auto opt_mvt = smm.recordMovement(r_pro.value("id").toInt(), "adjustment",
                                         calcqty, r_pro.value("stock").toDouble(),
                                         calcqty + r_pro.value("stock").toDouble(),
                                         m_header.admin_id, "Item Removal", -1, "");
      
      if(!opt_mvt) {
        qWarning() << "OrderModel::commit – DELETE order_item:"
                   << "tidak dapat mencatat stock_movement";
        db.rollback();
        return false;
      }
      
      if (!oim.remove(itemId)) {
        qWarning() << "OrderModel::commit – DELETE order_item:"
                   << oim.errorString();
        db.rollback();
        return false;
      }
    }

    // ── Step 3: DELETE removed finishings ────────────────────────────────────
    OrderItemFinishingManager oifm;
    for (auto it = m_deletedFinishingIds.begin(); it != m_deletedFinishingIds.end(); ++it) {
        for (int fiId : it.value()) {
            if (!oifm.remove(fiId)) {
                qWarning() << "OrderModel::commit – DELETE finishing failed:"
                           << oifm.errorString();
                db.rollback();
                return false;
            }
        }
    }

    // ── Step 4: INSERT / UPDATE dirty items and their finishings ─────────────
    StockMovementManager stockManager;
    ProductManager productManager;
    for (int row : m_dirtyRows) {
        OrderItem &item = m_items[row];
        item.order_id = m_orderId;

        bool isNew = (item.id == -1); // simpan sebelum save() mengisi id
        double qtyBefore = 0.0;

        if (!isNew) {
            // Ambil qty lama dari DB sebelum di-overwrite
            auto opt_old = oim.getById(item.id);
            if (opt_old.has_value()) {
                auto old = *opt_old;
                bool useArea = old.value("use_area").toBool();
                qtyBefore = useArea ? old.value("quantity").toDouble()
                                        * old.value("size_width").toDouble()
                                        * old.value("size_height").toDouble()
                                    : old.value("quantity").toDouble();
            }
        }

        if (!item.save(db)) {
            db.rollback();
            return false;
        }

        auto opt_pro = productManager.getById(item.product_id);
        if (!opt_pro.has_value()) { db.rollback(); return false; }
        auto r_pro = *opt_pro;

        qreal qtyAfter = r_pro.value("use_area").toBool()
                             ? item.size_width * item.size_height * item.quantity
                             : item.quantity;

        double delta = -(qtyAfter - qtyBefore); // negatif = pengurangan stok

        if (qAbs(delta) > 0.0001) { // hanya jika ada perubahan qty
            if (!productManager.adjustStock(r_pro.value("id").toInt(), delta)) {
                db.rollback();
                return false;
            }

            stockManager.recordMovement(item.product_id,
                                         isNew ? "out" : "adjustment",
                                         delta,
                                         r_pro.value("stock").toDouble(),
                                         r_pro.value("stock").toDouble() + delta,
                                         m_header.admin_id,
                                         "orders", m_orderId,
                                         isNew ? "Penjualan Produk" : "Update Item Order");
        }
    }

    // ── Step 5: Commit the transaction ───────────────────────────────────────
    if (!db.commit()) {
        qWarning() << "OrderModel::commit – db.commit() failed:"
                   << db.lastError().text();
        db.rollback();
        return false;
    }

    // ── Step 6: Clear dirty state ─────────────────────────────────────────────
    m_headerDirty = false;
    m_dirtyRows.clear();
    m_deletedItemIds.clear();
    m_deletedFinishingIds.clear();

    if (!m_items.isEmpty())
        emit dataChanged(index(0), index(m_items.size() - 1), {Qt::UserRole + 18});

    // Broadcast the final total_amount (VIRTUAL col: subtotal - discount + tax).
    QSqlQuery tq(db);
    tq.prepare("SELECT total_amount FROM orders WHERE id = :oid");
    tq.bindValue(":oid", m_orderId);
    if (tq.exec() && tq.next())
        emit orderTotalChanged(tq.value(0).toInt());

    return true;
}

bool OrderModel::revert(QSqlDatabase &db)
{
    if (m_orderId == -1) {
        // Order was never saved – just wipe everything back to a blank slate.
        beginResetModel();
        m_items.clear();
        m_header         = OrderHeader{};
        m_headerDirty    = false;
        m_dirtyRows.clear();
        m_deletedItemIds.clear();
        m_deletedFinishingIds.clear();
        endResetModel();
        emit orderTotalChanged(0);
        return true;
    }
    return loadOrder(m_orderId, db);
}

// ============================================================================
// FinishingItem
// ============================================================================

int FinishingItem::subtotal() const
{
    return quantity * finishing_price;
}

FinishingItem FinishingItem::loadFromId(int id)
{
  
    FinishingItem fi;
    OrderItemFinishingManager oifm;
    auto opt_fin = oifm.getById(id);
    if (!opt_fin.has_value()) {
      return fi;
    }
    auto q = *opt_fin;
    fi.id              = q.value("id").toInt();
    fi.order_item_id   = q.value("order_item_id").toInt();
    fi.finishing_id    = q.value("finishing_id").toInt();
    fi.finishing_name  = q.value("finishing_name").toString();
    fi.quantity        = q.value("quantity").toInt();
    fi.finishing_price = q.value("finishing_price").toInt();
    return fi;
}

// ============================================================================
// OrderItem – computed columns
// ============================================================================

int OrderItem::subtotal() const
{
    const double raw = static_cast<double>(quantity)
                     * static_cast<double>(sale_price)
                     * (use_area ? size_width  : 1.0)
                     * (use_area ? size_height : 1.0);
    return static_cast<int>((raw + 99.99999) / 100.0) * 100;
}

int OrderItem::total() const
{
    return subtotal() + finishing_total - discount_amount;
}

// ============================================================================
// OrderModel::OrderItem – persistence
// ============================================================================

OrderItem &OrderItem::loadFromId(int id)
{
    OrderManager oman;
    auto opt_ord = oman.getById(id);
    if (!opt_ord.has_value()) {
      return *this;
    }
    
    auto q = *opt_ord;

    this->id            = q.value("id").toInt();
    order_id            = q.value("order_id").toInt();
    product_id          = q.value("product_id").toInt();
    product_name        = q.value("product_name").toString();
    sku                 = q.value("sku").toString();
    quantity            = q.value("quantity").toInt();
    unit                = q.value("unit").toString();
    size_width          = q.value("size_width").toReal();
    size_height         = q.value("size_height").toReal();
    use_area            = q.value("use_area").toBool();
    sale_price          = q.value("sale_price").toInt();
    base_price          = q.value("base_price").toInt();
    discount_percentage = q.value("discount_percentage").toReal();
    discount_amount     = q.value("discount_amount").toInt();
    finishing_total     = q.value("finishing_total").toInt();
    notes               = q.value("notes").toString();

    finishings.clear();
    
    OrderItemFinishingManager oifm;
    auto recs = oifm.getByOrderItem(id);
    for(auto const &rc : recs)
      finishings.append(FinishingItem::loadFromId(rc.value(0).toInt()));
    return *this;
}

bool OrderItem::save(QSqlDatabase &db)
{
    OrderItemManager oim;
    OrderItemFinishingManager oifm;
    QVariantMap params {
      {"order_id",            order_id},
      {"product_id",          product_id == -1 ? QVariant() : QVariant(product_id)},
      {"product_name",        product_name},
      {"sku",                 sku},
      {"quantity",            quantity},
      {"unit",                unit},
      {"size_width",          size_width},
      {"size_height",         size_height},
      {"use_area",            static_cast<int>(use_area)},
      {"sale_price",          sale_price},
      {"base_price",          base_price},
      {"discount_percentage", discount_percentage},
      {"discount_amount",     discount_amount},
      {"finishing_total",     finishing_total},
      {"notes",               notes.isEmpty() ? QVariant() : QVariant(notes)}
    };
    bool saveOk = false;
    int liveId = id;
    if (id == -1) {
      auto opt_order = oim.create(params);
      if(opt_order.has_value()) {
        liveId = (*opt_order).value("id").toInt();
        id = liveId;
        saveOk = true;
      }
    } else {
      saveOk = oim.update(id, params);
    }
    if (!saveOk) {
        qWarning() << "OrderItem::save failed:" << oim.errorString();
        return false;
    }
    
    // Upsert finishings: delete orphans first, then insert/update survivors.
    QList<int> keepIds;
    
    for (const FinishingItem &fi : finishings)
        if (fi.id != -1) keepIds << fi.id;
    
    auto allFinishingsByOrderItem = oifm.getByOrderItem(id);
    for (auto &rec : allFinishingsByOrderItem) {
      int fId = rec.value("id").toInt();
      if (!keepIds.contains(fId)) {
        if(!oifm.remove(fId)){
          qWarning() << "OrderItem save : finishing delete failed"
                     << oifm.errorString();
          return false;
        }
      }
    }

    for (auto &fi : finishings) {
      QVariantMap fiParams {
        {"order_item_id",  liveId},
        {"finishing_id",   fi.finishing_id == -1 ? QVariant() : QVariant(fi.finishing_id)},
        {"finishing_name", fi.finishing_name},
        {"quantity",       fi.quantity},
        {"finishing_price",fi.finishing_price}
      };
      bool succes = false;
      if (fi.id == -1) {
        auto opt_fin = oifm.create(fiParams);
        succes = opt_fin.has_value();
        if (succes) fi.id = (*opt_fin).value("id").toInt();
      } else {
        succes = oifm.update(fi.id, fiParams);
      }
      if (!succes) {
          qWarning() << "OrderItem::save – finishing upsert failed:" << oifm.errorString();
          return false;
      }
    }
    return true;
}

const QString OrderItem::descriptionText() const {
  QString desc(sku);
  if (use_area) {
    desc += " | " + QString("%1 x %2 %3").arg(size_width).arg(size_height).arg(unit);
    desc += QString(" x%1 ").arg(quantity);
  } else {
    desc += " | " + QString("x%1 %2").arg(quantity).arg(unit);
  }
  QStringList fins;
  for(auto const& fin : finishings) {
    fins << QString("%1:%2").arg(fin.finishing_name).arg(fin.quantity);
  }
  if (fins.count()) {
    return desc + QString(" | %1").arg(fins.join(" | "));
  }
  return desc;
}
