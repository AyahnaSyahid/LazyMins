#include "konsumenpickerdialog.h"
#include "ui_konsumenpickerdialog.h"
#include "src/managers/basemanager.h"

KonsumenPickerDialog::KonsumenPickerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KonsumenPickerDialog),
    model(new QSqlQueryModel(this))
{
    ui->setupUi(this);
    model->setQuery(R"--(SELECT k.id,
       nama_lengkap,
       pl.id
       level_name
  FROM konsumen k
       JOIN
       price_levels pl ON k.price_level_id = pl.id;)--", BaseManager::connection);
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Nama");
    model->setHeaderData(3, Qt::Horizontal, "Level Harga");
    ui->konsumenView->hideColumn(0);
    ui->konsumenView->hideColumn(2);
    ui->konsumenView->horizontalHeader()->setStretchLastSection(true);
    ui->konsumenView->setModel(model);
}

KonsumenPickerDialog::~KonsumenPickerDialog()
{
    delete ui;
}

void KonsumenPickerDialog::on_konsumenView_clicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }
    // Handle the selection of a customer
}   