#ifndef TableManager_H
#define TableManager_H

#include <QObject>
#include <QVariant>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

class TableManager : public QObject {
    Q_OBJECT
    QString _tableName;
    QSqlError _lastError();
public:
    TableManager(const QString& tableName, QObject* = nullptr);
    ~TableManager();
    QSqlRecord record() const;
    const QSqlError& lastError() const { return _lastError; }
    const QString& tableName() const { return _tableName; }

public slots:
    virtual bool add(const QVariantMap&);
    virtual bool erase(const QVariantMap&);
    virtual bool update(const QVariantMap&, const QVariantMap&);

signals:
    void inserted(const QVariant lastId);
    void erased(const QVariant erasedId);
    void updated(const QVariant updatedId);
};

#endif