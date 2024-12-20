#ifndef ExpenseItemH
#define ExpenseItemH

#include "databaseitem.h"
#include <QDateTime>

class ExpenseItem : public DatabaseItem {
public:
    ExpenseItem(qint64 eid=-1);
    ~ExpenseItem();
    
    implementDatabaseItem;
    tableNameAssign(expenses);
    
    qint64 expense_id = -1,
           division_id = -1,
           amount = 0,
           created_by = -1,
           modified_by = -1;
           
    QString description,
            category;
    
    QDateTime created_date,
              expense_date,
              modified_date;
};

#endif // ExpenseItemH