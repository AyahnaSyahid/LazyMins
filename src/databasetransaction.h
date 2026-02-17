#pragma once

/**
 * DatabaseTransaction.h
 * RAII-based Transaction Manager for Qt6 / QtSql
 *
 * Pola: transaksi dimulai saat objek dibuat,
 *        commit() harus dipanggil secara eksplisit,
 *        destructor otomatis rollback jika commit belum dipanggil
 *        (termasuk saat exception atau early-return).
 */

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QDebug>
#include <stdexcept>
#include <functional>

// ─────────────────────────────────────────────────────────────────────────────
// 1. Exception khusus untuk kegagalan database
// ─────────────────────────────────────────────────────────────────────────────
class DatabaseException : public std::runtime_error {
public:
    explicit DatabaseException(const QString& msg)
        : std::runtime_error(msg.toStdString())
        , m_message(msg) {}

    QString qwhat() const noexcept { return m_message; }

private:
    QString m_message;
};

// ─────────────────────────────────────────────────────────────────────────────
// 2. Core RAII Transaction Guard
// ─────────────────────────────────────────────────────────────────────────────
class DatabaseTransaction {
    Q_DISABLE_COPY(DatabaseTransaction)   // mencegah copy yang tidak disengaja

public:
    /**
     * @brief Konstruktor — langsung memulai transaksi.
     * @param db        Koneksi database Qt yang sudah terbuka.
     * @param savepointName  Kosongkan untuk transaksi top-level;
     *                       isi untuk SAVEPOINT (nested transaction).
     * @throws DatabaseException jika database tidak terbuka
     *         atau gagal memulai transaksi.
     */
    explicit DatabaseTransaction(QSqlDatabase& db,
                                  const QString& savepointName = {})
        : m_db(db)
        , m_savepointName(savepointName)
        , m_committed(false)
    {
        if (!m_db.isOpen()) {
            throw DatabaseException(
                QStringLiteral("DatabaseTransaction: koneksi tidak terbuka — '%1'")
                    .arg(m_db.connectionName()));
        }

        bool ok = m_savepointName.isEmpty()
                  ? m_db.transaction()
                  : executeSavepoint(QStringLiteral("SAVEPOINT %1").arg(m_savepointName));

        if (!ok) {
            throw DatabaseException(
                QStringLiteral("Gagal memulai transaksi: %1")
                    .arg(m_db.lastError().text()));
        }

        qDebug().noquote()
            << (m_savepointName.isEmpty()
                    ? QStringLiteral("[TX] Transaksi dimulai")
                    : QStringLiteral("[TX] Savepoint '%1' dibuat").arg(m_savepointName));
    }

    /**
     * @brief Destructor — rollback otomatis jika commit() belum dipanggil.
     *        Tidak melempar exception (noexcept) sesuai best-practice C++.
     */
    ~DatabaseTransaction() noexcept {
        if (!m_committed) {
            performRollback();
        }
    }

    // Move constructor — berguna saat dikembalikan dari factory function
    DatabaseTransaction(DatabaseTransaction&& other) noexcept
        : m_db(other.m_db)
        , m_savepointName(std::move(other.m_savepointName))
        , m_committed(other.m_committed)
    {
        other.m_committed = true; // sumber tidak boleh rollback lagi
    }

    /**
     * @brief Commit transaksi secara eksplisit.
     * @throws DatabaseException jika commit gagal.
     */
    void commit() {
        if (m_committed) {
            qWarning() << "[TX] commit() dipanggil lebih dari sekali — diabaikan";
            return;
        }

        bool ok = m_savepointName.isEmpty()
                  ? m_db.commit()
                  : executeSavepoint(
                        QStringLiteral("RELEASE SAVEPOINT %1").arg(m_savepointName));

        if (!ok) {
            // Coba rollback sebelum melempar exception
            performRollback();
            throw DatabaseException(
                QStringLiteral("Commit gagal: %1").arg(m_db.lastError().text()));
        }

        m_committed = true;
        qDebug().noquote()
            << (m_savepointName.isEmpty()
                    ? QStringLiteral("[TX] Commit berhasil")
                    : QStringLiteral("[TX] Savepoint '%1' di-release").arg(m_savepointName));
    }

    /**
     * @brief Rollback eksplisit (opsional — destructor sudah melakukannya).
     */
    void rollback() noexcept {
        if (!m_committed) {
            performRollback();
            m_committed = true; // cegah rollback ganda di destructor
        }
    }

    /** @return true jika transaksi sudah di-commit. */
    bool isCommitted() const noexcept { return m_committed; }

private:
    bool executeSavepoint(const QString& sql) noexcept {
        QSqlQuery q(m_db);
        return q.exec(sql);
    }

    void performRollback() noexcept {
        bool ok = m_savepointName.isEmpty()
                  ? m_db.rollback()
                  : executeSavepoint(
                        QStringLiteral("ROLLBACK TO SAVEPOINT %1").arg(m_savepointName));

        if (ok) {
            qDebug().noquote()
                << (m_savepointName.isEmpty()
                        ? QStringLiteral("[TX] Rollback berhasil")
                        : QStringLiteral("[TX] Rollback ke savepoint '%1'").arg(m_savepointName));
        } else {
            qCritical().noquote()
                << QStringLiteral("[TX] ROLLBACK GAGAL: %1").arg(m_db.lastError().text());
        }
    }

    QSqlDatabase& m_db;
    QString       m_savepointName;
    bool          m_committed;
};


// ─────────────────────────────────────────────────────────────────────────────
// 3. Helper: Scope-Guard berbasis lambda (bonus utility)
//    Gunakan untuk resource non-database yang perlu cleanup otomatis
// ─────────────────────────────────────────────────────────────────────────────
class ScopeGuard {
public:
    explicit ScopeGuard(std::function<void()> onExit)
        : m_onExit(std::move(onExit))
        , m_dismissed(false) {}

    ~ScopeGuard() noexcept {
        if (!m_dismissed && m_onExit) {
            try { m_onExit(); }
            catch (...) { /* noexcept: telan exception */ }
        }
    }

    void dismiss() noexcept { m_dismissed = true; }

    ScopeGuard(const ScopeGuard&)            = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

private:
    std::function<void()> m_onExit;
    bool m_dismissed;
};

using Transaction = DatabaseTransaction;

// Macro kenyamanan: SCOPE_EXIT { kode cleanup; };
#define SCOPE_EXIT ScopeGuard ANONYMOUS_GUARD_##__LINE__ = [&]()
