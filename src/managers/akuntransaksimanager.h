#pragma once

// ============================================================================
// AkunTransaksiManager — tabel: akun_transaksi
// ============================================================================

#include "basemanager.h"

class AkunTransaksiManager : public BaseManager
{
public:
    explicit AkunTransaksiManager()
        : BaseManager("akun_transaksi") {}

    // Mengambil semua akun yang masih aktif
    QList<QSqlRecord> getActive();
    
    // Mengambil akun berdasarkan kode unik (misal: 'CASH', 'BCA')
    std::optional<QSqlRecord> findByKode(const QString& kode);

    // Fungsi manual untuk update saldo (jika diperlukan di luar trigger)
    // newSaldo = Saldo saat ini, tidak melakukan operasi matematika
    bool updateSaldo(int id, qint64 newSaldo, int adminId);
};

