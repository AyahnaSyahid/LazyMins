#include "createinvoicedialog.h"
#include "ui_createinvoicedialog.h"
#include "notacolumndelegate.h"
#include "orderinputdialog.h"
#include <QHeaderView>
#include <QDate>
#include <QItemSelectionModel>
#include <QStandardItem>

CreateInvoiceDialog::CreateInvoiceDialog(QWidget *parent) :
  ui(new Ui::CreateInvoiceDialog),
  notaModel(new QStandardItemModel(this)),
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
  
  auto sm = ui->notaTable->selectionModel();
  connect(sm, &QItemSelectionModel::selectionChanged, [this]() {
    ui->tableActionDelete->setEnabled(ui->notaTable->selectionModel()->hasSelection());
    });
  connect(notaModel, &QAbstractItemModel::dataChanged, this, &CreateInvoiceDialog::notaDataChanged);
  ui->tanggalDateEdit->setDate(QDate::currentDate());
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

void CreateInvoiceDialog::notaDataChanged(const QModelIndex& t, const QModelIndex& b, const QList<int> &roles)
{
  if (b.column() == 4) {
    int t = 0;
    for(int i=0; i<notaModel->rowCount(); ++i) {
      t += notaModel->index(i, 4).data(Qt::EditRole).toInt();
    }
    ui->totalLineEdit->setProperty("totalValue", t);
    if (t == 0) {
      ui->cashBackLabel->setText("Kembalian");
      ui->bayarButton->setEnabled(false);
      ui->simpanButton->setEnabled(false);
      ui->spinBoxBayar->setEnabled(false);
      ui->totalLineEdit->setText("0");
      ui->kembalianLineEdit->setText("N/A");
      ui->spinBoxBayar->setValue(0);
      return;
    }
    
    ui->totalLineEdit->setText(QLocale().toString(t));
    if (ui->spinBoxBayar->value() >= t) {
      ui->kembalianLineEdit->setText(locale().toString(ui->spinBoxBayar->value() - t));
      ui->bayarButton->setEnabled(true);
      ui->simpanButton->setEnabled(false);
    } else {
      ui->kembalianLineEdit->setText(locale().toString(t - ui->spinBoxBayar->value()));
      ui->simpanButton->setEnabled(true);
      ui->bayarButton->setEnabled(false);
    }
  }
}

void CreateInvoiceDialog::on_spinBoxBayar_valueChanged(int nv) {
  int t = ui->totalLineEdit->property("totalValue").toInt();
  if (nv >= t) {
    ui->bayarButton->setEnabled(true);
    ui->simpanButton->setEnabled(true);
    ui->kembalianLineEdit->setText(locale().toString(t - nv));
  } else {
    ui->bayarButton->setEnabled(false);
    ui->kembalianLineEdit->setText("N/A");
  }
}