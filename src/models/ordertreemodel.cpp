#include "ordertreemodel.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QLocale>
#include <QTimeZone>
#include <QDebug>

// ═══════════════════════════════════════════════════════════════
//  TreeNode
// ═══════════════════════════════════════════════════════════════

TreeNode::TreeNode(NodeLevel lvl, int id, int parentId, TreeNode* parent)
    : level(lvl), id(id), parentId(parentId),
      columns(OrderTreeCol::COUNT), parent(parent)
{}

TreeNode::~TreeNode()
{
    qDeleteAll(children);
}

// ═══════════════════════════════════════════════════════════════
//  OrderTreeModel  —  constructor / destructor
// ═══════════════════════════════════════════════════════════════

OrderTreeModel::OrderTreeModel(QSqlDatabase db, QObject* parent)
    : QAbstractItemModel(parent),
      m_db(db),
      m_root(new TreeNode(NodeLevel::Root))
{
    fetchDateNodes(m_root);
}

OrderTreeModel::~OrderTreeModel()
{
    delete m_root;
}

// ═══════════════════════════════════════════════════════════════
//  QAbstractItemModel  —  core interface
// ═══════════════════════════════════════════════════════════════

QModelIndex OrderTreeModel::index(int row, int column,
                                  const QModelIndex& parentIndex) const
{
    if (!hasIndex(row, column, parentIndex))
        return {};

    TreeNode* parentNode = nodeFromIndex(parentIndex);
    if (row < 0 || row >= parentNode->children.size())
        return {};

    return createIndex(row, column, parentNode->children.at(row));
}

QModelIndex OrderTreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return {};

    TreeNode* node       = nodeFromIndex(index);
    TreeNode* parentNode = node->parent;

    if (!parentNode || parentNode == m_root)
        return {};

    TreeNode* grandParent = parentNode->parent;
    if (!grandParent)
        return {};

    int row = grandParent->children.indexOf(parentNode);
    return createIndex(row, 0, parentNode);
}

int OrderTreeModel::rowCount(const QModelIndex& parentIndex) const
{
    TreeNode* node = nodeFromIndex(parentIndex);
    return node ? node->children.size() : 0;
}

int OrderTreeModel::columnCount(const QModelIndex&) const
{
    return OrderTreeCol::COUNT;
}

QVariant OrderTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    TreeNode* node = nodeFromIndex(index);
    if (!node)
        return {};

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        const int col = index.column();
        if (col >= 0 && col < node->columns.size())
            return node->columns.at(col);
    }

    if (role == Qt::TextAlignmentRole)
    {
        using namespace OrderTreeCol;
        switch (index.column()) {
            case Qty:
            case UnitPrice:
            case Subtotal:
            case Discount:
            case Total:
                return QVariant(Qt::AlignRight | Qt::AlignVCenter);
            case Date:
            case CustomerName:
            case OrderNumber:
            case Name:
            default:
                return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    // Expose node level and id via custom roles
    if (role == Qt::UserRole)     return static_cast<int>(node->level);
    if (role == Qt::UserRole + 1) return node->id;

    return {};
}

QVariant OrderTreeModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    using namespace OrderTreeCol;
    switch (section) {
        case Date:         return tr("Tanggal");
        case CustomerName: return tr("Nama Customer");
        case OrderNumber:  return tr("No. Order");
        case Name:         return tr("Nama Item");
        case Qty:          return tr("Qty");
        case Size:         return tr("Ukuran (W×H)");
        case UnitPrice:    return tr("Harga Satuan");
        case Subtotal:     return tr("Subtotal");
        case Discount:     return tr("Diskon");
        case Total:        return tr("Total");
        case Notes:        return tr("Catatan");
        default:           return {};
    }
}

bool OrderTreeModel::hasChildren(const QModelIndex& parentIndex) const
{
    TreeNode* node = nodeFromIndex(parentIndex);
    if (!node) return false;

    // Finishing nodes have no children
    if (node->level == NodeLevel::Finishing) return false;

    // If children already fetched, answer directly
    if (node->childrenFetched) return !node->children.isEmpty();

    // For un-fetched nodes: rely on the early-fetch hint (populated via an
    // EXISTS(...) check when the node itself was fetched) instead of
    // optimistically assuming true. Root is always pre-fetched.
    if (node->level == NodeLevel::Root) return !node->children.isEmpty();
    return node->hasChildrenHint;
}

