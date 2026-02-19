#include "advancedquerymodel.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QRegularExpression>

AdvancedQueryModel::AdvancedQueryModel(QObject *parent)
    : QSqlQueryModel(parent)
    , m_currentPage(1)
    , m_pageSize(20)
    , m_totalRecords(0)
    , m_rowMappingDirty(true)
{
}

AdvancedQueryModel::~AdvancedQueryModel()
{
}

void AdvancedQueryModel::setQueryArgs(const QString &query, const QVariantMap &bindings, const QSqlDatabase &db)
{
    m_baseQuery = query;
    m_queryBindings = bindings;
    m_database = db;

    // Reset paging and sorting
    m_currentPage = 1;
    m_sortInfo = SortInfo();
    m_filter.clear();
    m_pendingInserts.clear();
    m_pendingUpdates.clear();
    m_pendingDeletes.clear();
    m_newRowToPendingIndex.clear();
    m_rowMappingDirty = true;

    rebuildQuery();
}

void AdvancedQueryModel::setPage(int page)
{
    if (page < 1) page = 1;
    if (m_currentPage != page) {
        m_currentPage = page;
        rebuildQuery();
    }
}

void AdvancedQueryModel::setPageSize(int size)
{
    if (size > 0 && m_pageSize != size) {
        m_pageSize = size;
        rebuildQuery();
    }
}

int AdvancedQueryModel::currentPage() const
{
    return m_currentPage;
}

int AdvancedQueryModel::pageSize() const
{
    return m_pageSize;
}

int AdvancedQueryModel::totalPages() const
{
    if (m_pageSize <= 0) return 0;
    return (m_totalRecords + m_pageSize - 1) / m_pageSize;
}

int AdvancedQueryModel::totalRecords() const
{
    return m_totalRecords;
}

void AdvancedQueryModel::setSort(int column, Qt::SortOrder order)
{
    if (column >= 0 && column < this->columnCount()) {
        m_sortInfo = SortInfo(column, order);
        rebuildQuery();
    }
}


void AdvancedQueryModel::clearSort()
{
    if (m_sortInfo.enabled) {
        m_sortInfo.enabled = false;
        rebuildQuery();
    }
}

AdvancedQueryModel::SortInfo AdvancedQueryModel::currentSort() const
{
    return m_sortInfo;
}

void AdvancedQueryModel::setReadOnlyColumns(const QSet<int> &columns)
{
    m_readOnlyColumns = columns;
}

void AdvancedQueryModel::setReadOnlyColumn(int column, bool readOnly)
{
    if (readOnly)
        m_readOnlyColumns.insert(column);
    else
        m_readOnlyColumns.remove(column);
}

QSet<int> AdvancedQueryModel::readOnlyColumns() const
{
    return m_readOnlyColumns;
}

void AdvancedQueryModel::setFilter(const QString &filter)
{
    if (m_filter != filter) {
        m_filter = filter;
        m_currentPage = 1; // Reset ke halaman pertama saat filter berubah
        rebuildQuery();
    }
}

QString AdvancedQueryModel::currentFilter() const
{
    return m_filter;
}

// =============================================================================
// Transaction Management
// =============================================================================

bool AdvancedQueryModel::submitAll()
{
    bool success = true;
    m_lastError.clear();

    QSqlDatabase db = m_database.isValid() ? m_database : QSqlDatabase::database();
    if (db.transaction()) {
        bool transactionSuccess = true;

        // Process deletes first (untuk menghindari constraint issues)
        for (int originalRow : std::as_const(m_pendingDeletes)) {
            if (!deleteFromDatabase(originalRow)) {
                transactionSuccess = false;
                break;
            }
        }

        // Process updates
        if (transactionSuccess) {
            for (auto it = m_pendingUpdates.begin(); it != m_pendingUpdates.end(); ++it) {
                int originalRow = it.key();
                const QMap<int, QVariant> &changes = it.value().changes;
                if (!updateToDatabase(originalRow, changes)) {
                    transactionSuccess = false;
                    break;
                }
            }
        }

        // Process inserts
        if (transactionSuccess) {
            for (const PendingInsert &insert : std::as_const(m_pendingInserts)) {
                if (isRowFilled(insert) && !insertToDatabase(insert)) {
                    transactionSuccess = false;
                    break;
                }
            }
        }

        if (transactionSuccess) {
            if (!db.commit()) {
                m_lastError = db.lastError().text();
                success = false;
            } else {
                m_pendingInserts.clear();
                m_pendingUpdates.clear();
                m_pendingDeletes.clear();
                m_newRowToPendingIndex.clear();
                m_rowMappingDirty = true;
                rebuildQuery();
            }
        } else {
            db.rollback();
            success = false;
        }
    } else {
        m_lastError = "Failed to start database transaction";
        success = false;
    }

    return success;
}

