#include "csvproductupdaterdialog.h"
#include "ui_csvproductupdaterdialog.h"
#include "helper.h"
#include "productitem.h"

#include <QStandardItemModel>
#include <QFileSystemModel>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QCompleter>
#include <QSqlQuery>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QtDebug>

CSVProductUpdaterDialog::CSVProductUpdaterDialog(QWidget* parent)
    : ui(new Ui::CSVProductUpdaterDialog), QDialog(parent)
{
    ui->setupUi(this);
    auto cpl = new QCompleter(this);
    auto fsm = new QFileSystemModel(this);
    fsm->setRootPath(QDir::currentPath());
    cpl->setModel(fsm);
    ui->lineEdit->setCompleter(cpl);
    ui->lineEdit->setText(QDir::currentPath());
    iMod = new QStandardItemModel(0, 6);
    iMod->setHeaderData(0, Qt::Horizontal, "Kode");
    iMod->setHeaderData(1, Qt::Horizontal, "Nama");
    iMod->setHeaderData(2, Qt::Horizontal, "QR/Barcode");
    iMod->setHeaderData(3, Qt::Horizontal, "H. Beli");
    iMod->setHeaderData(4, Qt::Horizontal, "H. Jual");
    iMod->setHeaderData(5, Qt::Horizontal, "Deskripsi");
    ui->tableView->setModel(iMod);
    ui->tableView->addAction(ui->actionDeleteSelected);
}

CSVProductUpdaterDialog::~CSVProductUpdaterDialog(){
    delete ui;
    delete iMod;
}

void CSVProductUpdaterDialog::on_pushButton_clicked(){
    auto fn = QFileDialog::getOpenFileName(this, "Pilih file *.csv", ui->lineEdit->text(), "CSV Data File (*.csv)");
    if(!fn.isEmpty()) {
        ui->lineEdit->setText(QDir(fn).absoluteFilePath(fn));
        loadFile(fn, ';');
    }
}

void CSVProductUpdaterDialog::on_pushButton2_clicked(){
    QSqlQuery q;
    auto cds = StringHelper::currentDateTimeString();
    ProductItem pi;
    for(int r = 0; r < iMod->rowCount(); ++r) {
        pi = ProductItem(iMod->index(r, 0).data(Qt::EditRole).toString());
        pi.name = iMod->index(r, 1).data(Qt::EditRole).toString();
        pi.qr_bar = iMod->index(r, 2).data(Qt::EditRole).toString();
        pi.cost = iMod->index(r, 3).data(Qt::EditRole).toLongLong();
        pi.price = iMod->index(r, 4).data(Qt::EditRole).toLongLong();
        pi.description = iMod->index(r, 5).data(Qt::EditRole).toString();
        pi.use_area = 0;
        pi.save();
    }
    QDialog::accept();
}

void CSVProductUpdaterDialog::loadFile(const QString& fn, const char sep) {
    QFile ff(fn);
    if(!ff.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::information(this, "Gagal", "Gagal membuka file");
        return;
    }
    QTextStream ts(&ff);
    QString cline;
    QStringList sl;
    cline = ts.readLine();
    while (!ts.atEnd()) {
        cline = ts.readLine();
        sl = cline.split(sep);
        if(sl.count() < 6) {
            continue;
        }
        iMod->insertRow(0);
        iMod->setData(iMod->index(0, 0), sl.at(0));
        iMod->setData(iMod->index(0, 1), sl.at(1));
        iMod->setData(iMod->index(0, 2), sl.at(2));
        iMod->setData(iMod->index(0, 3), sl.at(3).toDouble());
        iMod->setData(iMod->index(0, 4), sl.at(4).toDouble());
        iMod->setData(iMod->index(0, 5), sl.at(5));
    }
    ff.close();
    ui->tableView->resizeColumnsToContents();
}

void CSVProductUpdaterDialog::on_actionDeleteSelected_triggered()
{
    auto sm = ui->tableView->selectionModel()->selectedIndexes();
    if(sm.count()) {
        QModelIndexList mil;
        for(auto i : sm) {
            if(!mil.contains(i.siblingAtColumn(0))) {
                mil << i.siblingAtColumn(0);
            }
        }
        qSort(mil.begin(), mil.end(), [](QModelIndex a, QModelIndex b){return a.row() > b.row();});
        for(auto mi : mil) {
            ui->tableView->model()->removeRow(mi.row());
        }
    }
}