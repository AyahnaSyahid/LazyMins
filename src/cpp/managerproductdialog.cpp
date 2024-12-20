#include "managerproductdialog.h"
#include "../ui/ui_managerproductdialog.h"
#include "editproductdialog.h"
#include "helper.h"
#include "itemdelegates.h"
#include <QSqlTableModel>
#include <QHeaderView>
#include <QModelIndex>
#include <QMessageBox>
#include <QtDebug>

ManagerProductDialog::ManagerProductDialog(QWidget* parent)
    : ui(new Ui::ManagerProductDialog), QDialog(parent)
{
    ui->setupUi(this);
    tableModel = new QSqlTableModel;
    tableModel->setTable("products");
    tableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    tableModel->select();
    ui->tableView->setModel(tableModel);
    tableModel->setHeaderData(1, Qt::Horizontal, "Kode Barang");
    tableModel->setHeaderData(2, Qt::Horizontal, "Nama");
    tableModel->setHeaderData(3, Qt::Horizontal, "Kode QR/Bar");
    tableModel->setHeaderData(7, Qt::Horizontal, "Deskripsi");
    tableModel->setHeaderData(8, Qt::Horizontal, "Terdaftar");
    tableModel->setHeaderData(9, Qt::Horizontal, "Update");
    ui->tableView->hideColumn(0);
    ui->tableView->hideColumn(4);
    ui->tableView->hideColumn(5);
    ui->tableView->hideColumn(6);
    ui->tableView->hideColumn(8);
    auto it = new LihatHari(this);
    // it->setIncludeTime(false);
    ui->tableView->setItemDelegateForColumn(9, it);
    ui->tableView->setItemDelegateForColumn(8, it);
    ui->tableView->addAction(ui->aDeleteSelectedIndexes);
    ui->tableView->resizeColumnsToContents();
}

ManagerProductDialog::~ManagerProductDialog() {
    delete ui;
    delete tableModel;
}

void ManagerProductDialog::on_lineEdit_textChanged(const QString& st) {
    QString s("LOWER(code || name || description) LIKE '%%%1%%'");
    s = s.arg(st);
    if(s.isEmpty()) {
      tableModel->setFilter("");
      return;
    }
    tableModel->setFilter(s);
}

void ManagerProductDialog::on_aDeleteSelectedIndexes_triggered() {
    auto mm = ui->tableView->selectionModel()->selectedRows();
    int rowCount = tableModel->rowCount();
    if(mm.count()) {
        auto yes = MessageHelper::question(this, "Konfirmasi", QString("Hapus %1 produk terpilih ?\nProses ini tidak dapat dibatalkan").arg(mm.count()));
        if(yes == QMessageBox::Yes) {
            std::sort(mm.begin(), mm.end(), [](const QModelIndex& a, QModelIndex& b) -> bool {return a.row() > b.row();});
            for(auto i=mm.cbegin(); i != mm.cend(); ++i) {
                tableModel->removeRow(i->row());
            }
            if(!tableModel->submitAll()) {
                MessageHelper::information(this, "Tidak diizinkan", "Beberapa produk menjadi referensi nota (mempunyai catatan jual)");
            } else {
                MessageHelper::information(this, "Berhasil", "Beberapa produk telah di hapus");
            }
        }    
    }
}

void ManagerProductDialog::on_pushButton_clicked() {
    auto sm = ui->tableView->selectionModel();
    auto sc = sm->selectedRows(1);
    if(sc.count() < 1) {
        MessageHelper::information(nullptr, "Kesalahan", "Pilih produk terlebih dahulu sebelum menghapus");
        return;
    }
    ui->aDeleteSelectedIndexes->trigger();
}

void ManagerProductDialog::on_tableView_doubleClicked(const QModelIndex& mi) {
    EditProductDialog* epd = new EditProductDialog();
    epd->setAttribute(Qt::WA_DeleteOnClose);
    if (epd->setCurrentProduct(mi.siblingAtColumn(0).data(Qt::EditRole).toInt())){
        connect(epd, &EditProductDialog::productEdited, tableModel, &QSqlTableModel::select);
        epd->adjustSize();
    } else {
        MessageHelper::information(this, "Kesalahan", QString("Produk dengan ID: %1, Tidak dapat ditemukan").arg(mi.siblingAtColumn(0).data(Qt::EditRole).toInt()));
        epd->deleteLater();
        return;
    }
    epd->open();
}

void ManagerProductDialog::accept() {
    if(tableModel->isDirty()) {
        auto ans = MessageHelper::question(this, "Konfirmasi", "Simpan semua perubahan data dalam tabel?");
        if(ans == QMessageBox::Yes) {
            tableModel->submitAll();
        }
    }
    QDialog::accept();
}