void AdvancedQueryModel::revertAll()
{
    beginResetModel();
    m_pendingInserts.clear();
    m_pendingUpdates.clear();
    m_pendingDeletes.clear();
    m_newRowToPendingIndex.clear();
    m_rowMappingDirty = true;
    endResetModel();
}

// =============================================================================
// Row Operations
// =============================================================================

bool AdvancedQueryModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || count <= 0)
        return false;

    int insertPos = qBound(0, row, rowCount());

    beginInsertRows(parent, insertPos, insertPos + count - 1);

    for (int i = 0; i < count; ++i) {
        PendingInsert newInsert;
        newInsert.tempId = -(m_pendingInserts.size() + 1); // ID negatif sementara
        m_pendingInserts.append(newInsert);

        int pendingIndex = m_pendingInserts.size() - 1;
        // Hitung view row: baris baru diletakkan di akhir data yang sudah ada + pending inserts sebelumnya
        int viewRow = QSqlQueryModel::rowCount() + pendingIndex;
        m_newRowToPendingIndex[viewRow] = pendingIndex;
    }

    m_rowMappingDirty = true;
    endInsertRows();
    return true;
}

bool AdvancedQueryModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || count <= 0)
        return false;

    int baseRows = QSqlQueryModel::rowCount();

    beginRemoveRows(parent, row, row + count - 1);

    for (int i = row + count - 1; i >= row; --i) {
        if (i < baseRows) {
            // Baris dari database: tandai sebagai pending delete
            int originalRow = mapToOriginalRow(i);
            m_pendingDeletes.insert(originalRow);
            // Hapus update pending jika ada
            m_pendingUpdates.remove(originalRow);
        } else {
            // Baris baru (pending insert): hapus langsung
            auto it = m_newRowToPendingIndex.find(i);
            if (it != m_newRowToPendingIndex.end()) {
                int pendingIdx = it.value();
                m_pendingInserts.removeAt(pendingIdx);
                m_newRowToPendingIndex.erase(it);
            }
        }
    }

    m_rowMappingDirty = true;
    endRemoveRows();
    return true;
}

// =============================================================================
// Model Overrides
// =============================================================================

Qt::ItemFlags AdvancedQueryModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;

    // Baris yang ditandai delete masih ditampilkan tapi tidak bisa diedit
    int row = index.row();
    int originalRow = mapToOriginalRow(row);
    if (m_pendingDeletes.contains(originalRow)) {
        f &= ~Qt::ItemIsEditable;
    }
    if (m_readOnlyColumns.contains(index.column()))
      f &= ~Qt::ItemIsEditable;

    return f;
}

bool AdvancedQueryModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;
  
    int row = index.row();
    int col = index.column();
    
    if (m_readOnlyColumns.contains(col))
      return false;
    
    int baseRows = QSqlQueryModel::rowCount();

    if (row < baseRows) {
        // Baris existing dari database
        int originalRow = mapToOriginalRow(row);

        // Jangan edit baris yang sudah ditandai hapus
        if (m_pendingDeletes.contains(originalRow))
            return false;

        // Cek apakah nilai benar-benar berubah
        QVariant original = getOriginalValue(originalRow, col);
        if (original == value)
            return true; // Tidak ada perubahan

        m_pendingUpdates[originalRow].changes[col] = value;
        emit dataChanged(index, index, {Qt::EditRole, Qt::DisplayRole, PendingUpdateRole});
        return true;
    } else {
        // Baris pending insert
        auto it = m_newRowToPendingIndex.find(row);
        if (it == m_newRowToPendingIndex.end())
            return false;

        int pendingIdx = it.value();
        m_pendingInserts[pendingIdx].data[col] = value;
        emit dataChanged(index, index, {Qt::EditRole, Qt::DisplayRole});
        return true;
    }
}

