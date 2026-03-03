#include "orderitemeditormodel.h"
#include "src/managers/managers.h"

// Maps Column enum → the SQL field name used in QSqlRecord / QVariantMap
namespace {
    QHash<int, QString> columnMap {
        {0,                 "id"},
        {1,            "order_id"},
        {2,          "product_id"},
        {3,        "product_name"},
        {4,                "sku"},
        {5,           "quantity"},
        {6,               "unit"},
        {7,          "base_price"},
        {8, "discount_percentage"},
        {9,     "discount_amount"},
        {9,           "subtotal"},
        {10,              "notes"},
        {11,          "created_at"},
        {12,          "updated_at"},
    };
}
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
    m_orderid = orderid;
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
        case Col_ProductId:          return "Product";
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
bool OrderItemEditorModel::appendRow(const QVariantMap &defaultValues)
{
    int newRowIndex = rowCount();  // posisi setelah semua baris saat ini

    beginInsertRows(QModelIndex(), newRowIndex, newRowIndex);

    QVariantMap newItem;

    // 1. Isi nilai default standar model
    newItem["order_id"]            = QVariant();  // biasanya diisi saat commit/save
    newItem["product_id"]          = 0;
    newItem["product_name"]        = QString();
    newItem["sku"]                 = QString();
    newItem["quantity"]            = 1;
    newItem["unit"]                = QStringLiteral("pcs");
    newItem["base_price"]          = 0.0;
    newItem["discount_percentage"] = 0.0;
    newItem["discount_amount"]     = 0.0;
    newItem["subtotal"]            = 0.0;
    newItem["notes"]               = QString();
    // created_at dan updated_at dibiarkan kosong → diisi oleh database

    // 2. Override dengan nilai yang diberikan pengguna (jika ada)
    for (auto it = defaultValues.constBegin(); it != defaultValues.constEnd(); ++it)
    {
        const QString &key = it.key();
        // Hanya override field yang benar-benar ada di model
        if (columnMap.values().contains(key)) {  // opsional: validasi field
            newItem[key] = it.value();
        }
    }

    m_newData.append(newItem);

    endInsertRows();

    // Emit sinyal bahwa data telah berubah (opsional, tergantung kebutuhan view)
    // emit dataChanged(...) bisa ditambahkan jika diperlukan

    return true;
}

bool OrderItemEditorModel::removeRow(int row, const QModelIndex &parent)
{
    if (parent.isValid())
        return false;

    if (row < 0 || row >= rowCount())
        return false;

    beginRemoveRows(parent, row, row);

    if (row < m_fromDatabase.count())
    {
        // Baris dari database → kita hanya tandai untuk dihapus nanti (soft delete logic)
        // Untuk model editor sederhana, biasanya kita hapus dari daftar sementara
        m_fromDatabase.removeAt(row);

        // Hapus juga semua edit yang sudah dilakukan pada baris ini
        QMutableHashIterator<QPair<int,int>, QVariant> it(m_editedCells);
        while (it.hasNext())
        {
            it.next();
            if (it.key().first == row)
                it.remove();
            else if (it.key().first > row)
            {
                // Geser index baris yang lebih besar
                auto newKey = qMakePair(it.key().first - 1, it.key().second);
                m_editedCells[newKey] = it.value();
                it.remove();
            }
        }
    }
    else
    {
        // Baris baru (belum disimpan)
        int newDataIdx = row - m_fromDatabase.count();
        if (newDataIdx >= 0 && newDataIdx < m_newData.count())
        {
            m_newData.removeAt(newDataIdx);
        }

        // Hapus edit cells yang terkait (seharusnya tidak ada, tapi untuk aman)
        QMutableHashIterator<QPair<int,int>, QVariant> it(m_editedCells);
        while (it.hasNext())
        {
            it.next();
            if (it.key().first == row)
                it.remove();
            else if (it.key().first > row)
            {
                auto newKey = qMakePair(it.key().first - 1, it.key().second);
                m_editedCells[newKey] = it.value();
                it.remove();
            }
        }
    }

    endRemoveRows();
    return true;
}

bool OrderItemEditorModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || count <= 0)
        return false;

    int last = row + count - 1;
    if (last >= rowCount())
        return false;

    beginRemoveRows(parent, row, last);

    // Implementasi sederhana: panggil removeRow berulang (bisa dioptimasi nanti)
    for (int i = 0; i < count; ++i)
    {
        removeRow(row, parent);  // perhatikan: row tidak bertambah karena list bergeser
    }

    endRemoveRows();
    return true;
}

void OrderItemEditorModel::revertAllChanges()
{
    beginResetModel();
    m_editedCells.clear();
    m_newData.clear();
    // Catatan: data dari database tidak di-reload otomatis
    // Jika ingin reload dari DB, panggil loadFromOrder lagi setelah ini
    endResetModel();
    if (!m_fromDatabase.isEmpty()) {
        loadFromOrder(m_orderid);
    }
}

bool OrderItemEditorModel::isDirty() const
{
    return !m_editedCells.isEmpty() || !m_newData.isEmpty();
}