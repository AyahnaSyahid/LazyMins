#pragma once

#include <QString>

class ItemFlowService {
public:
    ItemFlowService() = default;
    bool handleItemSold(int orderItemId); 
    bool handleItemCanceled(int orderItemId, bool restock = true);
    
    inline const QString &errorString() const { return m_errorString; }

private:
    QString m_errorString;
    void resetError() { m_errorString = ""; }
};