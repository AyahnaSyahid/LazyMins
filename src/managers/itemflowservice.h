#pragma once

#include <QString>

class ItemFlowService {
public:
    ItemFlowService() = default;
    bool processItemSold(int orderItemId); 
    bool processItemCancel(int orderItemId, bool restock = true);
    bool processOrderCancel(int orderId, bool restock = true);

    bool stockIn(int productId, qreal stockIn, const QString& supplier, const QString& notes);
    bool stockAdjust(int productId, qreal stockOut, const QString& supplier, const QString& notes);
    
    inline const QString &errorString() const { return m_errorString; }

private:
    QString m_errorString;
    void resetError() { m_errorString = ""; }
};