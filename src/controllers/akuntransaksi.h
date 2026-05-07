#pragma once

#include <QVariantMap>

class AkunTransaksiController
{
    public:
        AkunTransaksiController() = default;
        ~AkunTransaksiController() = default;
    
        bool createAccount(QVariantMap &params, QString *error);
        bool updateAccount(int accId, QVariantMap &params, QString *error);
};