bool OrderTreeModel::canFetchMore(const QModelIndex& parentIndex) const
{
    TreeNode* node = nodeFromIndex(parentIndex);
    if (!node) return false;
    if (node->level == NodeLevel::Finishing) return false;
    if (node->childrenFetched) return false;
    // No point issuing a fetch we already know will come back empty.
    return node->hasChildrenHint;
}

void OrderTreeModel::fetchMore(const QModelIndex& parentIndex)
{
    TreeNode* node = nodeFromIndex(parentIndex);
    if (!node || node->childrenFetched) return;

    switch (node->level) {
        case NodeLevel::Date:
            fetchCustomerNodes(node, node->columns[OrderTreeCol::Date].toDate());
            break;
        case NodeLevel::Customer:
            fetchOrderNodes(node, node->id,
                            node->parent
                                ? node->parent->columns[OrderTreeCol::Date].toDate()
                                : QDate{});
            break;
        case NodeLevel::Order:
            fetchItemNodes(node, node->id);
            break;
        case NodeLevel::Item:
            fetchFinishingNodes(node, node->id);
            break;
        default:
            break;
    }
}

// ═══════════════════════════════════════════════════════════════
//  Public helpers
// ═══════════════════════════════════════════════════════════════

void OrderTreeModel::reload()
{
    beginResetModel();
    qDeleteAll(m_root->children);
    m_root->children.clear();
    m_root->childrenFetched = false;
    fetchDateNodes(m_root);
    endResetModel();
}

NodeLevel OrderTreeModel::nodeLevel(const QModelIndex& index) const
{
    TreeNode* n = nodeFromIndex(index);
    return n ? n->level : NodeLevel::Root;
}

int OrderTreeModel::nodeId(const QModelIndex& index) const
{
    TreeNode* n = nodeFromIndex(index);
    return n ? n->id : -1;
}

// ═══════════════════════════════════════════════════════════════
//  Fetch helpers
// ═══════════════════════════════════════════════════════════════

void OrderTreeModel::fetchDateNodes(TreeNode* parent)
{
    // Distinct order dates in local time, excluding cancelled orders.
    // SQLite modifier 'localtime' converts the stored UTC value to the
    // system's local timezone — same as what the OS/SQLite driver uses.
    QSqlQuery q(m_db);
    q.prepare(
        "SELECT DISTINCT DATE(datetime(order_date, 'localtime')) AS d "
        "FROM orders "
        "WHERE staging_status != 'cancelled' "
        "ORDER BY d DESC"
    );

    if (!q.exec()) {
        emit fetchError(q.lastError().text());
        return;
    }

    QVector<TreeNode*> newChildren;
    while (q.next()) {
        // SQLite already returned the local date — parse directly as QDate
        const QDate localDate = QDate::fromString(q.value(0).toString(), Qt::ISODate);
        if (!localDate.isValid()) continue;

        auto* node = new TreeNode(NodeLevel::Date, -1, -1, parent);
        // Date nodes only ever come from dates that already have at least
        // one qualifying order, so a Customer child is guaranteed.
        node->hasChildrenHint = true;
        node->columns[OrderTreeCol::Date] = localDate;
        newChildren.append(node);
    }

    if (!newChildren.isEmpty()) {
        const int first = parent->children.size();
        const int last  = first + newChildren.size() - 1;
        beginInsertRows(QModelIndex(), first, last);
        parent->children.append(newChildren);
        parent->childrenFetched = true;
        endInsertRows();
    } else {
        parent->childrenFetched = true;
    }
}

void OrderTreeModel::fetchCustomerNodes(TreeNode* parent, const QDate& date)
{
    // Filter by local date using SQLite's 'localtime' modifier —
    // avoids manual UTC round-trip and is consistent with fetchDateNodes.
    QSqlQuery q(m_db);
    q.prepare(
        "SELECT DISTINCT customer_id, customer_name, customer_phone "
        "FROM orders "
        "WHERE staging_status != 'cancelled' "
        "  AND DATE(datetime(order_date, 'localtime')) = :localdate "
        "ORDER BY customer_name"
    );
    q.bindValue(":localdate", date.toString(Qt::ISODate));

    if (!q.exec()) {
        emit fetchError(q.lastError().text());
        return;
    }

    QModelIndex parentIndex = createIndex(
        parent->parent ? parent->parent->children.indexOf(parent) : 0,
        0, parent);

    QVector<TreeNode*> newChildren;
    while (q.next()) {
        const int    custId    = q.value(0).toInt();
        const QString custName = q.value(1).toString();
        const QString custPhone= q.value(2).toString();

        auto* node = new TreeNode(NodeLevel::Customer, custId, -1, parent);
        // Customer nodes are only ever created for a (date) that already has
        // at least one matching order (see the WHERE below), so they are
        // guaranteed to have at least one Order child.
        node->hasChildrenHint = true;
        // Col "Nama Customer": "ID — Nama (Phone)"
        node->columns[OrderTreeCol::CustomerName] =
            QStringLiteral("%1 [%2] - %3)")
                .arg(custName)
                .arg(custId)
                .arg(custPhone.isEmpty() ? QString{} : QStringLiteral(" (%1)").arg(custPhone));
        newChildren.append(node);
    }

    if (!newChildren.isEmpty()) {
        const int first = parent->children.size();
        const int last  = first + newChildren.size() - 1;
        beginInsertRows(parentIndex, first, last);
        parent->children.append(newChildren);
        parent->childrenFetched = true;
        endInsertRows();
    } else {
        parent->childrenFetched = true;
    }
}

