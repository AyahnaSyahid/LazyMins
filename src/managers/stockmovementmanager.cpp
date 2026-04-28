#include "stockmovementmanager.h"
#include "orderitemmanager.h"
#include "productmanager.h"
#include "src/utils/sessionmanager.h"

StockMovementManager::StockMovementManager()
    : BaseManager("stock_movements", false)
{
}

// DIBLOKIR
bool StockMovementManager::update(int id, const QVariantMap &params)
{
    Q_UNUSED(id)
    Q_UNUSED(params)
    setErrorString("Stock movement bersifat immutable dan tidak dapat diubah.");
    return false;
}

// DIBLOKIR
bool StockMovementManager::remove(int id)
{
    Q_UNUSED(id)
    setErrorString("Stock movement bersifat immutable dan tidak dapat dihapus.");
    return false;
}

QList<QSqlRecord> StockMovementManager::getByProduct(int productId, const QString &orderBy)
{
    return getWhere("product_id = :product_id",
                    {{"product_id", productId}},
                    orderBy);
}

QList<QSqlRecord> StockMovementManager::getByReference(const QString &referenceType, int referenceId)
{
    return getWhere(
        "reference_type = :ref_type AND reference_id = :ref_id",
        {{"ref_type", referenceType}, {"ref_id", referenceId}},
        "movement_date DESC");
}

QList<QSqlRecord> StockMovementManager::getByDateRange(const QDate &from, const QDate &to, int productId)
{
    QString condition = "movement_date >= :from AND movement_date <= :to";
    QVariantMap bindings = {
        {"from", from.toString(Qt::ISODate)},
        {"to", to.toString(Qt::ISODate)}};
    if (productId > 0)
    {
        condition += " AND product_id = :product_id";
        bindings["product_id"] = productId;
    }
    return getWhere(condition, bindings, "movement_date DESC");
}

std::optional<QSqlRecord> StockMovementManager::recordMovement(int productId,
                                                               const QString &movementType,
                                                               double quantity,
                                                               double stockBefore,
                                                               int adminId,
                                                               const QString &referenceType,
                                                               int referenceId,
                                                               const QString &notes)
{
    QVariantMap params;
    params["product_id"] = productId;
    params["movement_type"] = movementType;
    params["quantity"] = quantity;
    params["stock_before"] = stockBefore;
    params["stock_after"] = qCeil((stockBefore + quantity) * 100.0) / 100.0;
    params["admin_id"] = adminId;
    params["reference_type"] = referenceType.isEmpty() ? QVariant() : QVariant(referenceType);
    params["reference_id"] = referenceId > 0 ? QVariant(referenceId) : QVariant();
    params["notes"] = notes.isEmpty() ? QVariant() : QVariant(notes);

    // beforeCreate dan afterCreate (update products.stock) sudah diblokir
    // untuk tidak override stock_before/after yang kita set manual di sini.
    // Karena itu kita bypass beforeCreate dengan set stock_before/after eksplisit.
    return create(params);
}

bool StockMovementManager::orderItemLog(int orderItemId, ItemLogType type, int adminId)
{
    if (type == RETURN)
        return true;

    OrderItemManager oim;
    auto oimRecord = oim.getById(orderItemId);
    if (!oimRecord)
    {
        setErrorString("Order item tidak ditemukan");
        return false;
    }

    // 1. Hitung Quantity (Mendukung Produk Area)
    double qty = oimRecord->value("quantity").toDouble();
    if (oimRecord->value("use_area").toBool())
    {
        double width = oimRecord->value("size_width").toDouble();
        double height = oimRecord->value("size_height").toDouble();
        qty = qCeil((width * height * qty) * 100.0) / 100.0;
    }

    // 2. Tentukan arah stok berdasarkan tipe
    double delta = (type == ItemLogType::SALE) ? -qty : qty;

    // 3. Update Stock Table
    auto q = baseQuery();
    q.prepare(R"-(
        UPDATE products
        SET stock = stock + :delta,
            updated_at = :now
        WHERE id = :id AND
            EXISTS (
                SELECT 1
                    FROM orders
                    WHERE orders.id = :order_id AND
                        orders.staging_status <> 'cancelled' ); )-");
    q.bindValue(":delta", delta);
    q.bindValue(":now", dateTimeToSql());
    q.bindValue(":id", oimRecord->value("product_id").toInt());
    q.bindValue(":order_id", oimRecord->value("order_id").toInt());
    if (!qexec(q))
        return false;
    if (q.numRowsAffected() == 0)
        return true;

    // 4. Ambil data produk terbaru untuk Logging
    ProductManager pm;
    auto opt_pr = pm.getById(oimRecord->value("product_id").toInt());
    if (!opt_pr)
        return false;

    double currentStock = opt_pr->value("stock").toDouble();

    // 5. Catat Movement
    QVariantMap params;
    params["product_id"] = oimRecord->value("product_id").toInt();
    params["movement_type"] = (delta < 0) ? "out" : "in";
    params["quantity"] = delta;
    params["stock_after"] = currentStock;
    params["stock_before"] = currentStock - delta; // Pasti benar baik untuk in maupun out
    params["admin_id"] = adminId > 0 ? adminId : SessionManager::instance().currentUserId();
    params["reference_id"] = orderItemId;

    if (type == SALE)
    {
        params["notes"] = "[SYSTEM] Penjualan Item";
        params["reference_type"] = "order_item";
    }
    else if (type == RETURN_RESTOCK)
    {
        params["notes"] = "[SYSTEM] Pembatalan (Restock)";
        params["reference_type"] = "[Dibatalkan] order_item";
    }

    return create(params).has_value();
}

bool StockMovementManager::beforeCreate(QVariantMap &params)
{
    if (!params.contains("admin_id") || params["admin_id"].toInt() <= 0)
    {
        params["admin_id"] = SessionManager::instance().currentUserId();
    }
    return true;
}
