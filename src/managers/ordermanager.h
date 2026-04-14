#pragma once

#include "basemanager.h"

// ============================================================================
// OrderManager — tabel: orders
// ============================================================================
class OrderManager : public BaseManager
{
public:
    explicit OrderManager()
        : BaseManager("orders") {}

    // Filters
    QList<QSqlRecord> getByStatus(const QString& status, const QString& orderBy = "order_date DESC");
    QList<QSqlRecord> getByPaymentStatus(const QString& paymentStatus);
    QList<QSqlRecord> getByCustomer(int customerId);
    QList<QSqlRecord> getByDateRange(const QDate& from, const QDate& to);
    QList<QSqlRecord> getByInvoice(int id);
    QList<QSqlRecord> getPending();
    QList<QSqlRecord> getOverdue();

    std::optional<QSqlRecord> findByOrderNumber(const QString& orderNumber);

    // Status transitions
    bool updateStagingStatus(int id, const QString& newStatus);
    bool cancel(int id);
    bool markCompleted(int id);
    
    // Number generation  (prefix from app_settings, e.g. "ORD")
    static QString generateOrderNumber(const QString& prefix = "ORD");
    
    bool updateSubtotal(int order_id);

protected:
    bool beforeCreate(QVariantMap& params) override;
    bool beforeUpdate(int, QVariantMap& params) override;
    bool afterUpdate(int id, const QSqlRecord&, const QSqlRecord&) override;
};
