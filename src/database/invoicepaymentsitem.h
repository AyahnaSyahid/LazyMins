#ifndef InvoicePaymentsItemH
#define InvoicePaymentsItemH

#include "databaseitem.h"
#include <QDateTime>

class InvoicePaymentsItem : public DatabaseItem {
  public:
    InvoicePaymentsItem(qint64 ip = -1);
    ~InvoicePaymentsItem();
    implementDatabaseItem;
    tableNameAssign(invoice_payments);
    
    qint64 payment_id = -1,
           invoice_id = -1,
           amount = 0,
           created_by = 0,
           modified_by = 0;
    QString method,
            note;
    QDateTime payment_date = QDateTime(),
              modified_date = QDateTime(),
              created_date = QDateTime();
};

#endif //InvoicePaymentsItemH