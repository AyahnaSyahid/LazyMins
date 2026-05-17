#include "instantorderdialog.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QTableView>

#include "customerpickerdialog.h"
#include "src/customs/buttonguard.h"
#include "src/customs/orderitemdelegate.h"
#include "src/dialogs/orderitemdialog.h"
#include "src/controllers/instantorder.h"
#include "src/utils/sessionmanager.h"
#include "ui_instantorderdialog.h"

/* namespace {
class executor {
 public:
  QString errorString;
  bool saveToDB(Ui::InstantOrderDialog* ui, OrderModel& omod,
                const QVariantMap& paymentInfo);
  OrderHeader generateOrderHeader(Ui::InstantOrderDialog* ui,
                                  const QVariantMap& customerSet) const;
};
bool executor::saveToDB(Ui::InstantOrderDialog* ui, OrderModel& omod,
                        const QVariantMap& paymentInfo) {
  OrderHeader oh = generateOrderHeader(ui, paymentInfo);
  QList<OrderItem> ois = omod.items();

  SqlTransaction tr;
  ItemFlowService ifs;
  FinancialLedgerService fls;

  if (!tr.started()) {
    errorString = "Gagal membuat transaksi Database";
    return false;
  }

  OrderManager om;
  QVariantMap omParams{
      {"order_number", om.nextNumber()},
      {"customer_name", oh.customer_name},
      {"customer_phone", oh.customer_phone},
      {"price_level_id", oh.price_level_id},
      {"discount_amount", oh.discount_amount},
      {"staging_status", "completed"},
      {"completion_date",
       QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")},
      {"internal_notes", "instant orders"},
      {"admin_id", SessionManager::instance().currentUserId()}};

  if (oh.customer_id > 0) omParams["customer_id"] = oh.customer_id;

  auto optOrder = om.create(omParams);

  if (!optOrder.has_value()) {
    qWarning() << "Pembuatan Order Gagal" << om.errorString();
    errorString = om.errorString();
    return false;
  }

  QList<int> itemIds;
  for (auto item : ois) {
    item.order_id = optOrder->value("id").toInt();
    if (!item.save(BaseManager::connection)) {
      qWarning() << "Pembuatan Order Item Gagal"
                 << BaseManager::connection.lastError().text();
      errorString = BaseManager::connection.lastError().text();
      return false;
    }
    if (!ifs.processItemSold(item.id)) {
      errorString = ifs.errorString();
      qWarning() << "Pendaftaran Item Flow Gagal"
                 << BaseManager::connection.lastError().text();
      return false;
    };
    itemIds << item.id;
  }

  if (!om.addItems(optOrder->value("id").toInt(), itemIds)) {
    qWarning() << "Pendaftaran Item ke Order Gagal" << om.errorString();
    errorString = om.errorString();
    return false;
  }

  InvoiceManager iman;
  auto optInvoice = iman.create(
      {{"invoice_number", paymentInfo["invoice_number"]},
       {"customer_id", optOrder->value("customer_id")},
       {"customer_name", optOrder->value("customer_name")},
       {"customer_phone", optOrder->value("customer_phone")},
       {"price_level_id", optOrder->value("price_level_id")},
       {"admin_id", SessionManager::instance().currentUserId()},
       {"price_level_id", SessionManager::instance().currentUserId()},
       {"tax_amount", ui->ppnSpinBox->value()}});

  if (!optInvoice.has_value()) {
    qWarning() << "Pembuatan Invoice Gagal" << iman.errorString();
    errorString = iman.errorString();
    return false;
  }

  if (!iman.addOrders(optInvoice->value("id").toInt(),
                      {optOrder->value("id").toInt()})) {
    qWarning() << "Penadaftaran Order ke Invoice Gagal" << iman.errorString();
    errorString = iman.errorString();
    return false;
  }

  optInvoice = iman.getById(optInvoice->value("id").toInt());

  if (!optInvoice.has_value()) {
    qWarning() << "Pembuatan Invoice Gagal" << iman.errorString();
    errorString = "Data invoice tidak ditemukan";
    return false;
  }

  PaymentManager pm;
  auto optPay = pm.create(
      {{"invoice_id", optInvoice->value("id")},
       {"payment_number", pm.nextNumber()},
       {"amount", paymentInfo["payment_amount"]},
       {"cash_received", paymentInfo["cash_received"]},
       {"cash_change", paymentInfo["cash_change"]},
       {"akun_transaksi_id", 1},                                     // for cash
       {"verification_status", "verified"},                          // for cash
       {"verified_by", SessionManager::instance().currentUserId()},  // for cash
       {"verified_at", QDateTime::currentDateTimeUtc().toString(
                           "yyyy-MM-dd HH:mm:ss")},  // for cash
       {"admin_id", SessionManager::instance().currentUserId()},
       {"notes", "Added by instant order"}});

  if (!optPay.has_value()) {
    qWarning() << "Pembuatan Payment Gagal" << pm.errorString();
    errorString = pm.errorString();
    return false;
  }

  if (!fls.handlePayment(optPay->value("id").toInt())) {
    qWarning() << "Pembuatan Financial Ledger Gagal" << fls.errorString();
    errorString = fls.errorString();
    return false;
  }
  errorString = "Gagal commit database";
  return tr.commit();
}

OrderHeader executor::generateOrderHeader(Ui::InstantOrderDialog* ui,
                                          const QVariantMap& cs) const {
  OrderHeader oh;
  oh.customer_name = cs["name"].toString();
  oh.customer_phone = cs["phone"].toString();  // Gunakan dari map
  oh.price_level_id = cs["price_level"].toInt();
  oh.discount_amount = ui->discountSpinBox->value();

  // Pastikan jika customer_id tidak ada di map, berikan nilai default -1 atau 0
  oh.customer_id = cs.contains("customer_id") ? cs["customer_id"].toInt() : -1;
  return oh;
}
} */  // namespace

