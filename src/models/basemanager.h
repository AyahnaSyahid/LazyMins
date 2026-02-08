#ifndef BASEMANAGER_H
#define BASEMANAGER_H

#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QVariant>
#include <QVariantMap>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QList>

class BaseManager
{
public:
    // static database connection
    static QSqlDatabase connection;
    
    explicit BaseManager(const QString& tableName, bool useSoftDelete = false);
    virtual ~BaseManager();
    
    // CRUD Operations
    virtual QSqlRecord create(const QVariantMap& params);
    virtual QSqlRecord getById(int id);
    virtual bool update(int id, const QVariantMap& params);
    virtual bool remove(int id);
    
    // Bulk operations
    virtual QList<QSqlRecord> getAll(const QString& orderBy = "", int limit = -1);
    virtual QList<QSqlRecord> getWhere(const QString& condition, 
                                       const QVariantMap& bindings = QVariantMap(),
                                       const QString& orderBy = "",
                                       int limit = -1);
    virtual QList<QSqlRecord> getByIds(const QList<int>& ids);
    
    // Soft delete support
    virtual bool softDelete(int id);
    virtual bool restore(int id);
    
    // Transaction support
    static bool beginTransaction();
    static bool commit();
    static bool rollback();
    
    // Count
    virtual int count(const QString& condition = "", const QVariantMap& bindings = QVariantMap());
    
    // Exists
    virtual bool exists(int id);
    
protected:
    QString tableName() const { return m_tableName; }
    bool useSoftDelete() const { return m_useSoftDelete; }
    
    // Methods untuk di-override jika perlu custom behavior
    virtual QString buildInsertQuery(const QVariantMap& params);
    virtual QString buildUpdateQuery(int id, const QVariantMap& params);
    virtual QVariantMap validateParams(const QVariantMap& params);
    virtual void beforeCreate(QVariantMap& params);
    virtual void afterCreate(const QSqlRecord& record);
    virtual void beforeUpdate(int id, QVariantMap& params);
    virtual void afterUpdate(int id, const QSqlRecord& record);
    virtual void beforeDelete(int id);
    virtual void afterDelete(int id);
    
    // Helper methods
    QString getDeleteCondition() const;
    
private:
    QString m_tableName;
    bool m_useSoftDelete;
};

#endif // BASEMANAGER_H
