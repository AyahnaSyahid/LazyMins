#include "createinvoicedialog.h"
#include "ui_createinvoicedialog.h"
#include "notacolumndelegate.h"
#include "orderinputdialog.h"
#include "paymentinputdialog.h"
#include "../databaseinterface.h"

#include <QMessageBox>
#include <QCompleter>
#include <QHeaderView>
#include <QDate>
#include <QItemSelectionModel>
#include <QStandardItem>

void CreateInvoiceDialog::RegisterMetaType() {
  qRegisterMetaType<InvoiceData::ItemData>();
  qRegisterMetaType<InvoiceData>();
}

CreateInvoiceDialog::CreateInvoiceDialog(QWidget *parent) :
  ui(new Ui::CreateInvoiceDialog),
  notaModel(new QStandardItemModel(this)),
  adminModel(new QSqlQueryModel(this)),
  konsumenModel(new QSqlQueryModel(this)),
  QDialog(parent)
{
  ui->setupUi(this);
  auto nn = new NotaNoDelegate(this);
  auto nh = new NotaHPDelegate(this);
  auto ns = new NotaSubTotalDelegate(this);
  notaModel->setColumnCount(5);
  notaModel->setHorizontalHeaderLabels(QString("No.;Nama Barang;Harga Satuan;Qty;SubTotal").split(";"));
  ui->notaTable->setModel(notaModel);
  ui->notaTable->setItemDelegateForColumn(0, nn);
  ui->notaTable->setItemDelegateForColumn(2, nh);
  ui->notaTable->setItemDelegateForColumn(3, nh);
  ui->notaTable->setItemDelegateForColumn(4, ns);
  ui->notaTable->addAction(ui->tableActionInsert);
  ui->notaTable->addAction(ui->tableActionDelete);
  
  adminModel->setQuery("SELECT DISTINCT admin FROM invoices ORDER BY admin", QSqlDatabase::database("JUST-INV_DB", true));
  konsumenModel->setQuery(R"--(
    SELECT customer,
       customer_phone
  FROM invoices
 WHERE created_at = (
        SELECT MAX(created_at) 
          FROM invoices t2
         WHERE t2.customer = invoices.customer )
 GROUP BY customer; )--", 
      QSqlDatabase::database("JUST-INV_DB", true));
  
  auto comp1 = new QCompleter(this);
  auto comp2 = new QCompleter(this);
  
  comp1->setModel(adminModel);
  comp2->setModel(konsumenModel);
  
  // LAMDAS SET PHONE HERE
 connect(comp2, QOverload<const QModelIndex &>::of(&QCompleter::activated),
   [=](const QModelIndex &index) {
     if (!index.siblingAtColumn(1).data(Qt::EditRole).isNull()) {
       ui->customerPhoneLineEdit->setText(index.siblingAtColumn(1).data(Qt::EditRole).toString());
     }
   });
  
  ui->adminLineEdit->setCompleter(comp1);
  ui->customerLineEdit->setCompleter(comp2);
  
  auto sm = ui->notaTable->selectionModel();
  connect(sm, &QItemSelectionModel::selectionChanged, [this]() {
    ui->tableActionDelete->setEnabled(ui->notaTable->selectionModel()->hasSelection());
    });
  connect(notaModel, &QAbstractItemModel::dataChanged, this, &CreateInvoiceDialog::updateTotalPrice);
  ui->tanggalDateEdit->setDate(QDate::currentDate());
  connect(notaModel, &QAbstractItemModel::rowsInserted, this, &CreateInvoiceDialog::updateTotalPrice);
  connect(notaModel, &QAbstractItemModel::rowsRemoved, this, &CreateInvoiceDialog::updateTotalPrice);
  DatabaseInterface &di = DatabaseInterface::instance();
  connect(this, &CreateInvoiceDialog::saveNotaRequest, &di, &DatabaseInterface::saveInvoiceData);
  connect(&di, &DatabaseInterface::saveDone, this, &CreateInvoiceDialog::onNotaSaveDone);
}

CreateInvoiceDialog::~CreateInvoiceDialog() {
  delete ui;
}

void CreateInvoiceDialog::resizeEvent(QResizeEvent *re) {
  // edit notaTable columns size
  auto hh = ui->notaTable->horizontalHeader();
  int oldSectionSize = hh->sectionSize(1);
  hh->resizeSection(1, oldSectionSize - (re->oldSize().width() - re->size().width()));
  QDialog::resizeEvent(re);
};

void CreateInvoiceDialog::showEvent(QShowEvent *se) {
  auto hh = ui->notaTable->horizontalHeader();
  hh->resizeSection(0, 50);
  hh->resizeSection(1, 907 - (50+85+70+100));
  hh->resizeSection(2, 85);
  hh->resizeSection(3, 65);
  hh->resizeSection(4, 95);
};

void CreateInvoiceDialog::on_tableActionInsert_triggered() {
  auto orderInputDialog = new OrderInputDialog(this);
  connect(orderInputDialog, &QDialog::accepted, this, &CreateInvoiceDialog::inputDialogAccepted);
  orderInputDialog->open();
}

