#pragma once
#include "basemanager.h"

class OrderManager : public BaseManager
{
public:
    static QString nextNumber();    
    explicit OrderManager();
    QList<QSqlRecord> getByCustomer(int customerId, const QString& orderBy = "created_at DESC");
    QList<QSqlRecord> getByStatus(const QString& status, const QString& orderBy = "deadline_date");
    QList<QSqlRecord> getByInvoice(int invoiceId);
    bool updateStagingStatus(int id, const QString& status);
    bool setInvoiceId(int id, int invoiceId);
    bool addItems(int id, QList<int> itemIds);
    bool recalculate(int oid);

protected:
    bool beforeCreate(QVariantMap& params) override;
    bool afterCreate(const QSqlRecord& record) override;
};
