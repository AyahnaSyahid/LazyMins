#pragma once
#include "basemanager.h"

class OrderManager : public BaseManager
{
public:
    explicit OrderManager();

    QList<QSqlRecord> getByCustomer(int customerId, const QString& orderBy = "created_at DESC");
    QList<QSqlRecord> getByStatus(const QString& status, const QString& orderBy = "deadline_date");
    QList<QSqlRecord> getByInvoice(int invoiceId);
    bool updateStatus(int id, const QString& status);
    bool updateSubtotal(int id, int subtotal);
    bool linkInvoice(int orderId, int invoiceId, const QString& invoiceNumber);
    static QString nextNumber();

protected:
    bool beforeCreate(QVariantMap& params) override;
    bool afterCreate(const QSqlRecord& record) override;
};
