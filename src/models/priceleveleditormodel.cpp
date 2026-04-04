#include "priceleveleditormodel.h"
#include <QSqlRecord>

PriceLevelEditorModel::PriceLevelEditorModel(QObject *parent)
    : QAbstractTableModel(parent) {}

void PriceLevelEditorModel::setProductId(int pid) {
    m_productId = pid;
    loadLevels();
}

int PriceLevelEditorModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return m_definedLevels.size();
}

int PriceLevelEditorModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return 2; // Col 0: Level Name, Col 1: Price
}

QVariant PriceLevelEditorModel::data(const QModelIndex& ix, int role) const {
    if (!ix.isValid() || ix.row() >= m_definedLevels.size()) return QVariant();

    const auto& level = m_definedLevels.at(ix.row());

    if (role == IdRole) return level.id;
    if (role == IsNewRole) return level.isNew;

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (ix.column() == 0) return level.level_name;
        if (ix.column() == 1) {
            // Cek cache edit dulu, jika tidak ada baru ambil data asli DB
            if (m_editPrices.contains(ix.row())) return m_editPrices.value(ix.row());
            return m_loadedPrices.value(ix.row());
        }
    }
    return QVariant();
}

bool PriceLevelEditorModel::setData(const QModelIndex& ix, const QVariant& va, int role) {
    if (!ix.isValid() || role != Qt::EditRole) return false;

    int row = ix.row();
    bool changed = false;

    if (ix.column() == 0 && m_definedLevels[row].isNew) {
        // Hanya boleh edit nama jika baris baru
        m_definedLevels[row].level_name = va.toString();
        changed = true;
    } 
    else if (ix.column() == 1) {
        // Logika Dirty Checking untuk Harga
        if (m_loadedPrices.value(row) == va) {
            if (m_editPrices.contains(row)) {
                m_editPrices.remove(row);
                changed = true;
            }
        } else {
            m_editPrices[row] = va;
            changed = true;
        }
    }

    if (changed) {
        emit dataChanged(ix, ix, {Qt::DisplayRole, Qt::EditRole});
        return true;
    }
    return false;
}

Qt::ItemFlags PriceLevelEditorModel::flags(const QModelIndex& ix) const {
    if (!ix.isValid()) return Qt::NoItemFlags;
    
    Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    
    if (ix.column() == 1) {
        f |= Qt::ItemIsEditable; // Harga selalu editable
    } else if (ix.column() == 0 && m_definedLevels.at(ix.row()).isNew) {
        f |= Qt::ItemIsEditable; // Nama hanya editable jika baru
    }
    
    return f;
}

QVariant PriceLevelEditorModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) return QVariant();
    return (section == 0) ? "Level Name" : "Price";
}

void PriceLevelEditorModel::loadLevels() {
    beginResetModel();
    m_definedLevels.clear();
    m_loadedPrices.clear();
    m_editPrices.clear();

    if (m_productId == -1) {
        endResetModel();
        return;
    }

    QSqlQuery q(BaseManager::connection);
    q.prepare(R"--(
        SELECT pl.id, pl.level_name, pl.description, pl.discount_percentage, pr.price
        FROM price_levels pl 
        LEFT JOIN product_prices pr ON pr.price_level_id = pl.id AND pr.product_id = :pid
        ORDER BY pl.id ASC
    )--");
    q.bindValue(":pid", m_productId);

    if (q.exec()) {
        int row = 0;
        while (q.next()) {
            m_definedLevels.append({
                q.value("id").toInt(),
                q.value("level_name").toString(),
                q.value("description").toString(),
                q.value("discount_percentage").toDouble(),
                false // isNew
            });
            m_loadedPrices.insert(row++, q.value("price"));
        }
    }
    endResetModel();
}

void PriceLevelEditorModel::addNewLevel() {
    int row = m_definedLevels.size();
    beginInsertRows(QModelIndex(), row, row);
    
    m_definedLevels.append({-1, "New Level", "", 0.0, true});
    m_editPrices.insert(row, 0); // Default price for new row
    
    endInsertRows();
}

bool PriceLevelEditorModel::commit() {
    PriceLevelManager lvMgr;
    ProductPriceManager prMgr;

    // Gunakan transaksi dari koneksi static BaseManager
    BaseManager::connection.transaction();

    for (int i = 0; i < m_definedLevels.size(); ++i) {
        auto& level = m_definedLevels[i];

        // 1. Jika level baru, buat dulu di tabel price_levels
        if (level.isNew) {
            QVariantMap lvParams;
            lvParams["level_name"] = level.level_name;
            lvParams["discount_percentage"] = level.discount_percentage;
            
            auto newRec = lvMgr.create(lvParams);
            if (newRec) {
                level.id = newRec->value("id").toInt();
                level.isNew = false;
            } else {
                BaseManager::connection.rollback();
                return false;
            }
        }

        // 2. Jika harga diedit atau ini baris baru yang punya harga
        if (m_editPrices.contains(i)) {
            int priceVal = m_editPrices[i].toInt();
            if (!prMgr.upsert(m_productId, level.id, priceVal)) {
                BaseManager::connection.rollback();
                return false;
            }
        }
    }

    if (BaseManager::connection.commit()) {
        loadLevels(); // Refresh untuk sinkronisasi state
        return true;
    }
    return false;
}