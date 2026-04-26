#include "basemanager.h"
#include <QSqlDriver>

void debugMap(const QVariantMap&);

QSqlDatabase BaseManager::connection;

QMap<QString, QStringList> BaseManager::s_columnCache;
QMap<QString, bool> BaseManager::s_dependencyCheckPassed;

QSqlQuery BaseManager::baseQuery() {
  return QSqlQuery {connection};
}

BaseManager::BaseManager(const QString& tableName, bool useSoftDelete)
    : m_errorString {}, m_tableName(tableName), m_useSoftDelete(useSoftDelete)
{
    baseDependencyCheck();
}

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
    auto query = baseQuery();
    
    QString deleteCondition = getDeleteCondition();
    QString sql = QString("SELECT * FROM %1 WHERE id = :id %2")
                      .arg(m_tableName, deleteCondition.isEmpty() ? "" : "AND " + deleteCondition);
    query.prepare(sql);
    query.bindValue(":id", id);
    
    if (!query.exec()) {
    qDebug() << "Error getting record from" << m_tableName << ":" << query.lastError().text();
    return std::nullopt;
    }
    if (query.next()) return query.record();
    return std::nullopt;
}

bool BaseManager::update(int id, const QVariantMap& params)
{
    resetErrorString();
    
    auto opt_rBefore = getById(id);
    if(!opt_rBefore.has_value()) {
      QString err ("Tidak dapat menemukan Record saat mencoba Update");
      setErrorString(err);
      qWarning() << "[BaseManager::update] Error: " << err;
      return false;
    }
    
    auto rBefore = *opt_rBefore;
    
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
        auto opt_rAfter = getById(id);
        if (!opt_rAfter.has_value()) {
          QString err("Tidak dapat menemukan Record saat update");
          qWarning() << "[BaseManager::update] Error :" << err;
          setErrorString(err);
          return false;
        }
        
        auto rAfter = *opt_rAfter;
        // Hook after update
        if ( !afterUpdate(id, rBefore, rAfter) ) return false;
    }
    return success;
}

bool BaseManager::remove(int id)
{
    // Hook before delete
    if( !beforeDelete(id) ) return false;
    
    auto opt_op = getById(id);
    
    if (!opt_op.has_value()) {
      QString err ("Id tidak ditemukan. Data corrupt ?");
      qWarning() << "[BaseManager::remove] " << m_tableName << ": " << err;
      setErrorString(err);
      return false;
    }
    
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
        if ( !afterDelete(id, *opt_op) ) return false;
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
    QList<QSqlRecord> recordList;
    QSqlQuery query = baseQuery();
    
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
            recordList << query.record();
        }
    } else {
        qDebug() << "Error getting recordList from" << m_tableName << ":" << query.lastError().text();
        qDebug() << "Query:" << query.lastQuery();
    }
    return recordList;
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
        if (!m_useSoftDelete && key == "deleted_at") continue;

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
    auto cc = s_columnCache.value(m_tableName);
    if (cc.count()) {
      if (cc.contains("updated_at") && !params.contains("updated_at")) {
        params["updated_at"] = QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss");
      }
    }
    return true;
}

bool BaseManager::afterUpdate(int id, const QSqlRecord& a, const QSqlRecord& b)
{
    // Hook kosong - override di derived class jika perlu
    Q_UNUSED(id)
    Q_UNUSED(a)
    Q_UNUSED(b)
    return true;
}

bool BaseManager::beforeDelete(int id)
{
    // Hook kosong - override di derived class jika perlu
    Q_UNUSED(id)
    return true;
}

bool BaseManager::afterDelete(int id, const QSqlRecord&)
{
    // Hook kosong - override di derived class jika perlu
    Q_UNUSED(id)
    return true;
}

void BaseManager::baseDependencyCheck()
{
    if (m_tableName.isEmpty()) return ;
    if (!s_dependencyCheckPassed.contains(m_tableName)) {
        bool passed = checkDependencies();
        s_dependencyCheckPassed[m_tableName] = passed;
        if (!passed) {
            qWarning() << "[BaseManager] Dependency check failed for table:" << m_tableName;
        } else {
            qWarning() << "[BaseManager] Dependency check passed for table:" << m_tableName;
        }
    }
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

bool BaseManager::qexec(QSqlQuery &query)
{
    if(!query.exec()) {
        m_errorString = query.lastError().text();
        return false;
    }
    return true;
}

QString BaseManager::generateCode(const QString& tableName,
                                   const QString& numberColumn,
                                   const QString& prefix,
                                   int padWidth,
                                   bool useDate)
{
    QString fullPrefix = prefix;
    if (useDate) {
        fullPrefix += QDateTime::currentDateTimeUtc().toString("yyyyMMdd") + "-";
    }

    QSqlQuery q(BaseManager::connection);
    q.prepare(QString(
        "SELECT COALESCE(MAX(CAST(SUBSTR(%1, :offset) AS INTEGER)), 0) + 1 AS next_val "
        "FROM %2 WHERE %1 LIKE :prefix"
    ).arg(numberColumn, tableName));
    q.bindValue(":offset", fullPrefix.length() + 1);
    q.bindValue(":prefix", fullPrefix + "%");

    if (q.exec() && q.next()) {
        int next = q.value("next_val").toInt();
        return QString("%1%2").arg(fullPrefix).arg(next, padWidth, 10, QChar('0'));
    }

    return QString("%1%2").arg(fullPrefix).arg(QDateTime::currentMSecsSinceEpoch());
}

QString BaseManager::dateToSql(const QDate& d) { return d.toString("yyyy-MM-dd"); }
QString BaseManager::dateTimeToSql(const QDateTime& d) { return d.toString("yyyy-MM-dd HH:mm:ss"); }
