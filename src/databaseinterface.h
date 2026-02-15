#ifndef DATABASEINTEFACE_H
#define DATABASEINTEFACE_H

#include "invoicedatatype.h"

#include <QObject>
#include <QSqlDatabase>
#include <QVariant>

class DatabaseInterface : public QObject
{
  Q_OBJECT
  public:
    static DatabaseInterface &instance();
    PrintInvoiceParams getPrintInvoiceParams(int invoice_id) const;
    InvoiceData getInvoiceData(int invoice_id) const;
    StoreInfoData getStoreInfo(QSqlDatabase &db) const;
    QList<QSqlRecord> getPaymentRecords(int invoice_id) const;
    QSqlDatabase database() const;
    
  public slots:
    // bool saveInvoiceData(const QVariant &va);
    bool saveInvoiceData(const InvoiceData &ida);
    bool saveInvoiceAndPayment(const InvoiceData&, int amount, const QString& method, QSqlRecord &ref);
    bool savePayment(const QSqlRecord&, int amount, const QString& method);
    bool saveStoreInfoData(const StoreInfoData& si);
    bool updateInvoice(int invoice_id, const InvoiceData &newData);
  
  signals:
    void saveDone(bool ok);
    void tableUpdate(const QList<QString> &tables);
  
  private:
    DatabaseInterface() : QObject(nullptr) {}
    ~DatabaseInterface() {}
};

#endif