InstantOrderDialog::InstantOrderDialog(QWidget* p)
    : ui(new Ui::InstantOrderDialog), omod(this), QDialog(p) {
  ui->setupUi(this);
  ui->labelInvoiceCode->setText(m_invm.nextNumber());
  ui->orderListView->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->orderListView->setModel(&omod);
  ui->orderListView->setItemDelegate(new OrderItemDelegate(this));

  ui->lHargaComboBox->blockSignals(true);
  auto tm = new QTableView;
  auto qm = new QSqlQueryModel(this);
  qm->setQuery("SELECT id, level_name, description FROM price_levels",
               BaseManager::connection);
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

  auto countUpdate = [this]() {
    ui->labelItemCount->setText(QString("Items : %1").arg(omod.rowCount()));
  };
  connect(&omod, &QAbstractItemModel::rowsInserted, countUpdate);
  connect(&omod, &QAbstractItemModel::rowsRemoved, countUpdate);
  connect(&omod, &OrderModel::orderTotalChanged, this,
          &InstantOrderDialog::recalculate);
  connect(ui->discountSpinBox, &QSpinBox::valueChanged, this,
          &InstantOrderDialog::recalculate);
  connect(ui->bayarSpinBox, &QSpinBox::valueChanged, this,
          &InstantOrderDialog::recalculate);
  auto hdr = omod.header();
  hdr.admin_id = SessionManager::instance().currentUserId();
  omod.setHeaderField(hdr);
}

InstantOrderDialog::~InstantOrderDialog() { delete ui; }

OrderHeader InstantOrderDialog::orderHeader() const {
  OrderHeader oh = omod.header();
  if (customerSet.name != ui->nameLineEdit->text()) {
    oh.customer_name = ui->nameLineEdit->text();
    oh.customer_id = -1;
  }
  return oh;
}

void InstantOrderDialog::on_pilihCustomer_clicked() {
  auto dialog = new CustomerPickerDialog(this);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setWindowFlags(dialog->windowFlags() | Qt::FramelessWindowHint);
  auto buttonGeo = ui->pilihCustomer->geometry();
  auto globalPos = mapToGlobal(buttonGeo.topRight());
  connect(dialog, &CustomerPickerDialog::customerPicked, this,
          &InstantOrderDialog::setCustomerRecord);
  dialog->move(globalPos);
  dialog->open();
}

#include <QAction>
#include <QMenu>

void InstantOrderDialog::on_orderListView_customContextMenuRequested(
    const QPoint& p) {
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
      dialog->setCustomerPriceLevel(
          ui->lHargaComboBox->model()
              ->index(ui->lHargaComboBox->currentIndex(), 0)
              .data()
              .toInt());
      connect(dialog, &OrderItemDialog::editFinished, this,
              &InstantOrderDialog::recalculate);
      dialog->open();
    });
  }
  auto hapus = context.addAction("Hapus");

  connect(add, &QAction::triggered, [this]() {
    auto dialog = new OrderItemDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setCustomerPriceLevel(
        ui->lHargaComboBox->model()
            ->index(ui->lHargaComboBox->currentIndex(), 0)
            .data()
            .toInt());
    connect(dialog, &OrderItemDialog::itemCreated, this,
            &InstantOrderDialog::addItem);
    dialog->open();
  });

  connect(hapus, &QAction::triggered, [this]() {});
  context.exec(global_point);
}

