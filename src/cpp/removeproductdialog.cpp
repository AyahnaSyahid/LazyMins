#include "removeproductdialog.h"
#include "../ui/ui_removeproductdialog.h"
#include <QSqlTableModel>
#include <QHeaderView>
#include <QModelIndex>
#include <QMessageBox>

RemoveProductDialog::RemoveProductDialog(QWidget* parent)
    : ui(new Ui::RemoveProductDialog), QDialog(parent)
{
    ui->setupUi(this);
    tableModel = new QSqlTableModel;
    tableModel->setTable("product");
    tableModel->select();
    ui->tableView->setModel(tableModel);
    ui->tableView->hideColumn(0);
    ui->tableView->hideColumn(4);
    ui->tableView->hideColumn(5);
    ui->tableView->hideColumn(7);
    ui->tableView->hideColumn(8);
    tableModel->setHeaderData(1, Qt::Horizontal, "Kode Barang");
    tableModel->setHeaderData(2, Qt::Horizontal, "Nama");
    tableModel->setHeaderData(3, Qt::Horizontal, "Kode QR/Bar");
    tableModel->setHeaderData(6, Qt::Horizontal, "Deskripsi");
    ui->tableView->resizeColumnsToContents();
}

RemoveProductDialog::~RemoveProductDialog() {
    delete ui;
    delete tableModel;
}

void RemoveProductDialog::on_lineEdit_textChanged(const QString& st) {
    QString s("LOWER(code || name || desc) LIKE '%%%1%%'");
    s = s.arg(st);
    if(s.isEmpty()) {
      tableModel->setFilter("");
      return;
    }
    tableModel->setFilter(s);
}

void RemoveProductDialog::on_pushButton_clicked() {
    auto sm = ui->tableView->selectionModel();
    auto sc = sm->selectedRows(1);
    if(sc.count() < 1) {
        QMessageBox::information(nullptr, "Kesalahan", "Pilih produk terlebih dahulu sebelum menghapus");
        return;
    }
    auto yes = QMessageBox::question(nullptr, "Konfirmasi Penghapusan", QString("Yakin anda akan menghapus %1 data").arg(sc.count()));
    if(yes == QMessageBox::Yes) {
        std::sort(sc.begin(), sc.end(),
            [](const QModelIndex& a, QModelIndex& b) -> bool { return a.row() > b.row(); });
        for(int i=0; i<sc.count(); ++i) {
            tableModel->removeRow(sc[i].row());
        }
        tableModel->select();
    }
}