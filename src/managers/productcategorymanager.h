#pragma once
#include "basemanager.h"

class ProductCategoryManager : public BaseManager
{
public:
    explicit ProductCategoryManager();

    QList<QSqlRecord> getActive(const QString& orderBy = "category_name", int limit = -1);
    bool deactivate(int id);
};
