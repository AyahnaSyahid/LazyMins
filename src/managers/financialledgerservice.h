#pragma once

#include <QString>

class FinancialLedgerService
{
public:
    enum KategoriTransaksi { PembayaranInvoice = 4, PembatalanInvoice };
    FinancialLedgerService() = default;
    
    inline const QString &errorString() const { return m_errorString; }

    // WARN: Not thread-safe. Caller is responsible for:
    // - Wrapping in a database transaction (commit/rollback)
    // - Ensuring no concurrent calls with the same paymentId
    bool handlePayment(int paymentId);

private:
    void resetError() { m_errorString = ""; }
    QString m_errorString;
};