#pragma once

#include "basemanager.h"

class InvoiceManager : public BaseManager
{
public:
    explicit InvoiceManager()
        : BaseManager("invoices") {}

    // Query helpers
    QList<QSqlRecord> getByStatus(const QString& status, const QString& orderBy = "issue_date DESC");
    QList<QSqlRecord> getByCustomer(int customerId);
    QList<QSqlRecord> getByDateRange(const QDate& from, const QDate& to);
    QList<QSqlRecord> getOverdue();

    std::optional<QSqlRecord> findByInvoiceNumber(const QString& invoiceNumber);

    // Status transitions
    bool updateStatus(int id, const QString& newStatus);
    bool cancel(int id);
    bool markPaid(int id);
    
    bool updateInvoiceBalances(int invoiceId);
    
    // Number generation (daily format like orders)
    static QString generateInvoiceNumber(const QString& prefix = "INV");

protected:
    bool beforeCreate(QVariantMap& params) override;
};