QVariant AdvancedQueryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int col = index.column();
    int baseRows = QSqlQueryModel::rowCount();

    // Custom roles
    if (role == PendingInsertRole) {
        return row >= baseRows;
    }
    if (role == PendingDeleteRole) {
        if (row < baseRows) {
            int origRow = mapToOriginalRow(row);
            return m_pendingDeletes.contains(origRow);
        }
        return false;
    }
    if (role == PendingUpdateRole) {
        if (row < baseRows) {
            int origRow = mapToOriginalRow(row);
            return m_pendingUpdates.contains(origRow);
        }
        return false;
    }
    if (role == RowStatusRole) {
        return static_cast<int>(rowStatus(row));
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (row >= baseRows) {
            // Pending insert row
            auto it = m_newRowToPendingIndex.find(row);
            if (it != m_newRowToPendingIndex.end()) {
                int pendingIdx = it.value();
                const PendingInsert &ins = m_pendingInserts.at(pendingIdx);
                return ins.data.value(col, QVariant());
            }
            return QVariant();
        }

        int originalRow = mapToOriginalRow(row);

        // Jika ada pending update untuk kolom ini, kembalikan nilai yang diubah
        if (m_pendingUpdates.contains(originalRow)) {
            const QMap<int, QVariant> &changes = m_pendingUpdates[originalRow].changes;
            if (changes.contains(col)) {
                return changes[col];
            }
        }

        // Kembalikan nilai dari database
        return QSqlQueryModel::data(QSqlQueryModel::index(originalRow, col), role);
    }

    // Untuk role lain, delegasikan ke parent (hanya untuk baris database)
    if (row < baseRows) {
        int originalRow = mapToOriginalRow(row);
        return QSqlQueryModel::data(QSqlQueryModel::index(originalRow, col), role);
    }

    return QVariant();
}

int AdvancedQueryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    int baseCount = QSqlQueryModel::rowCount();

    // Hitung baris pending insert yang belum di-submit
    int newRowCount = m_pendingInserts.size();

    // Kurangi baris yang ditandai delete (hanya dari baris database)
    // Catatan: baris delete tetap ditampilkan dengan visual berbeda, tidak dikurangi dari count
    // Jika ingin menyembunyikan baris delete, uncomment baris berikut:
    // int deletedCount = m_pendingDeletes.size();
    // return baseCount - deletedCount + newRowCount;

    return baseCount + newRowCount;
}

QVariant AdvancedQueryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
        // Nomor baris
        return section + 1;
    }
    return QSqlQueryModel::headerData(section, orientation, role);
}

// =============================================================================
// Status Checking
// =============================================================================

bool AdvancedQueryModel::hasPendingChanges() const
{
    return !m_pendingInserts.isEmpty() ||
           !m_pendingUpdates.isEmpty() ||
           !m_pendingDeletes.isEmpty();
}

bool AdvancedQueryModel::isRowModified(int row) const
{
    int originalRow = mapToOriginalRow(row);
    return m_pendingUpdates.contains(originalRow);
}

bool AdvancedQueryModel::isNewRow(int row) const
{
    return row >= QSqlQueryModel::rowCount();
}

bool AdvancedQueryModel::isDeletedRow(int row) const
{
    if (row >= QSqlQueryModel::rowCount())
        return false;
    int originalRow = mapToOriginalRow(row);
    return m_pendingDeletes.contains(originalRow);
}

AdvancedQueryModel::RowStatus AdvancedQueryModel::rowStatus(int row) const
{
    if (isNewRow(row))    return NewRow;
    if (isDeletedRow(row)) return DeletedRow;
    if (isRowModified(row)) return ModifiedRow;
    return NormalRow;
}

// =============================================================================
// Utility
// =============================================================================

void AdvancedQueryModel::refresh()
{
    rebuildQuery();
}

QString AdvancedQueryModel::lastError() const
{
    return m_lastError;
}

// =============================================================================
// Private: Query Building
// =============================================================================

void AdvancedQueryModel::rebuildQuery()
{
    if (m_baseQuery.isEmpty())
        return;

    updateTotalRecords();

    QString fullQuery = buildFullQuery();

    QSqlDatabase db = m_database.isValid() ? m_database : QSqlDatabase::database();
    QSqlQuery q(db);
    q.prepare(fullQuery);

    for (auto it = m_queryBindings.begin(); it != m_queryBindings.end(); ++it) {
        q.bindValue(it.key(), it.value());
    }

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        qWarning() << "AdvancedQueryModel::rebuildQuery error:" << m_lastError;
        return;
    }

    beginResetModel();
    QSqlQueryModel::setQuery(std::move(q));
    m_rowMappingDirty = true;
    // Jaga pending operations tetap hidup setelah rebuild
    endResetModel();
}

