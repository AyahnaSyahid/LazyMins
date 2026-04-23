#pragma once

#include <QString>
#include <QDateTime>
#include <QList>

// ============================================================
//  SHARED
// ============================================================

struct CompanyInfo {
    QString name;
    QString address;
    QString phone;
    QString email;
};

struct ReportMeta {
    QString documentNumber;   // e.g. RPT-H-20260423-001
    QDateTime printedAt;      // waktu cetak (UTC), render dalam WIB
    QString printedBy;        // nama + role admin
    QDate   periodDate;       // tanggal laporan (WIB)
};

// ============================================================
//  LAPORAN PENJUALAN HARIAN
// ============================================================

struct SalesSummary {
    int     totalOrders       = 0;
    qint64  totalRevenue      = 0;   // Rp
    qint64  paidAmount        = 0;   // Rp
    qint64  unpaidAmount      = 0;   // Rp
};

struct OrderRow {
    QString orderNumber;
    QString customerName;
    QString productSummary;   // deskripsi singkat item utama
    QString productionStatus; // pending | processing | ready | completed
    QString paymentStatus;    // paid | partial | unpaid
    qint64  totalAmount = 0;
};

struct PaymentMethodRow {
    QString methodName;       // Kas, Transfer, QRIS, dll
    int     txCount    = 0;
    qint64  amount     = 0;
};

struct TopProductRow {
    QString productName;
    QString categoryName;
    QString qtySold;          // "120 lbr" atau "8.5 m²" — sudah diformat
    qint64  revenue = 0;
};

struct DailySalesReport {
    CompanyInfo         company;
    ReportMeta          meta;
    SalesSummary        summary;
    QList<OrderRow>     orders;
    QList<PaymentMethodRow> paymentMethods;
    QList<TopProductRow>    topProducts;
    QString             notes;
};

// ============================================================
//  LAPORAN BELANJA HARIAN
// ============================================================

struct ExpenseSummary {
    qint64 totalExpense    = 0;
    qint64 materialExpense = 0;   // bahan baku
    qint64 opsExpense      = 0;   // operasional
};

struct ExpenseRow {
    QString txNumber;
    QString description;
    QString categoryName;
    QString categoryType;  // "material" | "ops" | "other" — untuk warna badge
    QString accountName;   // akun_transaksi
    QString recordedBy;    // nama admin
    qint64  amount = 0;
};

struct ExpenseCategoryRow {
    QString categoryName;
    int     txCount  = 0;
    qint64  amount   = 0;
    double  percent  = 0.0;  // 0–100
};

struct ExpenseAccountRow {
    QString accountName;
    int     txCount = 0;
    qint64  amount  = 0;
};

struct DailyExpenseReport {
    CompanyInfo              company;
    ReportMeta               meta;
    ExpenseSummary           summary;
    QList<ExpenseRow>        expenses;
    QList<ExpenseCategoryRow> byCategory;
    QList<ExpenseAccountRow>  byAccount;
    QString                  notes;
};
