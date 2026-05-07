#pragma once
#include "basemanager.h"

// StockMovementManager bersifat IMMUTABLE (append-only).
// update() dan remove() diblokir.
// afterCreate() otomatis mengupdate products.stock.
class StockMovementManager : public BaseManager
{
public:
    enum ItemLogType { SALE, RETURN_RESTOCK, RETURN };
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

    // NO-OP jika tipe == RETURN
    // untuk investigasi kegagalan cetak bisa lihat order_id yang di cancel
    // dan melihat daftar order_item yang ada disana
    // TODO : Mungkin nanti kita harus memiliki table waste sendiri untuk kebutuhan pelacakan kegagalan cetak
    bool orderItemLog(int orderItemId, ItemLogType type, int adminId= -1);
    bool orderItemSold(int orderItemId, int adminId= -1) { return orderItemLog(orderItemId, SALE, adminId); };    
    bool orderItemCancelled(int orderItemId, int adminId= -1) { return orderItemLog(orderItemId, RETURN_RESTOCK, adminId); };
protected:
    bool beforeCreate(QVariantMap& params) override;
};
