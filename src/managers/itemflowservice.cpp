#include "itemflowservice.h"
#include "src/managers/stockmovementmanager.h"
#include "src/managers/productmanager.h"
#include "src/managers/orderitemmanager.h"
#include "src/utils/sqltransaction.h"

bool ItemFlowService::handleItemSold(int orderItemId)
{
    resetError();
    ProductManager pm;
    StockMovementManager sm;
    OrderItemManager oim;
    auto orderItem = oim.getById(orderItemId);
    if (!orderItem.has_value()) {
        m_errorString = "Order item tidak ditemukan";
        return false;
    }
    auto product = pm.getById(orderItem->value("product_id").toInt());
    if (!product.has_value()) {
        m_errorString = "Product tidak ditemukan";
        return false;
    }
    if (!sm.orderItemLog(orderItemId, sm.SALE)) return false;
    return true;
}

bool ItemFlowService::handleItemCanceled(int orderItemId, bool restock)
{
    resetError();
    ProductManager pm;
    StockMovementManager sm;
    OrderItemManager oim;
    auto orderItem = oim.getById(orderItemId);
    if (!orderItem.has_value()) {
        m_errorString = "Order item tidak ditemukan";
        return false;
    }
    auto product = pm.getById(orderItem->value("product_id").toInt());
    if (!product.has_value()) {
        m_errorString = "Product tidak ditemukan";
        return false;
    }
    if (restock) {
        if (!sm.orderItemLog(orderItemId, sm.RETURN_RESTOCK)) return false;
    }
    return true;
}
