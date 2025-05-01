#ifndef TableManager_H
    #define TableManager_H

#include <QObject>
#include <QVariant>

class TableManager : public QObject {
    Q_OBJECT
    QString tableName;

public:
    TableManager(const QString& tableName, QObject* = nullptr);
    ~TableManager();
    QSqlRecord record() const;

public slots:
    virtual bool insert(const QVariantMap&);
    virtual bool erase(const QVariantMap&);
    virtual bool update(const QVariantMap&, const QVariantMap&);

signals:
    void inserted(const QVariantMap&);
    void erased(const QVariantMap&);
    void updated(const QVariantMap&);
};

#endif