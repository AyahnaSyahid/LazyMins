#include "advancedquerymodel.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QRegularExpression>

// =============================================================================
// Constructor / Destructor
// =============================================================================

AdvancedQueryModel::AdvancedQueryModel(QObject *parent)
    : QSqlQueryModel(parent)
    , m_currentPage(1)
    , m_pageSize(50)
    , m_totalRecords(0)
    , m_rowMappingDirty(true)
{}

AdvancedQueryModel::~AdvancedQueryModel()
{}

// =============================================================================
// Query Management
// =============================================================================

void AdvancedQueryModel::setQueryArgs(const QString &query,
                                      const QVariantMap &bindings,
                                      const QSqlDatabase &db)
{
    m_baseQuery    = query;
    m_queryBindings = bindings;
    m_database     = db;

    // Reset paging, sorting, and all pending operations
    m_currentPage  = 1;
    m_sortInfo     = SortInfo();
    m_filter.clear();
    m_pendingInserts.clear();
    m_pendingUpdates.clear();
    m_pendingDeletes.clear();
    m_newRowToPendingIndex.clear();
    m_rowMappingDirty = true;

    rebuildQuery();
}

// =============================================================================
// Primary Key
// =============================================================================

void AdvancedQueryModel::setPrimaryKeyColumn(const QString &columnName)
{
    m_primaryKeyColumn = columnName;
}

QString AdvancedQueryModel::primaryKeyColumn() const
{
    return m_primaryKeyColumn;
}

QString AdvancedQueryModel::resolvePrimaryKeyName() const
{
    if (!m_primaryKeyColumn.isEmpty())
        return m_primaryKeyColumn;

    QSqlRecord rec = QSqlQueryModel::record();
    if (rec.isEmpty())
        return QString();

    return rec.fieldName(0); // fallback: kolom pertama
}

QString AdvancedQueryModel::getPrimaryKey(int originalRow) const
{
    QString pkName = resolvePrimaryKeyName();
    if (pkName.isEmpty())
        return "";

    QSqlRecord rec = QSqlQueryModel::record();
    int pkCol = rec.indexOf(pkName);
    if (pkCol < 0)
        pkCol = 0; // fallback

    return QSqlQueryModel::data(QSqlQueryModel::index(originalRow, pkCol), Qt::DisplayRole).toString();
}

