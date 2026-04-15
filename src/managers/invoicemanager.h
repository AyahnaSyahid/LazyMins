#pragma once
#include "basemanager.h"

class InvoiceManager : public BaseManager
{
public:
    explicit InvoiceManager();

    std::optional<QSqlRecord> getByNumber(const QString& invoiceNumber) const;
    QList<QSqlRecord> getByCustomer(int customerId, const QString& orderBy = "created_at DESC");
    QList<QSqlRecord> getByStagingStatus(const QString& status);
    QList<QSqlRecord> getBySettlementStatus(const QString& status);
    QList<QSqlRecord> getActive(const QString& orderBy = "created_at DESC", int limit = -1);

    bool updateStagingStatus(int id, const QString& status);
    bool updateSettlementStatus(int id, const QString& status);
    bool recalculate(int id);

    bool addOrder(int id, int oid);
    bool addOrders(int id, QList<int> oids);
    bool removeOrder(int id, int oid);
    bool removeOrders(int id, QList<int> oids);

    // Dipanggil oleh PaymentManager::afterCreate untuk update paid_amount
    bool updatePaidAmount(int id, int paidAmount);

    bool deactivate(int id);

    // Generate nomor invoice berikutnya dengan format INV-YYYYMMDD-00001
    // Aman dipanggil tanpa instansiasi objek, misal: InvoiceManager::nextNumber()
    static QString nextNumber();

protected:
    bool beforeCreate(QVariantMap& params) override;
};
