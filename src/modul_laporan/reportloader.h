#pragma once

#include <QDate>
#include <QString>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QDebug>

#include "report_data.h"

// ============================================================
//  ReportLoader
//  Mengisi DailySalesReport dan DailyExpenseReport dari DB.
//
//  Semua query memfilter tanggal dalam WIB (UTC+7):
//    DATE(datetime(col, '+7 hours')) = :date
//
//  Contoh:
//    ReportLoader loader;
//    auto sales   = loader.loadDailySales(QDate(2026,4,23));
//    auto expense = loader.loadDailyExpense(QDate(2026,4,23));
//
//    if (loader.hasError())
//        qWarning() << loader.errorString();
// ============================================================

class ReportLoader
{
public:
    ReportLoader(QSqlDatabase &db) : m_db(db) {}

    // ---- entry points ----

    DailySalesReport   loadDailySales  (const QDate& date);
    DailyExpenseReport loadDailyExpense(const QDate& date);

    bool    hasError()    const { return !m_error.isEmpty(); }
    QString errorString() const { return m_error; }

private:
    QSqlDatabase &m_db;
    QString m_error;

    // ---- shared ----
    CompanyInfo loadCompanyInfo();
    ReportMeta  buildMeta(const QDate& date, const QString& prefix);

    // ---- sales helpers ----
    SalesSummary            loadSalesSummary   (const QString& dateStr);
    QList<OrderRow>         loadOrders         (const QString& dateStr);
    QList<PaymentMethodRow> loadPaymentMethods (const QString& dateStr);
    QList<TopProductRow>    loadTopProducts    (const QString& dateStr);

    // ---- expense helpers ----
    ExpenseSummary           loadExpenseSummary (const QString& dateStr);
    QList<ExpenseRow>        loadExpenses       (const QString& dateStr);
    QList<ExpenseCategoryRow> loadByCategory    (const QString& dateStr);
    QList<ExpenseAccountRow>  loadByAccount     (const QString& dateStr);

    // ---- utils ----
    static QString wibFilter() {
        // kolom waktu disimpan UTC, konversi ke WIB (+7) saat filter
        return "DATE(datetime(%1, '+7 hours')) = :date";
    }

    bool exec(QSqlQuery& q, const QString& label) {
        if (!q.exec()) {
            m_error = QString("[%1] %2").arg(label, q.lastError().text());
            qWarning() << m_error;
            return false;
        }
        return true;
    }

    static qint64 toInt(const QVariant& v) { return v.toLongLong(); }
};
