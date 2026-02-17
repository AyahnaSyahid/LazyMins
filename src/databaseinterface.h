#ifndef DATABASEINTEFACE_H
#define DATABASEINTEFACE_H

#include "databasetransaction.h"
#include "invoicedatatype.h"

#include <QObject>
#include <QSqlDatabase>
#include <QVariant>
#include <optional>


class DatabaseInterface : public QObject
{
  Q_OBJECT
  public:
    
    static DatabaseInterface &instance();
    std::optional<PrintInvoiceParams> getPrintInvoiceParams(int invoice_id) const;
    std::optional<InvoiceData> getInvoiceData(int invoice_id) const;
    std::optional<StoreInfoData> getStoreInfo(QSqlDatabase &db) const;
    QList<QSqlRecord> getPaymentRecords(int invoice_id) const;
    QSqlDatabase database() const;
    std::optional<QSqlRecord> getInvoiceRecord(int invoice_id) const;
    
  public slots:
    // bool saveInvoiceData(const QVariant &va);
    bool saveInvoiceData(const InvoiceData &ida);
    bool saveInvoiceAndPayment(const InvoiceData&, int amount, const QString& method, QSqlRecord &ref);
    bool savePayment(const QSqlRecord&, const QString& by, int amount, const QString& method);
    bool saveStoreInfoData(const StoreInfoData& si);
    bool updateInvoice(int invoice_id, const InvoiceData &newData, const QList<PaymentData> payments, int cashBack);
  
  signals:
    void saveDone(bool ok);
    void tableUpdate(const QList<QString> &tables);
  
  private:
    DatabaseInterface() : QObject(nullptr) {}
    ~DatabaseInterface() {}
};

#endif