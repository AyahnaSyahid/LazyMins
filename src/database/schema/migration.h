#pragma once

namespace Migration {
    namespace Sqlite {
        constexpr const char* FK_OFF = "PRAGMA foreign_keys = OFF;";
        constexpr const char* FK_ON = "PRAGMA foreign_keys = ON;";
        constexpr const char* BEGIN_TRANSACTION = "BEGIN TRANSACTION;";
        constexpr const char* COMMIT = "COMMIT;";
        constexpr const char* ROLLBACK = "ROLLBACK;";
    }
} // namespace Migration

#include "sqlitev1.h"