void CreateInvoiceDialog::on_tableActionDelete_triggered() {
  if (ui->notaTable->selectionModel()->hasSelection()) {
    QModelIndexList mil = ui->notaTable->selectionModel()->selectedRows(0);
    std::sort(mil.begin(), mil.end(), [](const QModelIndex& a, const QModelIndex& b) {
      return a.row() > b.row();
    });
    for(const auto ixr : mil) {
      ui->notaTable->model()->removeRows(ixr.row(), 1);
    }
  }
}

void CreateInvoiceDialog::inputDialogAccepted() {
  auto orderInputDialog = qobject_cast<OrderInputDialog*>(sender());
  QList<QStandardItem*> rowItems;
  for(int i=0; i<5; ++i) {
    rowItems << new QStandardItem();
  }
  rowItems[0]->setData(notaModel->rowCount() + 1, Qt::EditRole);
  rowItems[0]->setEditable(false);
  rowItems[1]->setData(orderInputDialog->namaBarang(), Qt::EditRole);
  rowItems[2]->setData(orderInputDialog->harga(), Qt::EditRole);
  rowItems[3]->setData(orderInputDialog->qty(), Qt::EditRole);
  rowItems[4]->setData(orderInputDialog->subTotal(), Qt::EditRole);
  rowItems[4]->setEditable(false);
  notaModel->insertRow(notaModel->rowCount(), rowItems);
  orderInputDialog->deleteLater();
}

int CreateInvoiceDialog::totalPrice() const {
  int tt = 0;
  for(int r=0; r < notaModel->rowCount(); ++r) {
    tt += notaModel->index(r, 4).data(Qt::EditRole).toInt();
  }
  return tt;
}

void CreateInvoiceDialog::updateTotalPrice() {
  ui->totalLineEdit->setText(locale().toString(totalPrice()));
}

void CreateInvoiceDialog::on_simpanButton_clicked() {
  auto ida = getInvoiceData();
  QString errs {};
  if ( !verifyInvoiceData(ida, errs)) {
    if (errs.contains("Admin") ) {
      errs = "Nama Admin harus diisi";
    } else if (errs.contains("Konsumen") ) {
      errs = "Nama Konsumen harus diisi";
    } else {
      errs = "Tidak ada item untuk dijual ?";
    }
    QMessageBox::information(this, "Periksa Input", errs);
    return;
  }
  emit saveNotaRequest(ida);
}

void CreateInvoiceDialog::on_bayarButton_clicked()
{
  auto ida = getInvoiceData();
  QString errs;
  
  if (! verifyInvoiceData(ida, errs)) {
    if (errs.contains("Admin") ) {
      errs = "Nama Admin harus diisi";
    } else if (errs.contains("Konsumen") ) {
      errs = "Nama Konsumen harus diisi";
    } else {
      errs = "Tidak ada item untuk dijual ?";
    }
    QMessageBox::information(this, "Periksa Input", errs);
    return;
  }
  
  auto payd = new PaymentInputDialog(ida, this);
  connect(payd, &QDialog::finished, this, &CreateInvoiceDialog::onPaymentDialogFinished);
  payd->open();
}

void CreateInvoiceDialog::onPaymentDialogFinished(int result) {
  auto payd = qobject_cast<PaymentInputDialog*>(sender());
  if (result == QDialog::Rejected) {
    payd->deleteLater();
    QMessageBox::information(this, "Pemberitahuan", "Pembayaran dibatalkan");
    return;
  }
  emit invoiceSaved();
  emit paymentSaved();
  resetUi();
}

void CreateInvoiceDialog::onNotaSaveDone(bool state) {
  QMessageBox::information(this, "Selesai", state ? "Nota berhasil disimpan" : "Nota gagal disimpan");
  if (state) {
    resetUi();
    emit invoiceSaved();
  }
}

void CreateInvoiceDialog::resetUi() {
  ui->adminLineEdit->clear();
  ui->customerLineEdit->clear();
  ui->customerPhoneLineEdit->clear();
  ui->tanggalDateEdit->setDate(QDate::currentDate());
  ui->totalLineEdit->setText("0");
  notaModel->removeRows(0, notaModel->rowCount());
}

InvoiceData CreateInvoiceDialog::getInvoiceData() const {
  InvoiceData ida;
  ida.adminName = ui->adminLineEdit->text().trimmed();
  ida.customerName = ui->customerLineEdit->text().trimmed();
  ida.customerPhone = ui->customerPhoneLineEdit->text().trimmed();
  ida.dateString = ui->tanggalDateEdit->date().toString("yyyy-MM-dd");
  ida.total = totalPrice();
  for(int i=0; i < notaModel->rowCount(); ++i) {
    ida.itemList << InvoiceData::ItemData { 
      notaModel->index(i, 1).data().toString(), 
      notaModel->index(i, 2).data(Qt::EditRole).toInt(), 
      notaModel->index(i, 3).data(Qt::EditRole).toInt(), 
      notaModel->index(i, 4).data(Qt::EditRole).toInt(), }; }
  return ida;
}

bool CreateInvoiceDialog::verifyInvoiceData(const InvoiceData &ida, QString &err) const {
  if (ida.adminName.isEmpty()) {
    err = "Nama Admin Kosong";
    return false;
  }
  if (ida.customerName.isEmpty()) {
    err = "Nama Konsumen Kosong";
    return false;
  }
  if (ida.itemList.count() < 1) {
    err = "Tidak ada Item";
    return false;
  }
  return true;
}