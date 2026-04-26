#pragma once
#include "basemanager.h"

class PaymentManager : public BaseManager
{
public:
    explicit PaymentManager();

    QList<QSqlRecord> getByInvoice(int invoiceId);
    QList<QSqlRecord> getByStatus(const QString& verificationStatus);
    bool verify(int id, int verifiedByAdminId = -1);
    bool cancel(int id, int verifier);
    static QString nextNumber();

protected:
    bool beforeCreate(QVariantMap& params) override;
};