QString AdvancedQueryModel::buildFullQuery() const
{
    // Bungkus query dasar sebagai subquery agar filter dan sort aman diterapkan
    QString wrapped = QString("SELECT * FROM (%1) AS _aqm_base").arg(m_baseQuery);

    // Terapkan filter
    if (!m_filter.isEmpty()) {
        wrapped += QString(" WHERE %1").arg(m_filter);
    }

    // Terapkan sort
    if (m_sortInfo.enabled && m_sortInfo.column >= 0) {
        // Ambil nama kolom dari record
        QString colName;
        if (QSqlQueryModel::columnCount() > m_sortInfo.column) {
            colName = QSqlQueryModel::record().fieldName(m_sortInfo.column);
        }

        if (colName.isEmpty()) {
            colName = QString::number(m_sortInfo.column + 1); // Fallback ke posisi
        }

        wrapped += QString(" ORDER BY %1 %2")
                       .arg(colName)
                       .arg(m_sortInfo.order == Qt::AscendingOrder ? "ASC" : "DESC");
    }

    // Terapkan paging (LIMIT/OFFSET – standar SQL)
    if (m_pageSize > 0) {
        int offset = (m_currentPage - 1) * m_pageSize;
        wrapped += QString(" LIMIT %1 OFFSET %2").arg(m_pageSize).arg(offset);
    }

    return wrapped;
}

void AdvancedQueryModel::updateTotalRecords()
{
    if (m_baseQuery.isEmpty()) {
        m_totalRecords = 0;
        return;
    }

    QString countQuery;
    if (!m_filter.isEmpty()) {
        countQuery = QString("SELECT COUNT(*) FROM (%1) AS _aqm_count WHERE %2")
                         .arg(m_baseQuery)
                         .arg(m_filter);
    } else {
        countQuery = QString("SELECT COUNT(*) FROM (%1) AS _aqm_count").arg(m_baseQuery);
    }

    QSqlDatabase db = m_database.isValid() ? m_database : QSqlDatabase::database();
    QSqlQuery q(db);
    q.prepare(countQuery);

    for (auto it = m_queryBindings.begin(); it != m_queryBindings.end(); ++it) {
        q.bindValue(it.key(), it.value());
    }

    if (q.exec() && q.next()) {
        m_totalRecords = q.value(0).toInt();
    } else {
        qWarning() << "AdvancedQueryModel::updateTotalRecords error:" << q.lastError().text();
        m_totalRecords = 0;
    }
}

// =============================================================================
// Private: Row Mapping
// =============================================================================

int AdvancedQueryModel::mapToOriginalRow(int viewRow) const
{
    // Dalam implementasi sederhana ini, view row = original row
    // (tidak ada filter yang menyembunyikan baris di level model)
    // Jika diperlukan mapping lebih kompleks (e.g., hide deleted rows),
    // implementasi ini bisa diperluas.
    return viewRow;
}

int AdvancedQueryModel::mapFromOriginalRow(int originalRow) const
{
    return originalRow;
}

// =============================================================================
// Private: Helpers
// =============================================================================

void AdvancedQueryModel::cleanupEmptyNewRows()
{
    // Hapus pending inserts yang sama sekali kosong (tidak ada data yang diisi)
    for (int i = m_pendingInserts.size() - 1; i >= 0; --i) {
        if (m_pendingInserts[i].data.isEmpty()) {
            m_pendingInserts.removeAt(i);
        }
    }
    m_rowMappingDirty = true;
}

bool AdvancedQueryModel::executeDatabaseOperation(const QString &sql, const QVariantMap &bindings)
{
    QSqlDatabase db = m_database.isValid() ? m_database : QSqlDatabase::database();
    QSqlQuery q(db);
    q.prepare(sql);

    for (auto it = bindings.begin(); it != bindings.end(); ++it) {
        q.bindValue(it.key(), it.value());
    }

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        qWarning() << "AdvancedQueryModel::executeDatabaseOperation error:" << m_lastError;
        qWarning() << "SQL:" << sql;
        return false;
    }
    return true;
}

QVariant AdvancedQueryModel::getOriginalValue(int originalRow, int column) const
{
    return QSqlQueryModel::data(QSqlQueryModel::index(originalRow, column), Qt::DisplayRole);
}

bool AdvancedQueryModel::isRowFilled(const PendingInsert &insert) const
{
    // Sebuah baris dianggap terisi jika memiliki setidaknya satu nilai non-null
    if (insert.data.isEmpty())
        return false;

    for (const QVariant &val : insert.data) {
        if (!val.isNull() && val.isValid())
            return true;
    }
    return false;
}

