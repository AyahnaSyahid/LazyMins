#pragma once

#include "basemanager.h"

// ============================================================================
// StockMovementManager — tabel: stock_movements
// ============================================================================
class StockMovementManager : public BaseManager
{
public:
    explicit StockMovementManager()
        : BaseManager("stock_movements") {}

    QList<QSqlRecord> getByProduct(int productId, int limit = -1);
    QList<QSqlRecord> getByType(const QString& movementType); // 'in' | 'out' | 'adjustment'
    QList<QSqlRecord> getByDateRange(const QDate& from, const QDate& to);
    QList<QSqlRecord> getByReference(const QString& referenceType, int referenceId);

    // Convenience recorder
    std::optional<QSqlRecord> recordMovement(int productId,
                                             const QString& type,
                                             qreal quantity,
                                             qreal stockBefore,
                                             qreal stockAfter,
                                             int adminId,
                                             const QString& referenceType = "",
                                             int referenceId = -1,
                                             const QString& notes = "");

};
