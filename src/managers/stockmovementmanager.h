#pragma once
#include "basemanager.h"

// StockMovementManager bersifat IMMUTABLE (append-only).
// update() dan remove() diblokir.
// afterCreate() otomatis mengupdate products.stock.
class StockMovementManager : public BaseManager
{
public:
    explicit StockMovementManager();

    QList<QSqlRecord> getByProduct(int productId, const QString& orderBy = "movement_date DESC");
    QList<QSqlRecord> getByReference(const QString& referenceType, int referenceId);
    QList<QSqlRecord> getByDateRange(const QDate& from, const QDate& to, int productId = -1);

    // DIBLOKIR — stock movements bersifat immutable
    bool update(int id, const QVariantMap& params) override;
    bool remove(int id) override;
    
    // HELPER
    std::optional<QSqlRecord> recordMovement(int productId,
                                          const QString& movementType, // "in"|"out"|"adjustment"
                                          double quantity,
                                          double stockBefore,
                                          int adminId,
                                          const QString& referenceType = QString(),
                                          int referenceId              = -1,
                                          const QString& notes         = QString());

protected:
    // Sebelum insert: hitung stock_before & stock_after otomatis
    bool beforeCreate(QVariantMap& params) override;
    // Setelah insert: update products.stock
    bool afterCreate(const QSqlRecord& record) override;
};
