#include "orderitemeditormodel.h"
#include "src/managers/managers.h"

// Maps Column enum → the SQL field name used in QSqlRecord / QVariantMap
QString OrderItemEditorModel::columnKey(int column)
{
    switch (column) {
        case Col_Id:                 return "id";
        case Col_OrderId:            return "order_id";
        case Col_ProductId:          return "product_id";
        case Col_ProductName:        return "product_name";
        case Col_Sku:                return "sku";
        case Col_Quantity:           return "quantity";
        case Col_Unit:               return "unit";
        case Col_BasePrice:          return "base_price";
        case Col_DiscountPercentage: return "discount_percentage";
        case Col_DiscountAmount:     return "discount_amount";
        case Col_Subtotal:           return "subtotal";
        case Col_Notes:              return "notes";
        case Col_CreatedAt:          return "created_at";
        case Col_UpdatedAt:          return "updated_at";
        default:                     return {};
    }
}

OrderItemEditorModel::OrderItemEditorModel(QObject *p)
    : QAbstractTableModel(p) {}

OrderItemEditorModel::~OrderItemEditorModel() {}

bool OrderItemEditorModel::loadFromOrder(int orderid)
{
    beginResetModel();
    m_fromDatabase = oim.getByOrder(orderid);   // fixed: was orderId (wrong name)
    m_newData.clear();
    m_editedCells.clear();
    endResetModel();                             // fixed: was missing semicolons
    return m_fromDatabase.count() > 0;
}

int OrderItemEditorModel::rowCount(const QModelIndex& parent) const   // fixed: added const + scope
{
    if (parent.isValid()) return 0;
    return m_fromDatabase.count() + m_newData.count();
}

int OrderItemEditorModel::columnCount(const QModelIndex& parent) const  // fixed: added const + scope + return
{
    if (parent.isValid()) return 0;
    return Col_COUNT;
}

QVariant OrderItemEditorModel::data(const QModelIndex& ix, int role) const  // fixed: added scope
{
    if (!ix.isValid()) return QVariant();
    if (ix.row() >= rowCount() || ix.column() >= Col_COUNT) return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        // Check for a pending user edit first
        auto key = qMakePair(ix.row(), ix.column());
        if (m_editedCells.contains(key))
            return m_editedCells[key];

        const QString field = columnKey(ix.column());

        // Row comes from the database
        if (ix.row() < m_fromDatabase.count()) {
            const QSqlRecord& record = m_fromDatabase.at(ix.row());
            return record.value(field);             // fixed: was record[record.fieldName(col)]
        }

        // Row is a newly added (unsaved) row
        int newDataRow = ix.row() - m_fromDatabase.count();
        return m_newData.at(newDataRow).value(field);   // fixed: was m_newData[]
    }

    return QVariant();   // fixed: was incorrectly forwarding to base class
}

bool OrderItemEditorModel::setData(const QModelIndex& ix, const QVariant& value, int role)  // fixed: added scope
{
    if (!ix.isValid()) return false;
    if (role != Qt::EditRole) return false;
    if (ix.row() >= rowCount() || ix.column() >= Col_COUNT) return false;

    // id, order_id, created_at, updated_at are read-only
    if (ix.column() == Col_Id || ix.column() == Col_OrderId ||
        ix.column() == Col_CreatedAt || ix.column() == Col_UpdatedAt)
        return false;

    m_editedCells[qMakePair(ix.row(), ix.column())] = value;
    emit dataChanged(ix, ix, {role});
    return true;
}

QVariant OrderItemEditorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return QVariant();
    if (orientation == Qt::Vertical) return section + 1;

    switch (section) {
        case Col_Id:                 return "ID";
        case Col_OrderId:            return "Order ID";
        case Col_ProductId:          return "Product ID";
        case Col_ProductName:        return "Nama";
        case Col_Sku:                return "SKU";
        case Col_Quantity:           return "Qty";
        case Col_Unit:               return "Unit";
        case Col_BasePrice:          return "Satuan";
        case Col_DiscountPercentage: return "Disc %";
        case Col_DiscountAmount:     return "Disc Amount";
        case Col_Subtotal:           return "Subtotal";
        case Col_Notes:              return "Catatan";
        case Col_CreatedAt:          return "Created At";
        case Col_UpdatedAt:          return "Updated At";
        default:                     return QVariant();
    }
}

Qt::ItemFlags OrderItemEditorModel::flags(const QModelIndex& mi) const  // fixed: added scope + body
{
    if (!mi.isValid()) return Qt::NoItemFlags;

    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    // Make read-only columns non-editable
    if (mi.column() != Col_Id && mi.column() != Col_OrderId &&
        mi.column() != Col_CreatedAt && mi.column() != Col_UpdatedAt)
        f |= Qt::ItemIsEditable;

    return f;
}