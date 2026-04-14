#pragma once
#include "basemanager.h"

class PaymentManager : public BaseManager
{
public:
    explicit PaymentManager();

    QList<QSqlRecord> getByInvoice(int invoiceId);
    QList<QSqlRecord> getByStatus(const QString& verificationStatus);
    bool verify(int id, int verifiedByAdminId);
    bool cancel(int id);
    static QString nextNumber();

protected:

    // Hook utama: setelah payment dibuat,
    // catat transaksi kas hanya jika verification_status = 'verified'
    bool beforeCreate(QVariantMap& params) override;
    bool afterCreate(const QSqlRecord& record) override;
};
