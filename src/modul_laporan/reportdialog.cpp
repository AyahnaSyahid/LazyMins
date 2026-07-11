#include "reportdialog.h"
#include "reportloader.h"
#include "reportview.h"
#include "ui_reportdialog.h"
#include <QFileDialog>
#include <QGraphicsScene>
#include <QSqlDatabase>

#include <QDate>

ReportDialog::ReportDialog(QSqlDatabase &db, int adminId, QWidget *parent)
    : QDialog(parent), ui(new Ui::ReportDialog), loader(new ReportLoader(db, adminId)), m_reportMode(SalesMode)
{
    ui->setupUi(this);
    ui->dateEdit->setDate(QDate::currentDate());
    connect(ui->dateEdit, &QDateEdit::dateChanged, ui->graphicsView->scene(), &QGraphicsScene::clear);
}

ReportDialog::~ReportDialog()
{
    delete ui;
    delete loader;
}

void ReportDialog::setMode(ReportMode rm)
{
    m_reportMode = rm;
}

void ReportDialog::on_refreshButton_clicked()
{
    QDate selectedDate = ui->dateEdit->date();
    if (m_reportMode == SalesMode)
    {
        auto loaded = loader->loadDailySales(selectedDate);
        ui->graphicsView->showSalesReport(loaded);
    }
    else
    {
        auto loaded = loader->loadDailyExpense(selectedDate);
        ui->graphicsView->showExpenseReport(loaded);
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
