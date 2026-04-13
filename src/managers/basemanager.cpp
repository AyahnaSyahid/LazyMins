#include "basemanager.h"
#include <QSqlDriver>

void debugMap(const QVariantMap&);

QSqlDatabase BaseManager::connection;

QMap<QString, QStringList> BaseManager::s_columnCache;

QSqlQuery BaseManager::baseQuery() {
  return QSqlQuery {connection};
}

BaseManager::BaseManager(const QString& tableName, bool useSoftDelete)
    : m_errorString {}, m_tableName(tableName), m_useSoftDelete(useSoftDelete)
{}

BaseManager::~BaseManager()
{}

// ============================================================================
// CRUD Operations
// ============================================================================

std::optional<QSqlRecord> BaseManager::create(const QVariantMap& params)
{
  m_lastInsertId = QVariant();
  resetErrorString();
  QVariantMap validatedParams = validateParams(params); 
  
  if (validatedParams.isEmpty()) { 
    setErrorString("Parameter Kosong");
    return std::nullopt;
  }

  // Hook before create
  if ( !beforeCreate(validatedParams) ) return std::nullopt;
  
  auto query = BaseManager::baseQuery();
  QString sql = buildInsertQuery(validatedParams);
  
  query.prepare(sql);
  
  // Bind values
  for (auto it = validatedParams.begin(); it != validatedParams.end(); ++it) {
      query.bindValue(":" + it.key(), it.value());
  }

  if (query.exec()) {
      m_lastInsertId = query.lastInsertId();
      int lastId = m_lastInsertId.toInt();
      auto record = getById(lastId);

      // Hook after create
      if (record && afterCreate(*record))
        return record;
  }
  
  qDebug() << "exec failed" << query.lastError().text() 
           << "last query :" << sql;
  setErrorString(query.lastError().text());
  return std::nullopt;
}

std::optional<QSqlRecord> BaseManager::getById(int id) const
{
    QSqlQuery query(BaseManager::connection);
    
    QString deleteCondition = getDeleteCondition();
    QString sql = QString("SELECT * FROM %1 WHERE id = :id %2")
                      .arg(m_tableName, deleteCondition.isEmpty() ? "" : "AND " + deleteCondition);
    query.prepare(sql);
    query.bindValue(":id", id);
    
    if (query.exec() && query.next()) {
        return query.record();
    } else if (!query.exec()) {
        qDebug() << "Error getting record from" << m_tableName << ":" << query.lastError().text();
    }
    
    return std::nullopt;
}

