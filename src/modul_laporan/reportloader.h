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
//  Mengisi DailySalesReport, DailyExpenseReport,
//  RangeSalesReport, dan RangeExpenseReport dari DB.
//
//  Filter tanggal harian (WIB):
//    DATE(datetime(col, '+7 hours')) = :date
//
//  Filter tanggal range (WIB):
//    DATE(datetime(col, '+7 hours')) BETWEEN :from AND :to
//
//  Contoh:
//    ReportLoader loader(db);
//
//    // Harian
//    auto daily = loader.loadDailySales(QDate(2026,4,23));
//
//    // Periode
//    DateRange range{ QDate(2026,4,1), QDate(2026,4,30) };
//    auto range_report = loader.loadRangeSales(range);
//
//    if (loader.hasError())
//        qWarning() << loader.errorString();
// ============================================================

class ReportLoader
{
public:
    ReportLoader(QSqlDatabase &db, int adminId=0) : m_db(db), m_adminId(adminId) {}

    // ---- harian ----
    DailySalesReport   loadDailySales  (const QDate& date);
    DailyExpenseReport loadDailyExpense(const QDate& date);

    // ---- periode ----
    RangeSalesReport   loadRangeSales  (const DateRange& range);
    RangeExpenseReport loadRangeExpense(const DateRange& range);

    bool    hasError()    const { return !m_error.isEmpty(); }
    QString errorString() const { return m_error; }

private:
    QSqlDatabase &m_db;
    int m_adminId;
    QString m_error;

    // ---- shared ----
    CompanyInfo loadCompanyInfo();
    ReportMeta  buildMeta      (const QDate& date,  const QString& prefix);
    ReportMeta  buildRangeMeta (const DateRange& range, const QString& prefix);

    // ---- sales helpers (harian) ----
    SalesSummary            loadSalesSummary  (const QString& dateStr);
    QList<OrderRow>         loadOrders        (const QString& dateStr);
    QList<PaymentMethodRow> loadPaymentMethods(const QString& dateStr);
    QList<TopProductRow>    loadTopProducts   (const QString& dateStr);

    // ---- sales helpers (range) ----
    SalesSummary              loadRangeSalesSummary  (const DateRange& r);
    QList<OrderRow>           loadRangeTopOrders     (const DateRange& r, int limit, int& outOtherCount, qint64& outOtherTotal);
    QList<PaymentMethodRow>   loadRangePaymentMethods(const DateRange& r);
    QList<TopProductRow>      loadRangeTopProducts   (const DateRange& r);
    QList<DailySalesTrendRow> loadRangeTrend         (const DateRange& r);

    // ---- expense helpers (harian) ----
    ExpenseSummary            loadExpenseSummary(const QString& dateStr);
    QList<ExpenseRow>         loadExpenses      (const QString& dateStr);
    QList<ExpenseCategoryRow> loadByCategory    (const QString& dateStr);
    QList<ExpenseAccountRow>  loadByAccount     (const QString& dateStr);

    // ---- expense helpers (range) ----
    ExpenseSummary            loadRangeExpenseSummary(const DateRange& r);
    QList<ExpenseCategoryRow> loadRangeByCategory    (const DateRange& r);
    QList<ExpenseAccountRow>  loadRangeByAccount     (const DateRange& r);

    // ---- utils ----

    // Filter harian: DATE(datetime(col, '+7 hours')) = :date
    static QString wibFilter() {
        return "DATE(datetime(%1, '+7 hours')) = :date";
    }

    // Filter range: DATE(datetime(col, '+7 hours')) BETWEEN :from AND :to
    static QString wibRangeFilter() {
        return "DATE(datetime(%1, '+7 hours')) BETWEEN :from AND :to";
    }

    // Bind parameter range ke QSqlQuery
    static void bindRange(QSqlQuery& q, const DateRange& r) {
        q.bindValue(":from", r.from.toString("yyyy-MM-dd"));
        q.bindValue(":to",   r.to.toString("yyyy-MM-dd"));
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

    // Helper: tentukan categoryType dari nama kategori
    static QString categoryTypeFrom(const QString& katNama) {
        const QString lower = katNama.toLower();
        if (lower.contains("bahan") || lower.contains("stok") || lower.contains("material"))
            return "material";
        if (lower.contains("listrik") || lower.contains("gaji") ||
            lower.contains("sewa")    || lower.contains("operasional"))
            return "ops";
        return "other";
    }
};
