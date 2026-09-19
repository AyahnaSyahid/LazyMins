#include "reportdialog.h"
#include "reportloader.h"
#include "reportview.h"
#include "report_data.h"
#include "ui_reportdialog.h"
#include <QFileDialog>
#include <QGraphicsScene>
#include <QSqlDatabase>

#include <QDate>
#include <algorithm>

ReportDialog::ReportDialog(QSqlDatabase &db, int adminId, QWidget *parent)
    : QDialog(parent), ui(new Ui::ReportDialog), loader(new ReportLoader(db, adminId)), m_reportMode(SalesMode)
{
    ui->setupUi(this);
    ui->dateEdit->setDate(QDate::currentDate());
    ui->toDateEdit->setDate(QDate::currentDate());
    connect(ui->dateEdit, &QDateEdit::dateChanged, ui->graphicsView->scene(), &QGraphicsScene::clear);
    connect(ui->toDateEdit, &QDateEdit::dateChanged, ui->graphicsView->scene(), &QGraphicsScene::clear);
}

ReportDialog::~ReportDialog()
{
    delete ui;
    delete loader;
}

void ReportDialog::setMode(ReportMode rm)
{
    m_reportMode = rm;
    // Range modes memerlukan rentang tanggal (dari → sampai);
    // daily modes hanya butuh satu tanggal.
    const bool range = (rm == RangeSalesMode || rm == RangeExpenseMode);
    ui->labelTo->setVisible(range);
    ui->toDateEdit->setVisible(range);
    if (range)
        ui->toDateEdit->setDate(ui->dateEdit->date());
}

void ReportDialog::on_refreshButton_clicked()
{
    if (m_reportMode == SalesMode)
    {
        QDate selectedDate = ui->dateEdit->date();
        auto loaded = loader->loadDailySales(selectedDate);
        ui->graphicsView->showSalesReport(loaded);
    }
    else if (m_reportMode == ExpenseMode)
    {
        QDate selectedDate = ui->dateEdit->date();
        auto loaded = loader->loadDailyExpense(selectedDate);
        ui->graphicsView->showExpenseReport(loaded);
    }
    else
    {
        // Periode (range): dari ui->dateEdit sampai ui->toDateEdit
        DateRange range;
        range.from = ui->dateEdit->date();
        range.to   = ui->toDateEdit->date();
        if (range.to < range.from)
            std::swap(range.from, range.to);

        if (m_reportMode == RangeSalesMode)
        {
            auto loaded = loader->loadRangeSales(range);
            ui->graphicsView->showRangeSalesReport(loaded);
        }
        else // RangeExpenseMode
        {
            auto loaded = loader->loadRangeExpense(range);
            ui->graphicsView->showRangeExpenseReport(loaded);
        }
    }
}

void ReportDialog::on_simpanButton_clicked()
{
    auto file_path = QFileDialog::getSaveFileName(this, "Simpan Laporan", QString("Laporan %1.pdf").arg(ui->dateEdit->date().toString("yyyyMMdd")), "PDF (*.pdf)");
    if (file_path.isEmpty())
        return;
    ui->graphicsView->exportToPdf(file_path);
    accept();
}