void InstantOrderDialog::recalculate() {
  auto subs = calculatedSubtotal();
  auto total = 0;
  ui->labelSubtotal->setText(locale().toString(subs));
  total = subs + ui->ppnSpinBox->value() - ui->discountSpinBox->value();
  ui->bayarSpinBox->setMinimum(total);
  ui->labelTotal->setText(locale().toString(total));
  ui->labelKembalian->setText(
      locale().toString(qAbs(total - ui->bayarSpinBox->value())));
}

void InstantOrderDialog::setCustomerRecord(const QSqlRecord& record) {
  customerSet.name = record.value("nama_lengkap").toString();
  customerSet.id = record.value("id").toInt();
  customerSet.phone = record.value("nomor_telp").toString();
  customerSet.price_level = record.value("pl_id").toInt();
  ui->nameLineEdit->setText(customerSet.name);
  ui->phoneLineEdit->setText(customerSet.phone);
  auto model = ui->lHargaComboBox->model();
  auto indexes = model->match(model->index(0, 0), Qt::DisplayRole,
                              customerSet.price_level, 1, Qt::MatchExactly);
  if (indexes.count()) {
    ui->lHargaComboBox->setCurrentIndex(indexes.at(0).row());
  };

  auto hdr = omod.header();
  hdr.customer_name = customerSet.name;
  hdr.customer_phone = customerSet.phone;
  hdr.customer_id = customerSet.id;
  hdr.price_level_id = customerSet.price_level;
  omod.setHeaderField(hdr);
}

bool InstantOrderDialog::checkInput() {
  if (ui->nameLineEdit->text().isEmpty()) {
    QMessageBox::warning(this, "Periksa Input",
                         "Anda belum mengisi nama konsumen");
    ui->nameLineEdit->setFocus(Qt::OtherFocusReason);
    return false;
  }
  if (ui->phoneLineEdit->text().isEmpty()) {
    QMessageBox::warning(this, "Periksa Input",
                         "Anda belum kontak telepon konsumen");
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

void InstantOrderDialog::on_nameLineEdit_textChanged(const QString& txt) {
  auto hdr = omod.header();
  if (customerSet.name != txt) {
    hdr.customer_name = txt;
    hdr.customer_id = -1;
  } else {
    hdr.customer_name = customerSet.name;
    hdr.customer_id = customerSet.id;
  }
  omod.setHeaderField(hdr);
}

void InstantOrderDialog::on_phoneLineEdit_textChanged(const QString& txt) {
  auto hdr = omod.header();
  hdr.customer_phone = txt;
  omod.setHeaderField(hdr);
}

void InstantOrderDialog::on_lHargaComboBox_currentIndexChanged(int index) {
  auto hdr = omod.header();
  hdr.price_level_id = ui->lHargaComboBox->model()
                           ->index(index, 0)
                           .data()
                           .toInt();
  omod.setHeaderField(hdr);
}

void InstantOrderDialog::on_bayarButton_clicked() {
  ButtonGuard guard(ui->bayarButton);
  if (!checkInput()) return;
  InstantOrderController ioc;
  auto inv_code = ui->labelInvoiceCode->text();
  auto p_amount = calculatedSubtotal();
  auto p_disc = ui->discountSpinBox->value();

  QVariantMap payments{
      {"payment_amount", p_amount - p_disc},
      {"tax_amount", ui->ppnSpinBox->value()},
      {"cash_received", ui->bayarSpinBox->value()},
      {"cash_change", ui->bayarSpinBox->value() -
                          (p_amount - p_disc)},  // Perbaikan logika kurang
      {"invoice_number", inv_code},
      {"name", ui->nameLineEdit->text()},
      {"phone", ui->phoneLineEdit->text()},  // Ambil langsung dari UI
      {"price_level", ui->lHargaComboBox->model()
                          ->index(ui->lHargaComboBox->currentIndex(), 0)
                          .data()
                          .toInt()}};
  QString errBuffer;

  auto oh = omod.header();
  oh.deadline_date = QDateTime::currentDateTimeUtc();
  oh.completion_date = QDateTime::currentDateTimeUtc();
  oh.discount_amount = ui->discountSpinBox->value();
  omod.setHeaderField(oh);
  if(!ioc.create(oh, &omod, payments, &errBuffer)) {
    QMessageBox::warning(this, "Periksa Input", errBuffer);
    return ;
  }
  accept();
}

int InstantOrderDialog::calculatedSubtotal() const {
  int subs = 0;
  for (int r = 0; r < omod.rowCount(); ++r) {
    subs += omod.itemAt(r).total();
  }
  return subs;
}