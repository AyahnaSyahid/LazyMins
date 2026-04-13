#pragma once

// ============================================================================
// PaymentManager — tabel: payments
// ============================================================================

#include "basemanager.h"
#include "managers.h"

class PaymentManager : public BaseManager
{
public:
    explicit PaymentManager()
        : BaseManager("payments") {}

    QList<QSqlRecord> getByOrder(int orderId);
    QList<QSqlRecord> getByCustomer(int customerId);
    QList<QSqlRecord> getByStatus(const QString& status);
    QList<QSqlRecord> getByDateRange(const QDate& from, const QDate& to);
    QList<QSqlRecord> getByInvoice(int invoiceId);
    
    std::optional<QSqlRecord> findByPaymentNumber(const QString& paymentNumber);

    bool verify(int id, int verifiedByAdminId);
    bool cancelPayment(int id);

    static QString generatePaymentNumber(const QString& prefix = "PYM");

protected:
    bool beforeCreate(QVariantMap& params) override;
    bool beforeUpdate(int id, QVariantMap& params) override;
    // bool afterUpdate(int, const QSqlRecord& rc);
    bool afterCreate(const QSqlRecord&) override;
    bool afterUpdate(int id, const QSqlRecord& before, const QSqlRecord& after) override;
};
