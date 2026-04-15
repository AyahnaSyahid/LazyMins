#include "sqltransaction.h"
#include <QSqlError>
#include <QDebug>

SqlTransaction::SqlTransaction(QSqlDatabase &db) 
    : m_db(db), 
      m_committed(false),
      m_started(false)
{
    if (m_db.isOpen()) {
        if (!m_db.transaction()) {
            qCritical() << "Gagal memulai transaksi:" << m_db.lastError().text();
        }
        m_started = true;
    }
}

SqlTransaction::~SqlTransaction() {
    if (!m_committed && m_db.isOpen()) {
        if (m_db.rollback()) {
            qDebug() << "RAII: Transaksi di-rollback otomatis.";
        } else {
            qCritical() << "RAII: Gagal melakukan rollback:" << m_db.lastError().text();
        }
    }
}

bool SqlTransaction::commit() {
    if (m_committed) {
        return true;
    }
    
    if (m_db.commit()) {
        m_committed = true;
        return true;
    }
    
    qCritical() << "Gagal melakukan commit:" << m_db.lastError().text();
    return false;
}