void OrderTreeModel::fetchOrderNodes(TreeNode* parent, int customerId, const QDate& date)
{
    QSqlQuery q(m_db);
    q.prepare(
        "SELECT o.id, o.order_number, o.discount_amount, o.total_amount, o.notes, "
        "       EXISTS(SELECT 1 FROM order_items oi WHERE oi.order_id = o.id) AS has_items "
        "FROM orders o "
        "WHERE o.staging_status != 'cancelled' "
        "  AND o.customer_id = :cid "
        "  AND DATE(datetime(o.order_date, 'localtime')) = :localdate "
        "ORDER BY o.order_date"
    );
    q.bindValue(":cid",       customerId);
    q.bindValue(":localdate", date.toString(Qt::ISODate));

    if (!q.exec()) {
        emit fetchError(q.lastError().text());
        return;
    }

    // Build parent index
    TreeNode* grandParent = parent->parent;
    int parentRow = grandParent ? grandParent->children.indexOf(parent) : 0;
    QModelIndex parentIndex = createIndex(parentRow, 0, parent);

    QVector<TreeNode*> newChildren;
    while (q.next()) {
        const int orderId = q.value(0).toInt();
        auto* node = new TreeNode(NodeLevel::Order, orderId, customerId, parent);
        node->hasChildrenHint = q.value(5).toBool();

        // Col "No. Order": "order_id — order_number"
        node->columns[OrderTreeCol::OrderNumber] =
            QStringLiteral("%2 [%1]").arg(orderId).arg(q.value(1).toString());
        // Col Discount
        node->columns[OrderTreeCol::Discount] = formatCurrency(q.value(2).toInt());
        // Col Total
        node->columns[OrderTreeCol::Total]    = formatCurrency(q.value(3).toInt());
        // Col Notes
        node->columns[OrderTreeCol::Notes]    = q.value(4);

        newChildren.append(node);
    }

    if (!newChildren.isEmpty()) {
        const int first = parent->children.size();
        const int last  = first + newChildren.size() - 1;
        beginInsertRows(parentIndex, first, last);
        parent->children.append(newChildren);
        parent->childrenFetched = true;
        endInsertRows();
    } else {
        parent->childrenFetched = true;
    }
}

void OrderTreeModel::fetchItemNodes(TreeNode* parent, int orderId)
{
    QSqlQuery q(m_db);
    q.prepare(
        "SELECT oi.id, oi.product_name, oi.quantity, oi.unit, "
        "       oi.size_width, oi.size_height, oi.use_area, "
        "       oi.sale_price, oi.subtotal, oi.discount_amount, oi.total, oi.notes, "
        "       EXISTS(SELECT 1 FROM order_item_finishings f "
        "              WHERE f.order_item_id = oi.id) AS has_finishings "
        "FROM order_items oi "
        "WHERE oi.order_id = :oid "
        "ORDER BY oi.id"
    );
    q.bindValue(":oid", orderId);

    if (!q.exec()) {
        emit fetchError(q.lastError().text());
        return;
    }

    // Build parent index
    TreeNode* grandParent = parent->parent;
    int parentRow = grandParent ? grandParent->children.indexOf(parent) : 0;
    QModelIndex parentIndex = createIndex(parentRow, 0, parent);

    QVector<TreeNode*> newChildren;
    while (q.next()) {
        const int itemId = q.value(0).toInt();
        auto* node = new TreeNode(NodeLevel::Item, itemId, orderId, parent);
        node->hasChildrenHint = q.value(12).toBool();

        // Col 0: product name
        node->columns[OrderTreeCol::Name] = q.value(1);
        // Col 1: "qty unit"
        node->columns[OrderTreeCol::Qty]  =
            QStringLiteral("%1 %2").arg(q.value(2).toInt()).arg(q.value(3).toString());
        // Col 2: size
        node->columns[OrderTreeCol::Size] =
            formatSize(q.value(4).toDouble(), q.value(5).toDouble(), q.value(6).toInt());
        // Col 3: sale price
        node->columns[OrderTreeCol::UnitPrice] = formatCurrency(q.value(7).toInt());
        // Col 4: subtotal
        node->columns[OrderTreeCol::Subtotal]  = formatCurrency(q.value(8).toInt());
        // Col 5: discount
        node->columns[OrderTreeCol::Discount]  = formatCurrency(q.value(9).toInt());
        // Col 6: total
        node->columns[OrderTreeCol::Total]     = formatCurrency(q.value(10).toInt());
        // Col 7: notes
        node->columns[OrderTreeCol::Notes]     = q.value(11);

        newChildren.append(node);
    }

    if (!newChildren.isEmpty()) {
        const int first = parent->children.size();
        const int last  = first + newChildren.size() - 1;
        beginInsertRows(parentIndex, first, last);
        parent->children.append(newChildren);
        parent->childrenFetched = true;
        endInsertRows();
    } else {
        parent->childrenFetched = true;
    }
}

