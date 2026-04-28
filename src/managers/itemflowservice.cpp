#include "itemflowservice.h"
#include "src/managers/orderitemmanager.h"
#include "src/managers/productmanager.h"
#include "src/managers/stockmovementmanager.h"
#include "src/utils/sqltransaction.h"

bool ItemFlowService::processItemSold(int orderItemId)
{
    resetError();
    ProductManager pm;
    StockMovementManager sm;
    OrderItemManager oim;
    auto orderItem = oim.getById(orderItemId);
    if (!orderItem.has_value())
    {
        m_errorString = "Order item tidak ditemukan";
        return false;
    }
    auto product = pm.getById(orderItem->value("product_id").toInt());
    if (!product.has_value())
    {
        m_errorString = "Product tidak ditemukan";
        return false;
    }
    if (!sm.orderItemLog(orderItemId, sm.SALE))
        return false;
    return true;
}

bool ItemFlowService::processItemCancel(int orderItemId, bool restock)
{
    resetError();
    ProductManager pm;
    StockMovementManager sm;
    OrderItemManager oim;
    auto orderItem = oim.getById(orderItemId);
    if (!orderItem.has_value())
    {
        m_errorString = "Order item tidak ditemukan";
        return false;
    }
    auto product = pm.getById(orderItem->value("product_id").toInt());
    if (!product.has_value())
    {
        m_errorString = "Product tidak ditemukan";
        return false;
    }
    if (restock)
    {
        if (!sm.orderItemLog(orderItemId, sm.RETURN_RESTOCK))
            return false;
    }
    return true;
}

bool ItemFlowService::stockIn(int productId, qreal stockIn, const QString &supplier, const QString &notes)
{
    resetError();

    SqlTransaction tr;
    if (!tr.started())
    {
        m_errorString = "Gagal membuat transaksi Database :" +
                        BaseManager::connection.lastError().text();
        return false;
    }

    QSqlQuery query(BaseManager::connection);
    query.prepare("SELECT stock FROM products WHERE id = :id");
    query.bindValue(":id", productId);
    if (!query.exec() || !query.next())
    {
        m_errorString = "Product tidak ditemukan";
        return false;
    }

    double stockBefore = query.value(0).toDouble();
    ProductManager pm;
    if (!pm.adjustStock(productId, stockIn) ){
        m_errorString = pm.errorString();
        return false;
    }

    StockMovementManager sm;
    QString unotes = "Supplier : " + supplier;
    if (!notes.isEmpty())
         unotes += "\n" + notes;

    QVariantMap smParams {
        {"product_id", productId},
        {"movement_type", "in"},
        {"stock_before", stockBefore},
        {"quantity", stockIn},
        {"stock_after", qCeil((stockBefore + stockIn) * 100.0) / 100.0},
        {"notes", unotes }
    };

    if (!sm.create(smParams))
    {
        m_errorString = sm.errorString();
        return false;
    }
    return tr.commit();
}
