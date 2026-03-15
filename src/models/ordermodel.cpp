#include "ordermodel.h"

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
               discount_amount, discount_percentage, tax_amount,
               status, priority, payment_status,
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
    m_header.tax_amount         = hq.value("tax_amount").toInt();
    m_header.status             = hq.value("status").toString();
    m_header.priority           = hq.value("priority").toString();
    m_header.payment_status     = hq.value("payment_status").toString();
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

    if (m_orderId == -1) {
        // Brand-new order – INSERT.
        QSqlQuery oq(db);
        oq.prepare(R"(
            INSERT INTO orders
                (admin_id, customer_id, customer_name, customer_phone,
                 price_level_id,
                 subtotal, discount_amount, discount_percentage, tax_amount,
                 status, priority, payment_status,
                 order_date, deadline_date,
                 notes, internal_notes)
            VALUES
                (:admin_id, :customer_id, :customer_name, :customer_phone,
                 :price_level_id,
                 :subtotal, :discount_amount, :discount_percentage, :tax_amount,
                 :status, :priority, :payment_status,
                 :order_date, :deadline_date,
                 :notes, :internal_notes)
        )");
        oq.bindValue(":admin_id",           m_header.admin_id);
        oq.bindValue(":customer_id",        m_header.customer_id == -1
                                                ? QVariant() : QVariant(m_header.customer_id));
        oq.bindValue(":customer_name",      m_header.customer_name);
        oq.bindValue(":customer_phone",     m_header.customer_phone.isEmpty()
                                                ? QVariant() : QVariant(m_header.customer_phone));
        oq.bindValue(":price_level_id",     m_header.price_level_id);
        oq.bindValue(":subtotal",           subtotal);
        oq.bindValue(":discount_amount",    m_header.discount_amount);
        oq.bindValue(":discount_percentage",m_header.discount_percentage);
        oq.bindValue(":tax_amount",         m_header.tax_amount);
        oq.bindValue(":status",             m_header.status);
        oq.bindValue(":priority",           m_header.priority);
        oq.bindValue(":payment_status",     m_header.payment_status);
        oq.bindValue(":order_date",         m_header.order_date.isNull()
                                                ? QVariant() : QVariant(m_header.order_date));
        oq.bindValue(":deadline_date",      m_header.deadline_date.isNull()
                                                ? QVariant() : QVariant(m_header.deadline_date));
        oq.bindValue(":notes",              m_header.notes.isEmpty()
                                                ? QVariant() : QVariant(m_header.notes));
        oq.bindValue(":internal_notes",     m_header.internal_notes.isEmpty()
                                                ? QVariant() : QVariant(m_header.internal_notes));

        if (!oq.exec()) {
            qWarning() << "OrderModel::commit – INSERT orders failed:"
                       << oq.lastError().text();
            db.rollback();
            return false;
        }

        m_orderId    = oq.lastInsertId().toInt();
        m_header.id  = m_orderId;

        // Fetch the auto-generated order_number (assigned by DB trigger).
        QSqlQuery nq(db);
        nq.prepare("SELECT order_number FROM orders WHERE id = :id");
        nq.bindValue(":id", m_orderId);
        if (nq.exec() && nq.next())
            m_header.order_number = nq.value(0).toString();

        // Stamp all pending items with the new order id.
        for (OrderItem &it : m_items)
            it.order_id = m_orderId;

    } else {
        // Existing order – UPDATE header fields and subtotal together.
        QSqlQuery oq(db);
        oq.prepare(R"(
            UPDATE orders SET
                admin_id            = :admin_id,
                customer_id         = :customer_id,
                customer_name       = :customer_name,
                customer_phone      = :customer_phone,
                price_level_id      = :price_level_id,
                subtotal            = :subtotal,
                discount_amount     = :discount_amount,
                discount_percentage = :discount_percentage,
                tax_amount          = :tax_amount,
                status              = :status,
                priority            = :priority,
                payment_status      = :payment_status,
                order_date          = :order_date,
                deadline_date       = :deadline_date,
                notes               = :notes,
                internal_notes      = :internal_notes,
                updated_at          = datetime('now')
            WHERE id = :id
        )");
        oq.bindValue(":id",                 m_orderId);
        oq.bindValue(":admin_id",           m_header.admin_id);
        oq.bindValue(":customer_id",        m_header.customer_id == -1
                                                ? QVariant() : QVariant(m_header.customer_id));
        oq.bindValue(":customer_name",      m_header.customer_name);
        oq.bindValue(":customer_phone",     m_header.customer_phone.isEmpty()
                                                ? QVariant() : QVariant(m_header.customer_phone));
        oq.bindValue(":price_level_id",     m_header.price_level_id);
        oq.bindValue(":subtotal",           subtotal);
        oq.bindValue(":discount_amount",    m_header.discount_amount);
        oq.bindValue(":discount_percentage",m_header.discount_percentage);
        oq.bindValue(":tax_amount",         m_header.tax_amount);
        oq.bindValue(":status",             m_header.status);
        oq.bindValue(":priority",           m_header.priority);
        oq.bindValue(":payment_status",     m_header.payment_status);
        oq.bindValue(":order_date",         m_header.order_date.isNull()
                                                ? QVariant() : QVariant(m_header.order_date));
        oq.bindValue(":deadline_date",      m_header.deadline_date.isNull()
                                                ? QVariant() : QVariant(m_header.deadline_date));
        oq.bindValue(":notes",              m_header.notes.isEmpty()
                                                ? QVariant() : QVariant(m_header.notes));
        oq.bindValue(":internal_notes",     m_header.internal_notes.isEmpty()
                                                ? QVariant() : QVariant(m_header.internal_notes));

        if (!oq.exec()) {
            qWarning() << "OrderModel::commit – UPDATE orders failed:"
                       << oq.lastError().text();
            db.rollback();
            return false;
        }
    }

    // ── Step 2: DELETE removed order_items ───────────────────────────────────
    // CASCADE removes their finishings automatically.
    for (int itemId : m_deletedItemIds) {
        QSqlQuery q(db);
        q.prepare("DELETE FROM order_items WHERE id = :id");
        q.bindValue(":id", itemId);
        if (!q.exec()) {
            qWarning() << "OrderModel::commit – DELETE order_item failed:"
                       << q.lastError().text();
            db.rollback();
            return false;
        }
    }

    // ── Step 3: DELETE removed finishings ────────────────────────────────────
    for (auto it = m_deletedFinishingIds.begin(); it != m_deletedFinishingIds.end(); ++it) {
        for (int fiId : it.value()) {
            QSqlQuery q(db);
            q.prepare("DELETE FROM order_item_finishings WHERE id = :id");
            q.bindValue(":id", fiId);
            if (!q.exec()) {
                qWarning() << "OrderModel::commit – DELETE finishing failed:"
                           << q.lastError().text();
                db.rollback();
                return false;
            }
        }
    }

    // ── Step 4: INSERT / UPDATE dirty items and their finishings ─────────────
    for (int row : m_dirtyRows) {
        OrderItem &item = m_items[row];
        item.order_id = m_orderId;

        if (!item.save(db)) {
            db.rollback();
            return false;
        }

        // If this was a fresh INSERT, save() is const so it cannot write the
        // new id back. Fetch it by finding the highest id for this order.
        if (item.id == -1) {
            QSqlQuery idQ(db);
            idQ.prepare(R"(
                SELECT id FROM order_items
                WHERE  order_id = :oid
                ORDER  BY id DESC LIMIT 1
            )");
            idQ.bindValue(":oid", m_orderId);
            if (!idQ.exec() || !idQ.next()) {
                qWarning() << "OrderModel::commit – could not fetch new item id:"
                           << idQ.lastError().text();
                db.rollback();
                return false;
            }
            item.id = idQ.value(0).toInt();
            for (FinishingItem &fi : item.finishings)
                fi.order_item_id = item.id;
        }

        // Sync finishing ids that were just inserted (id was -1 before save).
        QSqlQuery fq(db);
        fq.prepare(R"(
            SELECT id FROM order_item_finishings
            WHERE  order_item_id = :oid ORDER BY id
        )");
        fq.bindValue(":oid", item.id);
        if (fq.exec()) {
            int fi = 0;
            while (fq.next() && fi < item.finishings.size())
                item.finishings[fi++].id = fq.value(0).toInt();
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
    QSqlQuery q;
    q.prepare(R"(
        SELECT id, order_item_id, finishing_id, finishing_name, quantity, finishing_price
        FROM   order_item_finishings WHERE id = :id
    )");
    q.bindValue(":id", id);
    if (!q.exec() || !q.next()) {
        qWarning() << "FinishingItem::loadFromId failed for id" << id
                   << ":" << q.lastError().text();
        return fi;
    }
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
    QSqlQuery q;
    q.prepare(R"(
        SELECT id, order_id, product_id, product_name, sku,
               quantity, unit, size_width, size_height, use_area,
               sale_price, base_price, discount_percentage, discount_amount,
               finishing_total, notes
        FROM   order_items WHERE id = :id
    )");
    q.bindValue(":id", id);
    if (!q.exec() || !q.next()) {
        qWarning() << "OrderItem::loadFromId failed for id" << id
                   << ":" << q.lastError().text();
        return *this;
    }
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
    QSqlQuery fq;
    fq.prepare("SELECT id FROM order_item_finishings WHERE order_item_id = :oid ORDER BY id");
    fq.bindValue(":oid", this->id);
    if (fq.exec())
        while (fq.next())
            finishings.append(FinishingItem::loadFromId(fq.value(0).toInt()));
    return *this;
}

bool OrderItem::save(QSqlDatabase &db) const
{
    QSqlQuery q(db);
    if (id == -1) {
        q.prepare(R"(
            INSERT INTO order_items
                (order_id, product_id, product_name, sku,
                 quantity, unit, size_width, size_height, use_area,
                 sale_price, base_price, discount_percentage, discount_amount,
                 finishing_total, notes)
            VALUES
                (:order_id, :product_id, :product_name, :sku,
                 :quantity, :unit, :size_width, :size_height, :use_area,
                 :sale_price, :base_price, :discount_percentage, :discount_amount,
                 :finishing_total, :notes)
        )");
    } else {
        q.prepare(R"(
            UPDATE order_items SET
                order_id = :order_id, product_id = :product_id,
                product_name = :product_name, sku = :sku,
                quantity = :quantity, unit = :unit,
                size_width = :size_width, size_height = :size_height,
                use_area = :use_area,
                sale_price = :sale_price, base_price = :base_price,
                discount_percentage = :discount_percentage,
                discount_amount = :discount_amount,
                finishing_total = :finishing_total, notes = :notes,
                updated_at = datetime('now')
            WHERE id = :id
        )");
        q.bindValue(":id", id);
    }
    q.bindValue(":order_id",            order_id);
    q.bindValue(":product_id",          product_id == -1 ? QVariant() : QVariant(product_id));
    q.bindValue(":product_name",        product_name);
    q.bindValue(":sku",                 sku);
    q.bindValue(":quantity",            quantity);
    q.bindValue(":unit",                unit);
    q.bindValue(":size_width",          size_width);
    q.bindValue(":size_height",         size_height);
    q.bindValue(":use_area",            static_cast<int>(use_area));
    q.bindValue(":sale_price",          sale_price);
    q.bindValue(":base_price",          base_price);
    q.bindValue(":discount_percentage", discount_percentage);
    q.bindValue(":discount_amount",     discount_amount);
    q.bindValue(":finishing_total",     finishing_total);
    q.bindValue(":notes",               notes.isEmpty() ? QVariant() : QVariant(notes));

    if (!q.exec()) {
        qWarning() << "OrderItem::save failed:" << q.lastError().text();
        return false;
    }

    const int liveId = (id == -1) ? q.lastInsertId().toInt() : id;

    // Upsert finishings: delete orphans first, then insert/update survivors.
    QStringList keepIds;
    for (const FinishingItem &fi : finishings)
        if (fi.id != -1) keepIds << QString::number(fi.id);

    {
        QSqlQuery dq(db);
        if (!keepIds.isEmpty()) {
            dq.prepare(QString(
                "DELETE FROM order_item_finishings "
                "WHERE order_item_id = :oid AND id NOT IN (%1)").arg(keepIds.join(',')));
        } else {
            dq.prepare("DELETE FROM order_item_finishings WHERE order_item_id = :oid");
        }
        dq.bindValue(":oid", liveId);
        if (!dq.exec())
            qWarning() << "OrderItem::save – finishing delete failed:" << dq.lastError().text();
    }

    for (const FinishingItem &fi : finishings) {
        QSqlQuery fq(db);
        if (fi.id == -1) {
            fq.prepare(R"(
                INSERT INTO order_item_finishings
                    (order_item_id, finishing_id, finishing_name, quantity, finishing_price)
                VALUES (:order_item_id, :finishing_id, :finishing_name, :quantity, :finishing_price)
            )");
        } else {
            fq.prepare(R"(
                UPDATE order_item_finishings SET
                    finishing_id = :finishing_id, finishing_name = :finishing_name,
                    quantity = :quantity, finishing_price = :finishing_price,
                    updated_at = datetime('now')
                WHERE id = :id
            )");
            fq.bindValue(":id", fi.id);
        }
        fq.bindValue(":order_item_id",  liveId);
        fq.bindValue(":finishing_id",   fi.finishing_id == -1 ? QVariant() : QVariant(fi.finishing_id));
        fq.bindValue(":finishing_name", fi.finishing_name);
        fq.bindValue(":quantity",       fi.quantity);
        fq.bindValue(":finishing_price",fi.finishing_price);
        if (!fq.exec())
            qWarning() << "OrderItem::save – finishing upsert failed:" << fq.lastError().text();
    }
    return true;
}