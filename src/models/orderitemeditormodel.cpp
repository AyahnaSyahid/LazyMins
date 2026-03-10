#include "orderitemeditormodel.h"
#include "src/managers/managers.h"

// Maps Column enum → the SQL field name used in QSqlRecord / QVariantMap
namespace {
    QHash<int, QString> columnMap {
        {0,                   "id"},
        {1,             "order_id"},
        {2,           "product_id"},
        {3,         "product_name"},
        {4,                  "sku"},
        {5,             "quantity"},
        {6,                 "unit"},
        {7,           "size_width"},
        {8,          "size_height"},
        {9,             "use_area"},
        {10,          "sale_price"},
        {11,          "base_price"},
        {12, "discount_percentage"},
        {13,     "discount_amount"},
        {14,            "subtotal"},
        {15,               "total"},
        {16,               "notes"},
        {17,          "created_at"},
        {18,          "updated_at"},
    };
    
    struct ItemCalc {
      double sizeWidth, sizeHeight;
      int qty, salePrice, discountAmount;
      int subtotal() { return qRound(sizeWidth * sizeHeight * qty * salePrice / 100) * 100; }
      int total() { return subtotal() - discountAmount; }
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
        case Col_SizeWidth:          return "size_width";
        case Col_SizeHeight:         return "size_height";
        case Col_UseArea:            return "use_area";
        case Col_SalePrice:          return "sale_price";
        case Col_BasePrice:          return "base_price";
        case Col_DiscountPercentage: return "discount_percentage";
        case Col_DiscountAmount:     return "discount_amount";
        case Col_Subtotal:           return "subtotal";
        case Col_Total:              return "total";
        case Col_Notes:              return "notes";
        case Col_CreatedAt:          return "created_at";
        case Col_UpdatedAt:          return "updated_at";
        default:                     return {};
    }
}

OrderItemEditorModel::OrderItemEditorModel(QObject *p)
    : m_tableModel(new QSqlTableModel(this)), QAbstractTableModel(p) {
        m_tableModel->setTable("products");
        m_tableModel->select();
        // tarik semua data dari database agar bisa digunakan untuk lookup di flags width dan height
        while(m_tableModel->canFetchMore()) m_tableModel->fetchMore();
    }

OrderItemEditorModel::~OrderItemEditorModel() {}

