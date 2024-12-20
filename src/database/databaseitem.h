#ifndef DatabaseItemH
#define DatabaseItemH

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QtDebug>

#define qValueAssign(N, T) \
N = q.value(#N).value<T>();

#define implementDatabaseItem \
    bool save() override;\
    bool erase() override;\

#define tableNameAssign(TN) \
    virtual QString tableName() const override { return QString::fromUtf8(#TN); }

#define debugError(QSQLQUERY)\
    if(QSQLQUERY.lastError().isValid()) qDebug() << q.lastError().text(); 

struct DatabaseItem {
    virtual bool save()  = 0;
    virtual bool erase()  = 0;
    virtual bool revert() { return false; }
    virtual qint64 count() {
        if(tableName().isEmpty()) return 0;
        q.exec(QString("SELECT COUNT(*) FROM %1").arg(tableName()));
        if(q.next()) return q.value(0).toLongLong();
        return 0;
    }
    virtual QString tableName() const { return ""; }
    virtual QString createTableQuery() const { return QString(); }
    virtual bool operator==(const DatabaseItem& item) const { return false; }
    virtual bool operator!=(const DatabaseItem& item) const { return !(operator==(item)); }
  protected:
    QSqlQuery q;
};


#endif // DatabaseItemH