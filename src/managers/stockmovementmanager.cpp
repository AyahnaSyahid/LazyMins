#include "stockmovementmanager.h"
#include "productmanager.h"

StockMovementManager::StockMovementManager()
    : BaseManager("stock_movements", false)
{
}

bool StockMovementManager::beforeCreate(QVariantMap& params)
{
    int productId = params.value("product_id").toInt();
    if (productId <= 0) {
        setErrorString("product_id wajib diisi");
        return false;
    }

    // Hanya hitung otomatis jika stock_before/after belum diisi oleh caller
    if (!params.contains("stock_before") || !params.contains("stock_after")) {
        ProductManager pm;
        auto productRecord = pm.getById(productId);
        if (!productRecord) {
            setErrorString("Product tidak ditemukan");
            return false;
        }
        double stockBefore = productRecord->value("stock").toDouble();
        double quantity    = params.value("quantity").toDouble();
        params["stock_before"] = stockBefore;
        params["stock_after"]  = stockBefore + quantity;
    }

    if (!params.contains("movement_date"))
        params["movement_date"] = dateTimeToSql();

    return true;
}

bool StockMovementManager::afterCreate(const QSqlRecord& record)
{
    int    productId  = record.value("product_id").toInt();
    double stockAfter = record.value("stock_after").toDouble();

    ProductManager pm;
    if (!pm.update(productId, {{ "stock", stockAfter }})) {
        setErrorString("Gagal update stok produk: " + pm.errorString());
        return false;
    }
    return true;
}

// DIBLOKIR
bool StockMovementManager::update(int id, const QVariantMap& params)
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

QList<QSqlRecord> StockMovementManager::getByProduct(int productId, const QString& orderBy)
{
    return getWhere("product_id = :product_id",
                    {{ ":product_id", productId }},
                    orderBy);
}

QList<QSqlRecord> StockMovementManager::getByReference(const QString& referenceType, int referenceId)
{
    return getWhere(
        "reference_type = :ref_type AND reference_id = :ref_id",
        {{ ":ref_type", referenceType }, { ":ref_id", referenceId }},
        "movement_date DESC"
    );
}

QList<QSqlRecord> StockMovementManager::getByDateRange(const QDate& from, const QDate& to, int productId)
{
    QString condition = "movement_date >= :from AND movement_date <= :to";
    QVariantMap bindings = {
        { ":from", from.toString(Qt::ISODate) },
        { ":to",   to.toString(Qt::ISODate) }
    };
    if (productId > 0) {
        condition += " AND product_id = :product_id";
        bindings[":product_id"] = productId;
    }
    return getWhere(condition, bindings, "movement_date DESC");
}

std::optional<QSqlRecord> StockMovementManager::recordMovement(int productId,
                                                                 const QString& movementType,
                                                                 double quantity,
                                                                 double stockBefore,
                                                                 int adminId,
                                                                 const QString& referenceType,
                                                                 int referenceId,
                                                                 const QString& notes)
{
    QVariantMap params;
    params["product_id"]     = productId;
    params["movement_type"]  = movementType;
    params["quantity"]       = quantity;
    params["stock_before"]   = stockBefore;
    params["stock_after"]    = qCeil((stockBefore + quantity) * 100.0) / 100.0;
    params["admin_id"]       = adminId;
    params["reference_type"] = referenceType.isEmpty() ? QVariant() : QVariant(referenceType);
    params["reference_id"]   = referenceId > 0         ? QVariant(referenceId) : QVariant();
    params["notes"]          = notes.isEmpty()         ? QVariant() : QVariant(notes);

    // beforeCreate dan afterCreate (update products.stock) sudah diblokir
    // untuk tidak override stock_before/after yang kita set manual di sini.
    // Karena itu kita bypass beforeCreate dengan set stock_before/after eksplisit.
    return create(params);
}