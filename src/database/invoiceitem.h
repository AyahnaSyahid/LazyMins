#ifndef InvoiceItemH
#define InvoiceItemH

#include "databaseitem.h"
#include "orderitem.h"
#include "invoicepaymentsitem.h"
#include <QList>
#include <QDateTime>

class InvoiceItem : public DatabaseItem {
    
    QList<qint64> _orders {};
    QString _error;
  public:
    InvoiceItem(qint64 iid=-1);
    ~InvoiceItem();
    
    tableNameAssign(invoices)
    implementDatabaseItem
    
    // Fields
    qint64 user_id = -1,
           invoice_id = -1,
           customer_id = -1,
           modified_by = -1,
           total_amount = 0,
           total_paid = 0,
           discount_amount = 0;
    
    QString physical_iid;
            
    QDateTime invoice_date = QDateTime(),
              due_date = QDateTime(),
              created_date = QDateTime(),
              modified_date = QDateTime();
    
    QList<OrderItem> getOrders() const;
    void updateTotal();
    void updatePaid();
    bool addOrder(qint64 oi);
    bool removeOrder(qint64 oi);
    bool addPayment(InvoicePaymentsItem* ipi);
};

#endif //InvoiceItemH