// =============================================================================
// Private: Database Operations
// =============================================================================

bool AdvancedQueryModel::insertToDatabase(const PendingInsert &insert)
{
    if (insert.data.isEmpty())
        return false;

    // Bangun INSERT dari nama kolom yang ada di record
    QSqlRecord rec = QSqlQueryModel::record();
    if (rec.isEmpty())
        return false;

    QStringList colNames;
    QStringList placeholders;
    QVariantMap bindings;

    for (auto it = insert.data.begin(); it != insert.data.end(); ++it) {
        int colIndex = it.key();
        if (colIndex < 0 || colIndex >= rec.count())
            continue;

        QString colName = rec.fieldName(colIndex);
        colNames << colName;
        QString placeholder = QString(":ins_%1").arg(colName);
        placeholders << placeholder;
        bindings[placeholder] = it.value();
    }

    if (colNames.isEmpty())
        return false;

    // Ekstrak nama tabel dari query dasar
    // Asumsi query berbentuk: SELECT ... FROM tableName ...
    static QRegularExpression tableRe(
        R"(\bFROM\s+[`"\[]?(\w+)[`"\]]?\b)",
        QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch m = tableRe.match(m_baseQuery);
    if (!m.hasMatch()) {
        m_lastError = "Cannot determine table name from base query for INSERT";
        return false;
    }
    QString tableName = m.captured(1);

    QString sql = QString("INSERT INTO %1 (%2) VALUES (%3)")
                      .arg(tableName)
                      .arg(colNames.join(", "))
                      .arg(placeholders.join(", "));

    return executeDatabaseOperation(sql, bindings);
}

bool AdvancedQueryModel::updateToDatabase(int originalRow, const QMap<int, QVariant> &changes)
{
    if (changes.isEmpty())
        return true;

    QSqlRecord rec = QSqlQueryModel::record();
    if (rec.isEmpty())
        return false;

    // Ekstrak nama tabel
    static QRegularExpression tableRe(
        R"(\bFROM\s+[`"\[]?(\w+)[`"\]]?\b)",
        QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch m = tableRe.match(m_baseQuery);
    if (!m.hasMatch()) {
        m_lastError = "Cannot determine table name from base query for UPDATE";
        return false;
    }
    QString tableName = m.captured(1);

    // Asumsi kolom pertama adalah primary key
    QString pkName = rec.fieldName(0);
    QVariant pkValue = QSqlQueryModel::data(QSqlQueryModel::index(originalRow, 0), Qt::DisplayRole);

    QStringList setClauses;
    QVariantMap bindings;

    for (auto it = changes.begin(); it != changes.end(); ++it) {
        int colIndex = it.key();
        if (colIndex < 0 || colIndex >= rec.count())
            continue;

        QString colName = rec.fieldName(colIndex);
        QString placeholder = QString(":upd_%1").arg(colName);
        setClauses << QString("%1 = %2").arg(colName).arg(placeholder);
        bindings[placeholder] = it.value();
    }

    if (setClauses.isEmpty())
        return true;

    bindings[":pk_val"] = pkValue;

    QString sql = QString("UPDATE %1 SET %2 WHERE %3 = :pk_val")
                      .arg(tableName)
                      .arg(setClauses.join(", "))
                      .arg(pkName);

    return executeDatabaseOperation(sql, bindings);
}

bool AdvancedQueryModel::deleteFromDatabase(int originalRow)
{
    QSqlRecord rec = QSqlQueryModel::record();
    if (rec.isEmpty())
        return false;

    // Ekstrak nama tabel
    static QRegularExpression tableRe(
        R"(\bFROM\s+[`"\[]?(\w+)[`"\]]?\b)",
        QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch m = tableRe.match(m_baseQuery);
    if (!m.hasMatch()) {
        m_lastError = "Cannot determine table name from base query for DELETE";
        return false;
    }
    QString tableName = m.captured(1);

    // Asumsi kolom pertama adalah primary key
    QString pkName = rec.fieldName(0);
    QVariant pkValue = QSqlQueryModel::data(QSqlQueryModel::index(originalRow, 0), Qt::DisplayRole);

    QString sql = QString("DELETE FROM %1 WHERE %2 = :pk_val").arg(tableName).arg(pkName);
    QVariantMap bindings;
    bindings[":pk_val"] = pkValue;

    return executeDatabaseOperation(sql, bindings);
}