OrderItemEditorModel::ModelSaveResult OrderItemEditorModel::saveModel(const QSqlRecord &order) {
  auto newRows = pendingNewRows();
  auto edited  = editedCells();
  if(newRows.isEmpty() && edited.isEmpty()) return { .ok = false, .error = "Tidak ada data untuk disimpan" };
  auto ms = ModelSaveResult {};
  auto o_id = order.value("order_id");
  
  QList<qlonglong> insertedId;
  for(auto &newMap : newRows) {
    newMap["order_id"] = o_id;
    auto opt_oitem = oim.create(newMap);
    if(!opt_oitem.has_value()) {
      return {.ok = false, .error = oim.errorString()};
    }
    auto newItem  = *opt_oitem;
    insertedId << newItem.value("id").toLongLong();
  }
  
  QList<qlonglong> updatedId;
  QMap<int, QVariantMap> updates;
  for(auto const& [pair, edt] : edited.asKeyValueRange()) {
    auto cindex = index(pair.first, pair.second);
    auto u_id = index(pair.first, pair.second).siblingAtColumn(0).data(Qt::EditRole).toInt();
    if (!updates.contains(pair.first)) {
      updates[u_id] = { { columnKey(pair.second), edt } };
    } else {
      updates[u_id][columnKey(pair.second)] = edt;
    }
  }
  for(auto const &[upd_id, updmap] : updates.asKeyValueRange()) {
    bool updok = oim.update(upd_id, updmap);
    if (!updok) {
      return {.ok = false, .error = oim.errorString()};
    }
    updatedId << upd_id;
  }
  return {.ok = true, .error = ""};

}

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
    // jika row berasal dari database, kita simpan edit di m_editedCells
    if (ix.row() < m_fromDatabase.count()) {
        m_editedCells[qMakePair(ix.row(), ix.column())] = value;
    } else {
        // jika row baru, kita update langsung di m_newData
        int newDataRow = ix.row() - m_fromDatabase.count();
        if (newDataRow >= 0 && newDataRow < m_newData.count()) {
            const QString field = columnKey(ix.column());
            m_newData[newDataRow][field] = value;
        } else {
            return false; // index out of range untuk newData
        }
    }
    // tambahkan logic perhitungan subtotal jika quantity, sale_price, base_price, discount_percentage, atau discount_amount yang diedit
    bool subtotalChanged = false;
    QList<int> affectingColumns  { Col_Quantity, Col_SalePrice, Col_DiscountAmount, Col_SizeHeight, Col_SizeWidth };
    if (affectingColumns.contains(ix.column())) {
        int quantity = 0, salePrice = 0, disc = 0;
        double sizeWidth = 0 , sizeHeight = 0;
        if (ix.row() < m_fromDatabase.count()) {
            auto getEditedValue = [this](int row, int column) -> double {
                auto editKey = qMakePair(row, column);
                if (m_editedCells.contains(editKey))
                    return m_editedCells[editKey].toDouble();
                else
                    return m_fromDatabase.at(row).value(columnKey(column)).toDouble();
            };
            quantity = getEditedValue(ix.row(), Col_Quantity);
            salePrice = getEditedValue(ix.row(), Col_SalePrice);
            sizeWidth = getEditedValue(ix.row(), Col_SizeWidth);
            sizeHeight = getEditedValue(ix.row(), Col_SizeHeight);
            disc = getEditedValue(ix.row(), Col_DiscountAmount);
            
            ItemCalc ic {.sizeWidth = sizeWidth, .sizeHeight = sizeHeight, .qty = quantity, .salePrice = salePrice, .discountAmount = disc };
            
            // masukan subtotal ke editedCells agar langsung update di view
            auto subtotalKey = qMakePair(ix.row(), Col_Subtotal);
            if (!m_editedCells.contains(subtotalKey) || m_editedCells[subtotalKey] != ic.subtotal()) {
                m_editedCells[subtotalKey] = ic.subtotal();
                subtotalChanged = true;
            }
        } else {
            int newDataRow = ix.row() - m_fromDatabase.count();
            quantity =   m_newData.at(newDataRow).value(columnKey(Col_Quantity  )).toInt();
            salePrice =  m_newData.at(newDataRow).value(columnKey(Col_SalePrice )).toInt();
            sizeWidth =  m_newData.at(newDataRow).value(columnKey(Col_SizeWidth )).toDouble();
            sizeHeight = m_newData.at(newDataRow).value(columnKey(Col_SizeHeight)).toDouble();
            disc =       m_newData.at(newDataRow).value(columnKey(Col_DiscountAmount)).toInt();
            ItemCalc ic {.sizeWidth = sizeWidth, .sizeHeight = sizeHeight, .qty = quantity, .salePrice = salePrice, .discountAmount = disc };
            m_newData[newDataRow][columnKey(Col_Subtotal)] = ic.subtotal();
            subtotalChanged = true;
        }
    }
    if (subtotalChanged) {
        emit dataChanged(this->index(ix.row(), Col_Subtotal), this->index(ix.row(), Col_Subtotal), {Qt::DisplayRole});
        emit dataChanged(ix, ix, {role});
        emit this->subtotalChanged();
    } else {
        emit dataChanged(ix, ix, {role});
    }
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
        case Col_SizeWidth:          return "Lebar";
        case Col_SizeHeight:         return "Tinggi";
        case Col_UseArea:            return "Hitung Luas";
        case Col_SalePrice:          return "Harga Jual";
        case Col_BasePrice:          return "Harga Dasar";
        case Col_DiscountPercentage: return "Disc %";
        case Col_DiscountAmount:     return "Disc Amount";
        case Col_Subtotal:           return "Subtotal";
        case Col_Total:              return "Total";
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
    
    if (mi.column() == Col_SizeWidth || mi.column() == Col_SizeHeight) {
        if(mi.siblingAtColumn(Col_UseArea).data(Qt::EditRole).toInt() == 1) {
          f |= Qt::ItemIsEditable;
          return f;
        }
        return f;
    }

    // Make read-only columns non-editable
    if (mi.column() != Col_Id && mi.column() != Col_OrderId &&
        mi.column() != Col_CreatedAt && mi.column() != Col_UpdatedAt &&
        mi.column() != Col_Subtotal && mi.column() == Col_Total)
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
    newItem["sale_price"]          = 0;
    newItem["base_price"]          = 0;
    newItem["discount_percentage"] = 0.0;
    newItem["discount_amount"]     = 0.0;
    newItem["subtotal"]            = 0;
    newItem["total"]               = 0;
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

    emit subtotalChanged();

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
    emit subtotalChanged();
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
    emit subtotalChanged(); 
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
    emit subtotalChanged();
}

bool OrderItemEditorModel::isDirty() const
{
    return !m_editedCells.isEmpty() || !m_newData.isEmpty();
}

double OrderItemEditorModel::calculateSubtotal() const
{
    double total = 0;
    for (int r=0; r < rowCount(); ++r) {
        double st = data(index(r, Col_Total), Qt::EditRole).toInt();
        total += st;
    }
    return total;
}

int OrderItemEditorModel::orderItemIdForRow(int row) const {
  if (row < 0 && row >= rowCount()) return -1;
  auto ix = index(row, 0);
  return ix.data(Qt::EditRole).toInt();
}