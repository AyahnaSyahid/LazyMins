#pragma once
#include "basemanager.h"

class PriceLevelManager : public BaseManager
{
public:
    explicit PriceLevelManager();

    std::optional<QSqlRecord> getByName(const QString& levelName) const;
};
