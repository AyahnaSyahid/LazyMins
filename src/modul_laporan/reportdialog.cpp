#include "reportdialog.h"
#include "reportloader.h"
#include "reportview.h"
#include "ui_reportdialog.h"
#include <QFileDialog>
#include <QSqlDatabase>
#include <QGraphicsScene>

#include <QDate>

ReportDialog::ReportDialog(QSqlDatabase &db, int adminId, QWidget *parent) : QDialog(parent), ui(new Ui::ReportDialog), loader(new ReportLoader(db, adminId))
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

void ReportDialog::on_refreshButton_clicked()
{
    auto loaded = loader->loadDailySales(ui->dateEdit->date());
    ui->graphicsView->showSalesReport(loaded);
}

void ReportDialog::on_simpanButton_clicked()
{
    auto file_path = QFileDialog::getSaveFileName(this, "Simpan Laporan", QString("Laporan %1.pdf").arg(ui->dateEdit->date().toString("yyyyMMdd")), "PDF (*.pdf)");
    if (file_path.isEmpty())
        return;
    ui->graphicsView->exportToPdf(file_path);
    accept();
}