bool BaseManager::update(int id, const QVariantMap& params)
{
    resetErrorString();
    QVariantMap validatedParams = validateParams(params);
    
    // Update timestamp
    if (!validatedParams.contains("updated_at")) {
        validatedParams["updated_at"] = QDateTime::currentDateTimeUtc();
    }
    
    // Hook before update
    if ( !beforeUpdate(id, validatedParams) ) return false;
    
    QSqlQuery query(BaseManager::connection);
    QString deleteCondition = getDeleteCondition();
    QString sql = buildUpdateQuery(id, validatedParams);
    
    if (!deleteCondition.isEmpty()) {
        sql += " AND " + deleteCondition;
    }
    
    query.prepare(sql);
    query.bindValue(":id", id);
    
    // Bind values
    for (auto it = validatedParams.begin(); it != validatedParams.end(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }
    
    if (!query.exec()) {
        setErrorString("execution failed " + query.lastError().text());
        return false;
    }
    
    bool success = query.numRowsAffected() > 0;
    
    if (success) {
        auto record = getById(id);
        // Hook after update
        if ( !afterUpdate(id, *record) ) return false;
    }
    
    return success;
}

bool BaseManager::remove(int id)
{
    // Hook before delete
    if( !beforeDelete(id) ) return false;
    
    if (m_useSoftDelete) {
        return softDelete(id);
    }
    
    QSqlQuery query(BaseManager::connection);
    QString sql = QString("DELETE FROM %1 WHERE id = :id").arg(m_tableName);
    
    query.prepare(sql);
    query.bindValue(":id", id);
    
    if (!query.exec()) {
        qDebug() << "Error removing record from" << m_tableName << ":" << query.lastError().text();
        return false;
    }

    
    bool success = query.numRowsAffected() > 0;
    
    if (success) {
        // Hook after delete
        if ( !afterDelete(id) ) return false;
    }
    
    return success;
}

// ============================================================================
// Bulk Operations
// ============================================================================

QList<QSqlRecord> BaseManager::getAll(const QString& orderBy, int limit)
{
    QList<QSqlRecord> records;
    QSqlQuery query(BaseManager::connection);
    
    QString deleteCondition = getDeleteCondition();
    QString sql = QString("SELECT * FROM %1").arg(m_tableName);
    if (!deleteCondition.isEmpty()) sql += " WHERE " + deleteCondition;
    if (!orderBy.isEmpty()) sql += " ORDER BY " + orderBy;
    if (limit > 0) sql += QString(" LIMIT %1").arg(limit);
    
    if (query.exec(sql)) {
        while (query.next()) {
            records.append(query.record());
        }
    } else {
        qDebug() << "Error getting all records from" << m_tableName << ":" << query.lastError().text();
    }
    
    return records;
}

QList<QSqlRecord> BaseManager::getWhere(const QString& condition, 
                                        const QVariantMap& bindings,
                                        const QString& orderBy,
                                        int limit)
{
    QList<QSqlRecord> records;
    QSqlQuery query(BaseManager::connection);
    
    QString whereClause = condition;
    QString deleteCondition = getDeleteCondition();
    
    if (!whereClause.isEmpty() && !deleteCondition.isEmpty()) {
        whereClause = "(" + whereClause + ")" + " AND " + deleteCondition;
    } else if (!deleteCondition.isEmpty()) {
        whereClause = deleteCondition; // Remove "WHERE "
    }
    
    QString sql = QString("SELECT * FROM %1").arg(m_tableName);
    
    if (!whereClause.isEmpty()) {
        sql += " WHERE " + whereClause;
    }
    
    if (!orderBy.isEmpty()) {
        sql += " ORDER BY " + orderBy;
    }
    
    if (limit > 0) {
        sql += QString(" LIMIT %1").arg(limit);
    }
    
    query.prepare(sql);
    
    // Bind values
    for (auto it = bindings.begin(); it != bindings.end(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }
    
    if (query.exec()) {
        while (query.next()) {
            records.append(query.record());
        }
    } else {
        qDebug() << "Error getting records from" << m_tableName << ":" << query.lastError().text();
        qDebug() << "Query:" << query.lastQuery();
    }
    
    return records;
}

QList<QSqlRecord> BaseManager::getByIds(const QList<int>& ids)
{
    if (ids.isEmpty()) {
        return QList<QSqlRecord>();
    }
    
    QList<QSqlRecord> records;
    QSqlQuery query(BaseManager::connection);
    
    // Build placeholders
    QStringList placeholders;
    for (int i = 0; i < ids.size(); ++i) {
        placeholders << QString(":id%1").arg(i);
    }
    
    QString deleteCondition = getDeleteCondition();
    QString sql = QString("SELECT * FROM %1 WHERE id IN (%2)")
                      .arg(m_tableName, placeholders.join(", "));
    
    if (!deleteCondition.isEmpty()) sql += " AND " + deleteCondition;
    
    query.prepare(sql);
    
    // Bind values
    for (int i = 0; i < ids.size(); ++i) {
        query.bindValue(QString(":id%1").arg(i), ids[i]);
    }
    
    if (query.exec()) {
        while (query.next()) {
            records.append(query.record());
        }
    } else {
        qDebug() << "Error getting records by IDs from" << m_tableName << ":" << query.lastError().text();
    }
    
    return records;
}

// ============================================================================
// Soft Delete Support
// ============================================================================

bool BaseManager::softDelete(int id)
{
    QVariantMap params;
    params["deleted_at"] = QDateTime::currentDateTime();
    
    return update(id, params);
}

bool BaseManager::restore(int id)
{
    if (!m_useSoftDelete) {
        qDebug() << "Restore called but soft delete is not enabled for" << m_tableName;
        return false;
    }
    
    QSqlQuery query(BaseManager::connection);
    QString sql = QString("UPDATE %1 SET deleted_at = NULL, updated_at = :updated_at WHERE id = :id")
                      .arg(m_tableName);
    
    query.prepare(sql);
    query.bindValue(":id", id);
    query.bindValue(":updated_at", QDateTime::currentDateTime());
    
    if (!query.exec()) {
        qDebug() << "Error restoring record in" << m_tableName << ":" << query.lastError().text();
        return false;
    }
    
    return query.numRowsAffected() > 0;
}

// ============================================================================
// Transaction Support
// ============================================================================

/***
bool BaseManager::beginTransaction()
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) {
        qDebug() << "Error beginning transaction:" << db.lastError().text();
        return false;
    }
    return true;
}

bool BaseManager::commit()
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.commit()) {
        qDebug() << "Error committing transaction:" << db.lastError().text();
        return false;
    }
    return true;
}

bool BaseManager::rollback()
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.rollback()) {
        qDebug() << "Error rolling back transaction:" << db.lastError().text();
        return false;
    }
    return true;
}
***/

// ============================================================================
// Utility Methods
// ============================================================================

int BaseManager::count(const QString& condition, const QVariantMap& bindings)
{
    QSqlQuery query(BaseManager::connection);
    
    QString whereClause = condition;
    QString deleteCondition = getDeleteCondition();
    
    if (!whereClause.isEmpty() && !deleteCondition.isEmpty()) {
        whereClause = "(" + whereClause + ")" + " AND " + deleteCondition;
    } else if (!deleteCondition.isEmpty()) {
        whereClause = deleteCondition;
    }
    
    QString sql = QString("SELECT COUNT(*) as total FROM %1").arg(m_tableName);
    
    if (!whereClause.isEmpty()) {
        sql += " WHERE " + whereClause;
    }
    
    query.prepare(sql);
    
    // Bind values
    for (auto it = bindings.begin(); it != bindings.end(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }
    
    if (query.exec() && query.next()) {
        return query.value("total").toInt();
    }
    
    qDebug() << "Error counting records in" << m_tableName << ":" << query.lastError().text();
    return 0;
}

bool BaseManager::exists(int id)
{
    QSqlQuery query(BaseManager::connection);
    
    QString deleteCondition = getDeleteCondition();
    QString sql = QString("SELECT COUNT(*) as total FROM %1 WHERE id = :id")
                      .arg(m_tableName);
    if(!deleteCondition.isEmpty()) sql += " AND " + deleteCondition; 
    
    query.prepare(sql);
    query.bindValue(":id", id);
    
    if (query.exec() && query.next()) {
        return query.value("total").toInt() > 0;
    }
    
    return false;
}

// ============================================================================
// Protected Methods - Override jika perlu
// ============================================================================

QString BaseManager::buildInsertQuery(const QVariantMap& params)
{
    QStringList columns;
    QStringList placeholders;
    
    for (auto it = params.begin(); it != params.end(); ++it) {
        columns << it.key();
        placeholders << ":" + it.key();
    }
    
    return QString("INSERT INTO %1 (%2) VALUES (%3)")
        .arg(m_tableName, columns.join(", "), placeholders.join(", "));
}

QString BaseManager::buildUpdateQuery(int id, const QVariantMap& params)
{
    QStringList setPairs;
    
    for (auto it = params.begin(); it != params.end(); ++it) {
        setPairs << QString("%1 = :%1").arg(it.key());
    }
    
    return QString("UPDATE %1 SET %2 WHERE id = :id")
        .arg(m_tableName, setPairs.join(", "));
}

QVariantMap BaseManager::validateParams(const QVariantMap& params)
{
    if (m_tableName.isEmpty()) return params;

    // 1. Cek apakah skema tabel sudah ada di static cache
    if (!s_columnCache.contains(m_tableName)) {
        QSqlRecord schema = connection.record(m_tableName);
        QStringList fields;
        
        for (int i = 0; i < schema.count(); ++i) {
            fields << schema.fieldName(i);
        }
        
        // Simpan ke cache global agar bisa digunakan oleh instansi lain
        s_columnCache[m_tableName] = fields;
        
        qDebug() << "[Static Cache] Initialized schema for table:" << m_tableName 
                 << "with" << fields.count() << "columns";
    }

    // Ambil daftar kolom dari cache
    const QStringList& fieldNames = s_columnCache[m_tableName];

    if (fieldNames.isEmpty()) return params;

    QVariantMap filteredParams;
    // Gunakan iterator yang efisien untuk QVariantMap
    for (auto it = params.begin(); it != params.end(); ++it) {
        const QString& key = it.key();

        // A. Validasi keberadaan kolom di database
        if (!fieldNames.contains(key)) {
            qDebug() << "[Manager :" << m_tableName << "] :"
                     << "Unused parameter key :\"" << key << "\" Removed";
            continue;
        }

        // B. Proteksi Primary Key (Auto Increment)
        if (key == "id") continue;

        // C. Proteksi kolom Soft Delete internal
        if (m_useSoftDelete && key == "deleted_at") continue;

        filteredParams[key] = it.value();
    }
    
    return filteredParams;
}

bool BaseManager::beforeCreate(QVariantMap& params)
{
    // Hook kosong - override di derived class jika perlu
    Q_UNUSED(params)
    return true;
}

bool BaseManager::afterCreate(const QSqlRecord& record)
{
    // Hook kosong - override di derived class jika perlu
    Q_UNUSED(record)
    return true;
}

bool BaseManager::beforeUpdate(int id, QVariantMap& params)
{
    // Hook kosong - override di derived class jika perlu
    Q_UNUSED(id)
    Q_UNUSED(params)
    return true;
}

bool BaseManager::afterUpdate(int id, const QSqlRecord& record)
{
    // Hook kosong - override di derived class jika perlu
    Q_UNUSED(id)
    Q_UNUSED(record)
    return true;
}

bool BaseManager::beforeDelete(int id)
{
    // Hook kosong - override di derived class jika perlu
    Q_UNUSED(id)
    return true;
}

bool BaseManager::afterDelete(int id)
{
    // Hook kosong - override di derived class jika perlu
    Q_UNUSED(id)
    return true;
}

// ============================================================================
// Private Helper Methods
// ============================================================================

QString BaseManager::getDeleteCondition() const
{
    if (m_useSoftDelete) {
        return "deleted_at IS NULL";
    }
    return "";
}

void BaseManager::resetErrorString() {
  m_errorString = "";
}

QSqlRecord BaseManager::empty() const {
  return QSqlRecord();
}