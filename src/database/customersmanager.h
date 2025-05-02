#ifndef CustomersManager_H
#define CustomersManager_H

#include <QObject>
#include <QVariant>
#include <QMap>

class CustomersManager : public QObject {
    Q_OBJECT

public:
    CustomersManager(QObject *parent);
    ~CustomersManager();

    bool add(const QVariantMap&);
    bool erase(const QVariantMap&);
    bool update(const QVariantMap&, const QVariantMap&);

signals:
    void added(int count);
    void erased(int count);
    void updated(int count);
};

#endif