// contoh penggunaan — main.cpp
// Tambahkan ke .pro: QT += widgets printsupport

#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTimeZone>

#include "reportview.h"

// ---- helper: buat data dummy laporan penjualan ----
DailySalesReport makeSampleSalesReport()
{
    DailySalesReport r;

    r.company = { "Percetakan Maju Jaya",
                  "Jl. Contoh No. 123, Tasikmalaya",
                  "(0265) 000-0000",
                  "info@majujaya.com" };

    r.meta = {
        "RPT-H-20260423-001",
        QDateTime(QDate(2026, 4, 23), QTime(10, 0), QTimeZone::UTC),
        "Nur Holis (Super Admin)",
        QDate(2026, 4, 23)
    };

    r.summary = { 12, 4850000, 3200000, 1650000 };

    r.orders = {
        { "ORD-0423-001", "Budi Santoso",  "Banner Flexy 2×1 m",
          "completed", "paid",    350000 },
        { "ORD-0423-002", "CV Karya Muda", "AP150 – 50 lbr",
          "completed", "paid",    750000 },
        { "ORD-0423-003", "Rina Marlina",  "Sticker Vinyl – 30 lbr",
          "processing", "unpaid", 420000 },
        { "ORD-0423-004", "Toko Harapan",  "Cetak Offset SORM FC",
          "processing", "unpaid", 1200000 },
        { "ORD-0423-005", "Guest",          "Laminasi Doff A3+ – 10",
          "completed", "paid",    180000 },
    };

    r.paymentMethods = {
        { "Kas (tunai)",   5, 1850000 },
        { "Transfer bank", 3, 1350000 },
        { "QRIS",          1,       0 },
    };

    r.topProducts = {
        { "AP150",         "A3Plus",       "120 lbr", 900000 },
        { "Banner Flexy",  "Large Format", "8,5 m²",  765000 },
        { "Sticker Vinyl", "A3Plus",       "60 lbr",  510000 },
    };

    r.notes = "Laporan ini mencakup semua order selain 'cancelled'. "
              "Waktu menggunakan zona WIB (UTC+7).";
    return r;
}

// ---- helper: buat data dummy laporan belanja ----
DailyExpenseReport makeSampleExpenseReport()
{
    DailyExpenseReport r;

    r.company = { "Percetakan Maju Jaya",
                  "Jl. Contoh No. 123, Tasikmalaya",
                  "(0265) 000-0000",
                  "info@majujaya.com" };

    r.meta = {
        "RPT-B-20260423-001",
        QDateTime(QDate(2026, 4, 23), QTime(10, 0), QTimeZone::UTC),
        "Nur Holis (Super Admin)",
        QDate(2026, 4, 23)
    };

    r.summary = { 2175000, 1550000, 625000 };

    r.expenses = {
        { "TRX-0423-001", "Beli bahan Flexy 50m",    "Beli Bahan Baku",
          "material", "Kas Admin",    "Maman",    750000 },
        { "TRX-0423-002", "Beli kertas AP150 1 Rim","Beli Bahan Baku",
          "material", "Kas Admin",    "Maman",    500000 },
        { "TRX-0423-003", "Beli tinta printer",       "Beli Bahan Baku",
          "material", "Transfer BCA", "Nur Holis",300000 },
        { "TRX-0423-004", "Tagihan listrik April",    "Biaya Listrik",
          "ops",      "Transfer BCA", "Nur Holis",425000 },
        { "TRX-0423-005", "Beli alat kebersihan",     "Lain-lain",
          "other",    "Kas Admin",    "Syahid",   200000 },
    };

    r.byCategory = {
        { "Beli bahan baku", 3, 1550000, 71.3 },
        { "Biaya listrik",   1,  425000, 19.5 },
        { "Lain-lain",       1,  200000,  9.2 },
    };

    r.byAccount = {
        { "Kas Admin (tunai)", 3, 1450000 },
        { "Transfer BCA",      2,  725000 },
    };

    r.notes = "Hanya mencakup transaksi bertipe 'pengeluaran' hari ini. "
              "Waktu menggunakan zona WIB (UTC+7).";
    return r;
}

// ---- main window ----
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QMainWindow win;
    win.setWindowTitle("POS Laporan Viewer");
    win.resize(900, 700);

    auto* central = new QWidget;
    auto* vlay    = new QVBoxLayout(central);
    vlay->setContentsMargins(0, 0, 0, 0);
    vlay->setSpacing(0);

    // toolbar
    auto* toolbar = new QWidget;
    toolbar->setFixedHeight(40);
    auto* hlay = new QHBoxLayout(toolbar);
    hlay->setContentsMargins(8, 4, 8, 4);

    auto* btnSales   = new QPushButton("Laporan Penjualan");
    auto* btnExpense = new QPushButton("Laporan Belanja");
    auto* btnZoomIn  = new QPushButton("+");
    auto* btnZoomOut = new QPushButton("–");
    auto* btnReset   = new QPushButton("Reset zoom");
    auto* btnPdf     = new QPushButton("Ekspor PDF");

    hlay->addWidget(btnSales);
    hlay->addWidget(btnExpense);
    hlay->addStretch();
    hlay->addWidget(btnZoomOut);
    hlay->addWidget(btnZoomIn);
    hlay->addWidget(btnReset);
    hlay->addWidget(btnPdf);

    auto* reportView = new ReportView;
    reportView->showSalesReport(makeSampleSalesReport());

    vlay->addWidget(toolbar);
    vlay->addWidget(reportView, 1);
    win.setCentralWidget(central);

    QObject::connect(btnSales,   &QPushButton::clicked, [&]{
        reportView->showSalesReport(makeSampleSalesReport());
    });
    QObject::connect(btnExpense, &QPushButton::clicked, [&]{
        reportView->showExpenseReport(makeSampleExpenseReport());
    });
    QObject::connect(btnZoomIn,  &QPushButton::clicked,
                     reportView, &ReportView::zoomIn);
    QObject::connect(btnZoomOut, &QPushButton::clicked,
                     reportView, &ReportView::zoomOut);
    QObject::connect(btnReset,   &QPushButton::clicked,
                     reportView, &ReportView::resetZoom);
    QObject::connect(btnPdf,     &QPushButton::clicked, [&]{
        reportView->exportToPdf("laporan.pdf");
    });

    win.show();
    return app.exec();
}
