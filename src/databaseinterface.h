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
    InvoiceData getInvoiceData(int invoice_id);
    StoreInfoData getStoreInfo(QSqlDatabase &db) const;
    QSqlDatabase database();
    
  public slots:
    // bool saveInvoiceData(const QVariant &va);
    bool saveInvoiceData(const InvoiceData &ida);
    bool saveInvoiceAndPayment(const InvoiceData&, int amount, const QString& method, QSqlRecord &ref);
    bool savePayment(const QSqlRecord&, int amount, const QString& method);
    bool saveStoreInfoData(const StoreInfoData& si);
  
  signals:
    void saveDone(bool ok);
    void tableUpdate(const QList<QString> &tables);
  
  private:
    DatabaseInterface() : QObject(nullptr) {}
    ~DatabaseInterface() {}
};

#endif