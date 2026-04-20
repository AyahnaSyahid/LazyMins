#pragma once

#include <QSqlDatabase>

#include "src/managers/basemanager.h"

class SqlTransaction {
public:
    // Constructor memulai transaksi
    explicit SqlTransaction(QSqlDatabase &db = BaseManager::connection);
    
    // Destructor melakukan rollback jika commit() belum dipanggil
    ~SqlTransaction();

    // Menandai transaksi selesai dan berhasil
    bool commit();
    const bool started() const { return m_started; }
    // Menghapus copy constructor dan assignment operator
    SqlTransaction(const SqlTransaction&) = delete;
    SqlTransaction& operator=(const SqlTransaction&) = delete;

private:
    QSqlDatabase &m_db;
    bool m_committed;
    bool m_started;
};