#include "instantorderdialog.h"
#include "ui_instantorderdialog.h"

#include "customerpickerdialog.h"
#include "src/customs/orderitemdelegate.h"
#include "src/dialogs/orderitemdialog.h"
#include "src/managers/helpers.h"
#include "src/utils/sqltransaction.h"
#include <QSqlQueryModel>
#include <QSqlError>
#include <QTableView>
#include <QHeaderView>
#include <QMessageBox>

namespace {
  class executor {
    public:
    QString errorString;
    bool saveToDB(Ui::InstantOrderDialog *ui, OrderModel &omod, const QVariantMap& paymentInfo);
    OrderHeader generateOrderHeader( Ui::InstantOrderDialog *ui , const QVariantMap& customerSet) const;
  };
  bool executor::saveToDB(Ui::InstantOrderDialog *ui, OrderModel &omod, const QVariantMap &paymentInfo)
  {
    OrderHeader oh = generateOrderHeader(ui, paymentInfo);
    if (!omod.commit()) {
      errorString = BaseManager::connection.lastError().text();
    }
    SqlTransaction tr;
    if(!tr.started()) errorString = "Gagal membuat transaksi Database"; return false;
    
  }
  OrderHeader executor::generateOrderHeader(Ui::InstantOrderDialog *ui, const QVariantMap &cs) const
  {
      OrderHeader oh;
      oh.customer_name = ui->nameLineEdit->text();
      oh.customer_phone = ui->phoneLineEdit->text();
      oh.price_level_id = cs["price_level"].toInt();
      
      if(cs["name"].toString() != ui->nameLineEdit->text()) {
        oh.customer_id = -1;    
      } else {
        oh.customer_id = cs["id"].toInt();
      }
      oh.discount_amount = ui->discountSpinBox->value();
      return oh;
  }
}

InstantOrderDialog::InstantOrderDialog(QWidget *p):
  ui(new Ui::InstantOrderDialog), 
  omod(this),
  QDialog(p)
{
  ui->setupUi(this);
  ui->labelInvoiceCode->setText(m_invm.nextNumber());
  ui->orderListView->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->orderListView->setModel(&omod);
  ui->orderListView->setItemDelegate(new OrderItemDelegate(this));
  
  ui->lHargaComboBox->blockSignals(true);
  auto tm = new QTableView;
  auto qm = new QSqlQueryModel(this);
  qm->setQuery("SELECT id, level_name, description FROM price_levels", BaseManager::connection);
  ui->lHargaComboBox->setModel(qm);
  ui->lHargaComboBox->setModelColumn(1);  
  ui->lHargaComboBox->setView(tm);
  tm->verticalHeader()->setMinimumSectionSize(20);
  tm->verticalHeader()->setDefaultSectionSize(18);
  tm->hideColumn(0);
  tm->verticalHeader()->hide();
  tm->horizontalHeader()->hide();
  tm->resizeColumnsToContents();
  tm->setMinimumWidth(tm->horizontalHeader()->length());
  ui->lHargaComboBox->setCurrentIndex(-1);
  ui->lHargaComboBox->blockSignals(false);
  
  
  auto countUpdate = [this](){ ui->labelItemCount->setText(QString("Items : %1").arg(omod.rowCount())); };
  connect(&omod, &QAbstractItemModel::rowsInserted, countUpdate);
  connect(&omod, &QAbstractItemModel::rowsRemoved, countUpdate);
  connect(&omod, &OrderModel::orderTotalChanged, this, &InstantOrderDialog::recalculate);
  connect(ui->discountSpinBox, &QSpinBox::valueChanged, this, &InstantOrderDialog::recalculate);
  connect(ui->bayarSpinBox, &QSpinBox::valueChanged, this, &InstantOrderDialog::recalculate);
}

InstantOrderDialog::~InstantOrderDialog() { delete ui; }

void InstantOrderDialog::on_pilihButton_clicked()
{
  auto dialog = new CustomerPickerDialog(this);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setWindowFlags(dialog->windowFlags() | Qt::FramelessWindowHint);
  auto buttonGeo = ui->pilihButton->geometry();
  auto globalPos = mapToGlobal(buttonGeo.topRight());
  dialog->move(globalPos);

  connect(dialog, &CustomerPickerDialog::customerPicked,
    [this](const QSqlRecord &record)
      {
        ui->nameLineEdit->setText(record.value("nama_lengkap").toString());
        ui->phoneLineEdit->setText(record.value("nomor_telp").toString());
        customerSet.id = record.value("id").toInt();
        customerSet.name = record.value("nama_lengkap").toString();
        customerSet.price_level = record.value("pl_id").toInt(); 
        auto model = ui->lHargaComboBox->model();
        auto indexes = model->match(model->index(0, 0), Qt::DisplayRole, customerSet.price_level, 1, Qt::MatchExactly);
        if(indexes.count()) {
          ui->lHargaComboBox->setCurrentIndex(indexes.at(0).row());
        };
      });
  dialog->open();
}