void OrderTreeModel::fetchFinishingNodes(TreeNode* parent, int orderItemId)
{
    QSqlQuery q(m_db);
    q.prepare(
        "SELECT id, finishing_name, quantity, finishing_price, subtotal "
        "FROM order_item_finishings "
        "WHERE order_item_id = :iid "
        "ORDER BY id"
    );
    q.bindValue(":iid", orderItemId);

    if (!q.exec()) {
        emit fetchError(q.lastError().text());
        return;
    }

    // Build parent index
    TreeNode* grandParent = parent->parent;
    int parentRow = grandParent ? grandParent->children.indexOf(parent) : 0;
    QModelIndex parentIndex = createIndex(parentRow, 0, parent);

    QVector<TreeNode*> newChildren;
    while (q.next()) {
        const int fid = q.value(0).toInt();
        auto* node = new TreeNode(NodeLevel::Finishing, fid, orderItemId, parent);
        node->hasChildrenHint = false; // Finishing is always a leaf level

        // Col 0: finishing name
        node->columns[OrderTreeCol::Name]      = q.value(1);
        // Col 1: qty (no unit for finishing)
        node->columns[OrderTreeCol::Qty]       = q.value(2);
        // Col 3: finishing price per unit
        node->columns[OrderTreeCol::UnitPrice] = formatCurrency(q.value(3).toInt());
        // Col 4: subtotal
        node->columns[OrderTreeCol::Subtotal]  = formatCurrency(q.value(4).toInt());

        newChildren.append(node);
    }

    if (!newChildren.isEmpty()) {
        const int first = parent->children.size();
        const int last  = first + newChildren.size() - 1;
        beginInsertRows(parentIndex, first, last);
        parent->children.append(newChildren);
        parent->childrenFetched = true;
        endInsertRows();
    } else {
        parent->childrenFetched = true;
    }
}

// ═══════════════════════════════════════════════════════════════
//  Utility
// ═══════════════════════════════════════════════════════════════

QVariant OrderTreeModel::formatCurrency(int value)
{
    if (value == 0) return {};
    return QLocale().toCurrencyString(static_cast<double>(value), "Rp ", 0);
}

QVariant OrderTreeModel::formatSize(double w, double h, int useArea)
{
    if (useArea) {
        // area mode: show W × H in m²
        return QStringLiteral("%1 × %2 m²").arg(w, 0, 'f', 2).arg(h, 0, 'f', 2);
    }
    // non-area: just show dimensions
    return QStringLiteral("%1 × %2").arg(w, 0, 'f', 2).arg(h, 0, 'f', 2);
}

QDateTime OrderTreeModel::toLocal(const QVariant& dbValue)
{
    // DB stores UTC without 'Z' suffix — parse as UTC then convert
    QDateTime dt = QDateTime::fromString(dbValue.toString(), Qt::ISODate);
    if (!dt.isValid())
        dt = QDateTime::fromString(dbValue.toString(), "yyyy-MM-dd HH:mm:ss");
    dt.setTimeZone(QTimeZone::utc());
    return dt;
}

TreeNode* OrderTreeModel::nodeFromIndex(const QModelIndex& index) const
{
    if (!index.isValid())
        return m_root;
    return static_cast<TreeNode*>(index.internalPointer());
}
