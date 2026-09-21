#include "reportdialog.h"
#include "reportloader.h"
#include "reportview.h"
#include "ui_reportdialog.h"
#include <QFileDialog>
#include <QGraphicsScene>
#include <QSqlDatabase>
#include <QPushButton>
#include <QRadioButton>
#include <QDate>
#include <QDateTime>
#include "report_data.h"
#include <algorithm>

ReportDialog::ReportDialog(QSqlDatabase &db, int adminId, QWidget *parent)
    : QDialog(parent), ui(new Ui::ReportDialog), loader(new ReportLoader(db, adminId)), m_reportMode(SalesMode)
{
    ui->setupUi(this);

    // Default UI state: daily mode
    ui->dailyWidget->show();
    ui->rangeWidget->hide();
    ui->dailyRadio->setChecked(true);
    ui->rangeRadio->setChecked(false);

    // Seed dates
    ui->dateEdit->setDate(QDate::currentDate());
    ui->fromDateEdit->setDate(QDate::currentDate());
    ui->toDateEdit->setDate(QDate::currentDate());

    // Clear scene when any date is touched
    auto clearScene = [this]{ ui->graphicsView->scene()->clear(); };
    connect(ui->dateEdit,     &QDateEdit::dateChanged, this, clearScene);
    connect(ui->fromDateEdit, &QDateEdit::dateChanged, this, clearScene);
    connect(ui->toDateEdit,   &QDateEdit::dateChanged, this, clearScene);

    // Radio toggles — when user switches between Harian / Periode,
    // remap the report kind (Sales<->RangeSales, Expense<->RangeExpense)
    // and re-render the current selection.
    connect(ui->dailyRadio, &QRadioButton::toggled, this, &ReportDialog::on_dailyRadio_toggled);
    connect(ui->rangeRadio, &QRadioButton::toggled, this, &ReportDialog::on_rangeRadio_toggled);
}

ReportDialog::~ReportDialog()
{
    delete ui;
    delete loader;
}

void ReportDialog::setMode(ReportMode rm)
{
    m_reportMode = rm;
    if (rm == RangeSalesMode || rm == RangeExpenseMode) {
        ui->dailyRadio->setChecked(false);
        ui->rangeRadio->setChecked(true);
    } else {
        ui->dailyRadio->setChecked(true);
        ui->rangeRadio->setChecked(false);
    }
    // Trigger a refresh matching the new mode
    if (ui->rangeRadio->isChecked())
        on_refreshButton_2_clicked();
    else
        on_refreshButton_clicked();
}

void ReportDialog::on_simpanButton_clicked()
{
    const QDateTime wibNow = QDateTime::currentDateTimeUtc().addSecs(7 * 3600);
    const QString kind = (m_reportMode == ExpenseMode || m_reportMode == RangeExpenseMode)
                             ? QStringLiteral("Belanja")
                             : QStringLiteral("Penjualan");
    const QString defaultName = QString("Laporan %1 %2.pdf")
                                    .arg(kind, wibNow.toString("yyyyMMdd_hhmmss"));
    auto file_path = QFileDialog::getSaveFileName(this, "Simpan Laporan", defaultName, "PDF (*.pdf)");
    if (file_path.isEmpty())
        return;
    ui->graphicsView->exportToPdf(file_path);
    accept();
}

void ReportDialog::on_refreshButton_clicked()
{
    // Daily refresh — interpret the daily dateEdit as the period to show.
    const QDate selectedDate = ui->dateEdit->date();
    if (m_reportMode == SalesMode) {
        ui->graphicsView->showSalesReport(loader->loadDailySales(selectedDate));
    } else if (m_reportMode == ExpenseMode) {
        ui->graphicsView->showExpenseReport(loader->loadDailyExpense(selectedDate));
    } else {
        // Coming from range view but dateEdit was changed — degrade to
        // single-day range of the matching kind.
        DateRange r{ selectedDate, selectedDate };
        if (m_reportMode == RangeSalesMode)
            ui->graphicsView->showRangeSalesReport(loader->loadRangeSales(r));
        else
            ui->graphicsView->showRangeExpenseReport(loader->loadRangeExpense(r));
    }
}

void ReportDialog::on_refreshButton_2_clicked()
{
    // Range refresh — read fromDateEdit / toDateEdit.
    QDate from = ui->fromDateEdit->date();
    QDate to   = ui->toDateEdit->date();
    if (to < from) std::swap(from, to);

    DateRange range{ from, to };
    if (m_reportMode == RangeSalesMode) {
        ui->graphicsView->showRangeSalesReport(loader->loadRangeSales(range));
    } else if (m_reportMode == RangeExpenseMode) {
        ui->graphicsView->showRangeExpenseReport(loader->loadRangeExpense(range));
    } else {
        // Coming from daily view but range dates were changed —
        // promote to the range variant of the current kind.
        if (m_reportMode == SalesMode)
            ui->graphicsView->showRangeSalesReport(loader->loadRangeSales(range));
        else
            ui->graphicsView->showRangeExpenseReport(loader->loadRangeExpense(range));
    }
}

void ReportDialog::on_dailyRadio_toggled(bool checked)
{
    if (!checked) return;
    // Demote range mode to its daily counterpart, keeping sales vs. expense.
    if (m_reportMode == RangeSalesMode)   m_reportMode = SalesMode;
    if (m_reportMode == RangeExpenseMode) m_reportMode = ExpenseMode;

    ui->dailyWidget->show();
    ui->rangeWidget->hide();
    on_refreshButton_clicked();
}

void ReportDialog::on_rangeRadio_toggled(bool checked)
{
    if (!checked) return;
    // Promote daily mode to its range counterpart.
    if (m_reportMode == SalesMode)   m_reportMode = RangeSalesMode;
    if (m_reportMode == ExpenseMode) m_reportMode = RangeExpenseMode;

    ui->dailyWidget->hide();
    ui->rangeWidget->show();
    on_refreshButton_2_clicked();
}
