#pragma once

#include "basemanager.h"

// ============================================================================
// PriceLevelManager — tabel: price_levels
// ============================================================================
class PriceLevelManager : public BaseManager
{
public:
    explicit PriceLevelManager()
        : BaseManager("price_levels") {}

    std::optional<QSqlRecord> findByName(const QString& levelName);
};
