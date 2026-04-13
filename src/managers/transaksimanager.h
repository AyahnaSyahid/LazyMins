#pragma once

// ============================================================================
// TransaksiManager — tabel: transaksi
// ============================================================================
class TransaksiManager : public BaseManager
{
public:
    explicit TransaksiManager()
        : BaseManager("transaksi") {}

    QList<QSqlRecord> getByTipe(const QString& tipe);
    QList<QSqlRecord> getByAdmin(int adminId);
    QList<QSqlRecord> getByDateRange(const QDate& from, const QDate& to);
    QList<QSqlRecord> getByKategori(int kategoriId);
    QList<QSqlRecord> getByReference(const QString& referenceType, int referenceId);
    std::optional<QSqlRecord> lastTransaction() const ;

    // Aggregates
    qint64 sumByTipe(const QString& tipe, const QDate& from = QDate(), const QDate& to = QDate());
    
    static QString generateTransactionNumber(const QString& prefix = "TRX");
    
protected:
    bool beforeCreate(QVariantMap& params) override;
    bool afterCreate(const QSqlRecord& rc) override;
};