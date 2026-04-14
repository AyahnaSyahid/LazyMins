#pragma once
#include "basemanager.h"

class AkunTransaksiManager : public BaseManager
{
public:
    explicit AkunTransaksiManager();

    std::optional<QSqlRecord> getByKode(const QString& kode) const;
    QList<QSqlRecord> getActive(const QString& orderBy = "nama", int limit = -1);
    bool deactivate(int id);

    // Dipanggil oleh PaymentManager::afterCreate
    // Saldo hanya boleh diubah melalui method ini, bukan update() langsung
    bool updateSaldo(int id, int newSaldo);
};