#include <QMenu>
#include <QAction>

void InstantOrderDialog::on_orderListView_customContextMenuRequested(const QPoint& p) {
  if (!checkInput()) return;
  auto global_point = ui->orderListView->viewport()->mapToGlobal(p);
  QMenu context;
  auto add = context.addAction("Tambah");
  
  auto pIndex = ui->orderListView->indexAt(p);
  if (pIndex.isValid()) {
    auto edit = context.addAction("Edit");
    connect(edit, &QAction::triggered, [this, &pIndex]() {
      auto dialog = new OrderItemDialog(this);
      dialog->setAttribute(Qt::WA_DeleteOnClose);
      dialog->setOrder(&omod.itemRef(pIndex.row()));
      dialog->setCustomerPriceLevel(ui->lHargaComboBox->model()->index(ui->lHargaComboBox->currentIndex(), 0).data().toInt());
      connect(dialog, &OrderItemDialog::editFinished, this, &InstantOrderDialog::recalculate);
      dialog->open();
    });
  }
  auto hapus = context.addAction("Hapus");
  
  connect(add, &QAction::triggered, [this](){
    auto dialog = new OrderItemDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setCustomerPriceLevel(ui->lHargaComboBox->model()->index(ui->lHargaComboBox->currentIndex(), 0).data().toInt());
    connect(dialog, &OrderItemDialog::itemCreated, this, &InstantOrderDialog::addItem);
    dialog->open();
  });
  
  connect(hapus, &QAction::triggered, [this](){});
  context.exec(global_point);
}

void InstantOrderDialog::recalculate() {
  auto subs = calculatedSubtotal();
  auto total = 0;
  ui->labelSubtotal->setText(locale().toString(subs));
  total = subs + ui->ppnSpinBox->value() - ui->discountSpinBox->value();
  ui->bayarSpinBox->setMinimum(total);
  ui->labelTotal->setText(locale().toString(total));
  ui->labelKembalian->setText(locale().toString(qAbs(total - ui->bayarSpinBox->value())));
}

bool InstantOrderDialog::checkInput() {
  if (ui->nameLineEdit->text().isEmpty()) {
    QMessageBox::warning(this, "Periksa Input", "Anda belum mengisi nama konsumen");
    ui->nameLineEdit->setFocus(Qt::OtherFocusReason);
    return false;
  }
  if (ui->phoneLineEdit->text().isEmpty()) {
    QMessageBox::warning(this, "Periksa Input", "Anda belum kontak telepon konsumen");
    ui->phoneLineEdit->setFocus(Qt::OtherFocusReason);
    return false;
  }
  if (ui->lHargaComboBox->currentIndex() < 0) {
    QMessageBox::warning(this, "Periksa Input", "Gagal menerapkan level harga");
    ui->lHargaComboBox->setFocus(Qt::OtherFocusReason);
    return false;
  }
  
  return true;
}

void InstantOrderDialog::on_bayarButton_clicked() {
  if(!checkInput()) return;
  auto inv_code = ui->labelInvoiceCode->text();
  QVariantMap cparam {
    {"name", customerSet.name},
    {"phone", customerSet.phone},
    {"price_level", customerSet.price_level}
  };
  auto p_amount = calculatedSubtotal();
  auto p_disc = ui->discountSpinBox->value();
  
  const QVariantMap payments{
    {"payment_amount", p_amount - p_disc }, 
    {"tax_amount", ui->ppnSpinBox->value()}, 
    {"cash_received", ui->bayarSpinBox->value()}, 
    {"cash_change", ui->bayarSpinBox->value() - (p_amount + p_disc)},
    {"invoice_code", inv_code},
    {"name", customerSet.name},
    {"phone", customerSet.phone},
    {"price_level", customerSet.price_level}
  };
  
  executor exc;
  book ok = exc.saveToDB
  
  auto res = DBOperationHelper::createInstantOrder( oh, omod.items(), inv_code, 
                {  } );
  if(!res.ok) {
    QMessageBox::warning(this, "Operasi Gagal", QString("Transaksi Error:%1").arg(res.error));
    return ;
  }
  accept();
}

int InstantOrderDialog::calculatedSubtotal() const {
  int subs = 0;
  for(int r = 0; r < omod.rowCount(); ++r) {
    subs += omod.itemAt(r).total();
  }
  return subs;
}