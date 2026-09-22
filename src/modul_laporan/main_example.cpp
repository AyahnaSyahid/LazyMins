// contoh penggunaan — main.cpp
// Tambahkan ke .pro: QT += widgets printsupport

#include <QApplication>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSqlDatabase>
#include <QTimeZone>
#include <QVBoxLayout>

#include "reportview.h"
#include "reportloader.h"

// ---- main window ----
int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  app.setOrganizationName("BlackCircle");
  app.setApplicationName("LazyMins");

  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings s;
  QString databasePath = s.value("Database/databasePath", "").toString();
  if (databasePath.isEmpty())
  {
    QMessageBox::warning(nullptr, "Error", "Database path not found");
    app.quit();
    return 1;
  }

  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "DF");
  db.setDatabaseName(databasePath);
  if (!db.open())
  {
    QMessageBox::critical(nullptr, "Error", db.lastError().text());
    app.quit();
    return 1;
  }

  ReportLoader rl(db, 1);

  QMainWindow win;
  win.setWindowTitle("POS Laporan Viewer");
  win.resize(900, 700);

  auto *central = new QWidget;
  auto *vlay = new QVBoxLayout(central);
  vlay->setContentsMargins(0, 0, 0, 0);
  vlay->setSpacing(0);

  // toolbar
  auto *toolbar = new QWidget;
  toolbar->setFixedHeight(40);
  auto *hlay = new QHBoxLayout(toolbar);
  hlay->setContentsMargins(8, 4, 8, 4);

  auto *btnSales = new QPushButton("Laporan Penjualan");
  auto *btnExpense = new QPushButton("Laporan Belanja");
  auto *btnZoomIn = new QPushButton("+");
  auto *btnZoomOut = new QPushButton("–");
  auto *btnReset = new QPushButton("Reset zoom");
  auto *btnPdf = new QPushButton("Ekspor PDF");

  hlay->addWidget(btnSales);
  hlay->addWidget(btnExpense);
  hlay->addStretch();
  hlay->addWidget(btnZoomOut);
  hlay->addWidget(btnZoomIn);
  hlay->addWidget(btnReset);
  hlay->addWidget(btnPdf);

  auto *reportView = new ReportView;
  reportView->showSalesReport( [&rl](){ return rl.loadDailySales(QDate(2026, 7, 23)); }());

  vlay->addWidget(toolbar);
  vlay->addWidget(reportView, 1);
  win.setCentralWidget(central);

  QObject::connect(btnSales, &QPushButton::clicked, [&]
                   { reportView->showSalesReport([&rl](){ return rl.loadDailySales(QDate(2026, 7, 23)); }()); });
  QObject::connect(btnExpense, &QPushButton::clicked, [&]
                   { reportView->showExpenseReport([&rl](){ return rl.loadDailyExpense(QDate(2026, 7, 23)); }()); });
  QObject::connect(btnZoomIn, &QPushButton::clicked,
                   reportView, &ReportView::zoomIn);
  QObject::connect(btnZoomOut, &QPushButton::clicked,
                   reportView, &ReportView::zoomOut);
  QObject::connect(btnReset, &QPushButton::clicked,
                   reportView, &ReportView::resetZoom);
  QObject::connect(btnPdf, &QPushButton::clicked, [&]
                   { reportView->exportToPdf("laporan.pdf"); });

  win.show();
  return app.exec();
}