QString AdvancedQueryModel::resolveTableName() const
{
    static QRegularExpression tableRe(
        R"(\bFROM\s+[`"\[]?(\w+)[`"\]]?\b)",
        QRegularExpression::CaseInsensitiveOption);

    QRegularExpressionMatch m = tableRe.match(m_baseQuery);
    if (m.hasMatch())
        return m.captured(1);

    return QString();
}

// =============================================================================
// Paging
// =============================================================================

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

int AdvancedQueryModel::currentPage()  const { return m_currentPage; }
int AdvancedQueryModel::pageSize()     const { return m_pageSize; }
int AdvancedQueryModel::totalRecords() const { return m_totalRecords; }

int AdvancedQueryModel::totalPages() const
{
    if (m_pageSize <= 0) return 0;
    return (m_totalRecords + m_pageSize - 1) / m_pageSize;
}

// =============================================================================
// Sorting
// =============================================================================

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

// =============================================================================
// Filtering
// =============================================================================

void AdvancedQueryModel::setFilter(const QString &filter)
{
    if (m_filter != filter) {
        m_filter      = filter;
        m_currentPage = 1; // kembali ke halaman pertama
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
        bool ok = true;

        // 1. Deletes first (avoid FK constraint issues)
        for (const QVariant &pk : std::as_const(m_pendingDeletes)) {
            if (!deleteFromDatabaseByPk(pk)) { ok = false; break; }
        }

        // 2. Updates
        if (ok) {
            for (auto it = m_pendingUpdates.cbegin(); it != m_pendingUpdates.cend(); ++it) {
                if (!updateToDatabaseByPk(it.key(), it.value().changes)) { ok = false; break; }
            }
        }

        // 3. Inserts
        if (ok) {
            for (const PendingInsert &ins : std::as_const(m_pendingInserts)) {
                if (isRowFilled(ins) && !insertToDatabase(ins)) { ok = false; break; }
            }
        }

        if (ok) {
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
    refresh();
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
        newInsert.tempId = -(m_pendingInserts.size() + 1); // temporary negative ID
        m_pendingInserts.append(newInsert);

        int pendingIndex = m_pendingInserts.size() - 1;
        int viewRow      = QSqlQueryModel::rowCount() + pendingIndex;
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
            // Existing DB row: mark for deletion using PK
            int      originalRow = mapToOriginalRow(i);
            QString pk          = getPrimaryKey(originalRow);
            if (!pk.isEmpty()) {
                m_pendingDeletes.insert(pk);
                m_pendingUpdates.remove(pk);
            }
        } else {
            // New (pending-insert) row: remove immediately
            auto it = m_newRowToPendingIndex.find(i);
            if (it != m_newRowToPendingIndex.end()) {
                m_pendingInserts.removeAt(it.value());
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

    int      row         = index.row();
    int      originalRow = mapToOriginalRow(row);
    QString pk          = getPrimaryKey(originalRow);

    // Rows marked for deletion are not editable
    if (!pk.isEmpty() && m_pendingDeletes.contains(pk))
        f &= ~Qt::ItemIsEditable;

    return f;
}

bool AdvancedQueryModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    int row      = index.row();
    int col      = index.column();
    int baseRows = QSqlQueryModel::rowCount();

    // -----------------------------------------------------------------------
    // DisplayRole override (e.g. from delegate for FK / static-choice columns)
    // Stored separately so it is never written to the database.
    // -----------------------------------------------------------------------
    if (role == Qt::DisplayRole) {
        if (row < baseRows) {
            int      originalRow = mapToOriginalRow(row);
            QString pk          = getPrimaryKey(originalRow);
            if (!pk.isEmpty()) {
                m_pendingUpdates[pk].displayChanges[col] = value;
                emit dataChanged(index, index, {Qt::DisplayRole});
            }
        } else {
            auto it = m_newRowToPendingIndex.find(row);
            if (it != m_newRowToPendingIndex.end())
                m_pendingInserts[it.value()].displayData[col] = value;
            emit dataChanged(index, index, {Qt::DisplayRole});
        }
        return true;
    }

    if (role != Qt::EditRole)
        return false;

    // -----------------------------------------------------------------------
    // EditRole – the real value that goes to the database
    // -----------------------------------------------------------------------
    if (row < baseRows) {
        int      originalRow = mapToOriginalRow(row);
        QString pk           = getPrimaryKey(originalRow);

        if (pk.isEmpty())
            return false;

        // Cannot edit a row that is pending deletion
        if (m_pendingDeletes.contains(pk))
            return false;

        // Skip if value has not actually changed (ATAU revert ke nilai asli)
        QVariant original = getOriginalValue(originalRow, col);
        if (original == value) {
            // Jika sebelumnya pernah diubah, kita harus menghapus perubahannya
            if (m_pendingUpdates.contains(pk) && m_pendingUpdates[pk].changes.contains(col)) {
                m_pendingUpdates[pk].changes.remove(col);
                
                // Jika baris ini sudah tidak memiliki perubahan lain, hapus dari pending updates
                if (m_pendingUpdates[pk].changes.isEmpty() && m_pendingUpdates[pk].displayChanges.isEmpty()) {
                    m_pendingUpdates.remove(pk);
                }
                // Beritahu UI untuk me-render ulang sel ini kembali ke nilai aslinya
                emit dataChanged(index, index, {Qt::EditRole, Qt::DisplayRole, PendingUpdateRole});
            }
            return true;
        }

        // Jika nilainya benar-benar baru/berbeda dari database
        m_pendingUpdates[pk].changes[col] = value;
        emit dataChanged(index, index, {Qt::EditRole, Qt::DisplayRole, PendingUpdateRole});
        return true;
    } else {
        // Pending-insert row
        auto it = m_newRowToPendingIndex.find(row);
        if (it == m_newRowToPendingIndex.end())
            return false;

        m_pendingInserts[it.value()].data[col] = value;
        emit dataChanged(index, index, {Qt::EditRole, Qt::DisplayRole});
        return true;
    }
}

QVariant AdvancedQueryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row      = index.row();
    int col      = index.column();
    int baseRows = QSqlQueryModel::rowCount();

    // -----------------------------------------------------------------------
    // Custom roles
    // -----------------------------------------------------------------------
    if (role == PendingInsertRole)
        return row >= baseRows;

    if (role == PendingDeleteRole) {
        if (row < baseRows) {
            QString pk = getPrimaryKey(mapToOriginalRow(row));
            return !pk.isEmpty() && m_pendingDeletes.contains(pk);
        }
        return false;
    }

    if (role == PendingUpdateRole) {
        if (row < baseRows) {
            QString pk = getPrimaryKey(mapToOriginalRow(row));
            return !pk.isEmpty() && m_pendingUpdates.contains(pk);
        }
        return false;
    }

    if (role == RowStatusRole)
        return static_cast<int>(rowStatus(row));

    // -----------------------------------------------------------------------
    // DisplayRole
    // -----------------------------------------------------------------------
    if (role == Qt::DisplayRole) {
        if (row >= baseRows) {
            // Pending-insert row: prefer display override, then raw value
            auto it = m_newRowToPendingIndex.find(row);
            if (it != m_newRowToPendingIndex.end()) {
                const PendingInsert &ins = m_pendingInserts.at(it.value());
                if (ins.displayData.contains(col))
                    return ins.displayData[col];
                return ins.data.value(col, QVariant());
            }
            return QVariant();
        }

        int      originalRow = mapToOriginalRow(row);
        QString  pk          = getPrimaryKey(originalRow);

        // Check display override (set by delegate for FK / bool columns)
        if (!pk.isEmpty() && m_pendingUpdates.contains(pk)) {
            const QMap<int, QVariant> &disp = m_pendingUpdates[pk].displayChanges;
            if (disp.contains(col))
                return disp[col];
        }

        // Check edit-role pending value (fallback display)
        if (!pk.isEmpty() && m_pendingUpdates.contains(pk)) {
            const QMap<int, QVariant> &changes = m_pendingUpdates[pk].changes;
            if (changes.contains(col))
                return changes[col];
        }

        return QSqlQueryModel::data(QSqlQueryModel::index(originalRow, col), Qt::DisplayRole);
    }

    // -----------------------------------------------------------------------
    // EditRole
    // -----------------------------------------------------------------------
    if (role == Qt::EditRole) {
        if (row >= baseRows) {
            auto it = m_newRowToPendingIndex.find(row);
            if (it != m_newRowToPendingIndex.end())
                return m_pendingInserts.at(it.value()).data.value(col, QVariant());
            return QVariant();
        }

        int      originalRow = mapToOriginalRow(row);
        QString pk           = getPrimaryKey(originalRow);

        if (!pk.isEmpty() && m_pendingUpdates.contains(pk)) {
            const QMap<int, QVariant> &changes = m_pendingUpdates[pk].changes;
            if (changes.contains(col))
                return changes[col];
        }

        return QSqlQueryModel::data(QSqlQueryModel::index(originalRow, col), Qt::DisplayRole);
    }

    // -----------------------------------------------------------------------
    // All other roles – delegate to base (DB rows only)
    // -----------------------------------------------------------------------
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

    return QSqlQueryModel::rowCount() + m_pendingInserts.size();
}

QVariant AdvancedQueryModel::headerData(int section,
                                         Qt::Orientation orientation,
                                         int role) const
{
    if (orientation == Qt::Vertical && role == Qt::DisplayRole)
        return section + 1; // 1-based row numbers

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
    if (row >= QSqlQueryModel::rowCount())
        return false;

    QString pk = getPrimaryKey(mapToOriginalRow(row));
    return !pk.isEmpty() && m_pendingUpdates.contains(pk);
}

bool AdvancedQueryModel::isNewRow(int row) const
{
    return row >= QSqlQueryModel::rowCount();
}

bool AdvancedQueryModel::isDeletedRow(int row) const
{
    if (row >= QSqlQueryModel::rowCount())
        return false;

    QString pk = getPrimaryKey(mapToOriginalRow(row));
    return !pk.isEmpty() && m_pendingDeletes.contains(pk);
}

AdvancedQueryModel::RowStatus AdvancedQueryModel::rowStatus(int row) const
{
    if (isNewRow(row))     return NewRow;
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

    for (auto it = m_queryBindings.cbegin(); it != m_queryBindings.cend(); ++it)
        q.bindValue(it.key(), it.value());

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        qWarning() << "AdvancedQueryModel::rebuildQuery error:" << m_lastError;
        return;
    }

    beginResetModel();
    QSqlQueryModel::setQuery(std::move(q));
    m_rowMappingDirty = true;
    endResetModel();
}

QString AdvancedQueryModel::buildFullQuery() const
{
    QString wrapped = QString("SELECT * FROM (%1) AS _aqm_base").arg(m_baseQuery);

    if (!m_filter.isEmpty())
        wrapped += QString(" WHERE %1").arg(m_filter);

    if (m_sortInfo.enabled && m_sortInfo.column >= 0) {
        QString colName;
        if (QSqlQueryModel::columnCount() > m_sortInfo.column)
            colName = QSqlQueryModel::record().fieldName(m_sortInfo.column);

        if (colName.isEmpty())
            colName = QString::number(m_sortInfo.column + 1);

        wrapped += QString(" ORDER BY %1 %2")
                       .arg(colName)
                       .arg(m_sortInfo.order == Qt::AscendingOrder ? "ASC" : "DESC");
    }

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

    QString countQuery = m_filter.isEmpty()
        ? QString("SELECT COUNT(*) FROM (%1) AS _aqm_count").arg(m_baseQuery)
        : QString("SELECT COUNT(*) FROM (%1) AS _aqm_count WHERE %2").arg(m_baseQuery).arg(m_filter);

    QSqlDatabase db = m_database.isValid() ? m_database : QSqlDatabase::database();
    QSqlQuery q(db);
    q.prepare(countQuery);

    for (auto it = m_queryBindings.cbegin(); it != m_queryBindings.cend(); ++it)
        q.bindValue(it.key(), it.value());

    if (q.exec() && q.next())
        m_totalRecords = q.value(0).toInt();
    else {
        qWarning() << "AdvancedQueryModel::updateTotalRecords error:" << q.lastError().text();
        m_totalRecords = 0;
    }
}

// =============================================================================
// Private: Row Mapping
// =============================================================================

int AdvancedQueryModel::mapToOriginalRow(int viewRow) const
{
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
    for (int i = m_pendingInserts.size() - 1; i >= 0; --i) {
        if (m_pendingInserts[i].data.isEmpty())
            m_pendingInserts.removeAt(i);
    }
    m_rowMappingDirty = true;
}

bool AdvancedQueryModel::executeDatabaseOperation(const QString &sql, const QVariantMap &bindings)
{
    QSqlDatabase db = m_database.isValid() ? m_database : QSqlDatabase::database();
    QSqlQuery q(db);
    q.prepare(sql);

    for (auto it = bindings.cbegin(); it != bindings.cend(); ++it)
        q.bindValue(it.key(), it.value());

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

    QSqlRecord rec = QSqlQueryModel::record();
    if (rec.isEmpty())
        return false;

    QString tableName = resolveTableName();
    if (tableName.isEmpty()) {
        m_lastError = "Cannot determine table name from base query for INSERT";
        return false;
    }

    QStringList colNames, placeholders;
    QVariantMap bindings;

    for (auto it = insert.data.cbegin(); it != insert.data.cend(); ++it) {
        int colIndex = it.key();
        if (colIndex < 0 || colIndex >= rec.count())
            continue;

        QString colName     = rec.fieldName(colIndex);
        QString placeholder = QString(":ins_%1").arg(colName);
        colNames     << colName;
        placeholders << placeholder;
        bindings[placeholder] = it.value();
    }

    if (colNames.isEmpty())
        return false;

    QString sql = QString("INSERT INTO %1 (%2) VALUES (%3)")
                      .arg(tableName)
                      .arg(colNames.join(", "))
                      .arg(placeholders.join(", "));

    return executeDatabaseOperation(sql, bindings);
}

bool AdvancedQueryModel::updateToDatabaseByPk(const QVariant &pk,
                                               const QMap<int, QVariant> &changes)
{
    if (changes.isEmpty())
        return true;

    QSqlRecord rec = QSqlQueryModel::record();
    if (rec.isEmpty())
        return false;

    QString tableName = resolveTableName();
    if (tableName.isEmpty()) {
        m_lastError = "Cannot determine table name from base query for UPDATE";
        return false;
    }

    QString pkName = resolvePrimaryKeyName();

    QStringList setClauses;
    QVariantMap bindings;

    for (auto it = changes.cbegin(); it != changes.cend(); ++it) {
        int colIndex = it.key();
        if (colIndex < 0 || colIndex >= rec.count())
            continue;

        QString colName     = rec.fieldName(colIndex);
        QString placeholder = QString(":upd_%1").arg(colName);
        setClauses << QString("%1 = %2").arg(colName).arg(placeholder);
        bindings[placeholder] = it.value();
    }

    if (setClauses.isEmpty())
        return true;

    bindings[":pk_val"] = pk;

    QString sql = QString("UPDATE %1 SET %2 WHERE %3 = :pk_val")
                      .arg(tableName)
                      .arg(setClauses.join(", "))
                      .arg(pkName);

    return executeDatabaseOperation(sql, bindings);
}

bool AdvancedQueryModel::deleteFromDatabaseByPk(const QVariant &pk)
{
    QSqlRecord rec = QSqlQueryModel::record();
    if (rec.isEmpty())
        return false;

    QString tableName = resolveTableName();
    if (tableName.isEmpty()) {
        m_lastError = "Cannot determine table name from base query for DELETE";
        return false;
    }

    QString pkName = resolvePrimaryKeyName();

    QString     sql = QString("DELETE FROM %1 WHERE %2 = :pk_val").arg(tableName).arg(pkName);
    QVariantMap bindings;
    bindings[":pk_val"] = pk;

    return executeDatabaseOperation(sql, bindings);
}
