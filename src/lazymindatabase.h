#ifndef LazyMinDatabase_H
#define LazyMinDatabase_H

#include <QObject>

#include "datastruct.h"

class LazyMinDatabase : public QObject {
    Q_OBJECT

public:
    LazyMinDatabase(QObject* = nullptr);
    ~LazyMinDatabase();

public slots:
    int addProduct(const StructProduct&);
    bool setProduct(int id, const StructProduct&);
    bool removeProduct(int id);

signals:
    void productAdded(const StructProduct&);
    void productRemoved(const StructProduct&);
    void productModified(const StructProduct&);
};

#endif