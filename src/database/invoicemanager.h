#ifndef InvoiceManager_H
#define InvoiceManager_H

#include "tablemanager.h"

class QSqlRecord;
class QSqlError;
class Database;
class InvoiceManager : public TableManager {
    Q_OBJECT

public:
    explicit InvoiceManager(Database* =nullptr);
    ~InvoiceManager();


public slots:
    void insertRecord(const QSqlRecord&);
    void updateRecord(const QSqlRecord&);
    void removeRecord(const QSqlRecord&);

signals:
    void insertStatus(const QSqlError&, const QSqlRecord&);
    void updateStatus(const QSqlError&, const QSqlRecord&);
    void removeStatus(const QSqlError&, const QSqlRecord&);
};


#endif
