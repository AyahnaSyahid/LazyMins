#ifndef DivisionItemH
#define DivisionItemH

#include "databaseitem.h"
#include <QDateTime>

class DivisionItem : public DatabaseItem {
public:
    DivisionItem(qint64 did=-1);
    DivisionItem(const QString& name);
    ~DivisionItem();
    
    implementDatabaseItem;
    tableNameAssign(invoices);
    
    qint64 division_id = -1;
    QString division_name = QString();
    QDateTime created_date = QDateTime();
};

#endif // DivisionItemH