#include "orderdialog.h"

#include <QAction>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QSqlTableModel>
#include <QTimer>
#include <cmath>

#include "src/customs/buttonguard.h"
#include "src/customs/flexibledelegate.h"
#include "src/customs/orderitemdelegate.h"
#include "src/dialogs/customerpickerdialog.h"
#include "src/dialogs/orderitemdialog.h"
#include "src/managers/basemanager.h"
#include "src/utils/helper.h"
#include "ui_orderdialog.h"

namespace {
const QHash<int, QString> Column{
    {0, "id"},
    {1, "order_id"},
    {2, "product_id"},
    {3, "product_name"},
    {4, "sku"},
    {5, "quantity"},
    {6, "unit"},
    {7, "size_width"},
    {8, "size_height"},
    {9, "use_area"},
    {10, "sale_price"},
    {11, "base_price"},
    {12, "discount_percentage"},
    {13, "discount_amount"},
    {14, "subtotal"},
    {15, "notes"},
    {16, "created_at"},
    {17, "updated_at"},
};

class ProductDelegate : public QStyledItemDelegate {
 public:
  explicit ProductDelegate(QObject* parent = nullptr)
      : QStyledItemDelegate(parent), m_productModel(new QSqlTableModel(this)) {
    m_productModel->setTable("products");
    m_productModel->select();
  }

  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                        const QModelIndex& index) const override {
    auto editor = new QueryComboBox(parent);
    editor->setQuery(
        "SELECT id, name, description, use_area, cost_price FROM products");
    editor->showColumn(3, false);  // Sembunyikan kolom use_area
    editor->showColumn(4, false);  // Sembunyikan kolom cost_price
    editor->setModelColumn(1);     // Tampilkan nama produk di combo box
    editor->setEditable(true);
    editor->boxViewAutoResize();
    return editor;
  }
  void setEditorData(QWidget* editor, const QModelIndex& index) const override {
    auto combo = qobject_cast<QueryComboBox*>(editor);
    if (combo) {
      int productId = index.data().toInt();
      int currentIndex = combo->findValue(productId);
      combo->setCurrentIndex(currentIndex);
    }
  }
  void setModelData(QWidget* editor, QAbstractItemModel* model,
                    const QModelIndex& index) const override {
    auto combo = qobject_cast<QueryComboBox*>(editor);
    if (combo) {
      auto qmod = combo->model();
      int selectedRow = combo->currentIndex();
      if (selectedRow >= 0) {
        int productId = qmod->index(selectedRow, 0).data().toInt();
        QString productName = qmod->index(selectedRow, 1).data().toString();
        model->setData(index, productId);  // Simpan product_id di model
        // model->setData(index.siblingAtColumn(3), productName); // Simpan nama
        // produk di kolom tersembunyi
      }
    }
  }
  QString displayText(const QVariant& value,
                      const QLocale& locale) const override {
    // Tampilkan nama produk berdasarkan product_id
    int productId = value.toInt();
    if (productId == 0) return "";  // Jika belum dipilih, tampilkan kosong
    int row = m_productModel
                  ->match(m_productModel->index(0, 0), Qt::DisplayRole,
                          productId, 1, Qt::MatchExactly)
                  .value(0)
                  .row();
    if (row >= 0) {
      return m_productModel->index(row, 2)
          .data()
          .toString();  // Tampilkan nama produk
    }
    return QString("Unknown Product (ID: %1)").arg(productId);
  }

 private:
  QSqlTableModel* m_productModel;
};

void disableSignalAndSet(
    std::variant<QLineEdit*, QSpinBox*, QDoubleSpinBox*, QPlainTextEdit*>
        editor,
    const QVariant& value) {
  std::visit(
      [&value](auto* editor) {
        using T = std::decay_t<decltype(*editor)>;
        editor->blockSignals(true);
        if constexpr (std::is_same_v<T, QLineEdit>)
          editor->setText(value.toString());
        else if constexpr (std::is_same_v<T, QDoubleSpinBox>)
          editor->setValue(value.toDouble());
        else if constexpr (std::is_same_v<T, QSpinBox>)
          editor->setValue(value.toInt());
        else if constexpr (std::is_same_v<T, QPlainTextEdit>)
          editor->setPlainText(value.toString());
        editor->blockSignals(false);
      },
      editor);
}
}  // namespace

OrderDialog::OrderDialog(QWidget* p)
    : QDialog(p), ui(new Ui::OrderDialog), m_model(new OrderModel(this)) {
  ui->setupUi(this);
  ui->orderItemList->setModel(m_model);
  ui->orderItemList->setItemDelegate(new OrderItemDelegate(this));
  ui->orderNumberLineEdit->setText(oman.nextNumber());
  auto crDate = QDateTime::currentDateTime();
  ui->tOrderDateTimeEdit->setDateTime(crDate);
  ui->dLineDateTimeEdit->setDateTime(crDate.addDays(5));

  // Subtotal and total are computed fields – prevent user editing them,
  // which would otherwise fire valueChanged and create signal loops.
  ui->subtotalSpinBox->setReadOnly(true);
  ui->totalSpinBox->setReadOnly(true);

  connect(m_model, &OrderModel::orderTotalChanged, this,
          [this](int) { updateCalculation(); });
}

OrderDialog::~OrderDialog() { delete ui; }

void OrderDialog::addOrderItem(const OrderItem& oi) {
  auto ix = m_model->addItem(oi);
}

void OrderDialog::on_diskonDoubleSpinBox_valueChanged(double percent) {
  if (ui->subtotalSpinBox->value() <= 0) {
    disableSignalAndSet(ui->diskonRpSpinBox, 0);
    QTimer::singleShot(0, this, [this]() { updateCalculation(); });
    return;
  }

  double subtotal = ui->subtotalSpinBox->value();
  double disc_exact = subtotal * percent / 100.0;
  // Round UP to nearest 100
  int disc_amount = static_cast<int>(std::ceil(disc_exact / 100.0)) * 100;

  // Block re-entry: setting diskonRpSpinBox would fire
  // on_diskonRpSpinBox_valueChanged
  disableSignalAndSet(ui->diskonRpSpinBox, disc_amount);

  QTimer::singleShot(0, this, [this]() { updateCalculation(); });
}

void OrderDialog::on_diskonRpSpinBox_valueChanged(int disc_rupiah) {
  double subtotal = ui->subtotalSpinBox->value();
  if (subtotal <= 0) {
    disableSignalAndSet(ui->diskonDoubleSpinBox, 0.0);
    QTimer::singleShot(0, this, [this]() { updateCalculation(); });
    return;
  }

  // Hitung persentase secara akurat (tanpa pembulatan paksa)
  double percent = (static_cast<double>(disc_rupiah) / subtotal) * 100.0;

  // Optional: bulatkan persen ke 2 desimal jika diinginkan
  percent = qRound(percent * 100.0) / 100.0;

  disableSignalAndSet(ui->diskonDoubleSpinBox, percent);
  QTimer::singleShot(0, this, [this]() { updateCalculation(); });
}

void OrderDialog::on_cariButton_clicked() {
  // buat dialog pencarian konsumen (CustomerSearchDialog)
  // setelah konsumen dipilih, set nama dan kontak di form ini
  auto dialog = new CustomerPickerDialog(this);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setWindowFlags(dialog->windowFlags() | Qt::FramelessWindowHint);
  if (!dialog->availableCustomers()) {
    dialog->deleteLater();
    QMessageBox::information(
        nullptr, "Selesai",
        "Tidak ditemukan konsumen yang memiliki order tanpa invoice");
    return;
  }
  auto buttonGeo = ui->cariButton->geometry();
  auto globalPos = mapToGlobal(buttonGeo.topRight());
  dialog->move(globalPos);
  connect(dialog, &CustomerPickerDialog::customerPicked, this,
          &OrderDialog::setCustomer);
  dialog->open();
}

void OrderDialog::setCustomer(const QSqlRecord& rc) {
  auto new_id = rc.value("id").toInt();
  if (new_id == 0) return;
  if (new_id == customerSet.id) return;

  if (customerSet.id > 0) {
    auto confirm = QMessageBox::question(
        this, "Ganti Pelanggan?",
        "Anda akan mengubah pelanggan yang telah disetel saat ini.\n"
        "Lanjutkan?",
        QMessageBox::Yes | QMessageBox::No);
    if (confirm == QMessageBox::No) return;
  }
  customerSet.id = new_id;
  customerSet.name = rc.value("nama_lengkap").toString();
  int priceLevelId = rc.value("pl_id").toInt();
  ui->priceLevelComboBox->setLevelID(priceLevelId);
  ui->kontakLineEdit->setText(rc.value("nomor_telp").toString());
  ui->konsumenLineEdit->setText(customerSet.name);
}

void OrderDialog::editOrderItemDialogFinished()
{
  OrderItemDialog *oid = qobject_cast<OrderItemDialog*>(sender());
  if(oid){
    auto er = oid->editResult();
    if(er.rowNumber > -1 && er.rowNumber < m_model->rowCount()) {
      if ( m_model->setItem(er.rowNumber, er.item) ) {
        updateCalculation();
        oid->deleteLater();
      }
    }
  }
}

void OrderDialog::updateCalculation() {
  auto subtotal = 0;
  for (int i = 0; i < m_model->rowCount(); ++i) {
    subtotal += m_model->itemAt(i).total();
  }
  ui->subtotalSpinBox->setValue(subtotal);
  ui->totalSpinBox->setValue(subtotal - ui->diskonRpSpinBox->value());
}

void OrderDialog::on_orderItemList_customContextMenuRequested(
    const QPoint& pos) {
  QMenu contextMenu;
  contextMenu.addAction(ui->tambahItem);
  auto ix = ui->orderItemList->indexAt(pos);
  if (ix.isValid()) {
    int row = ix.row();

    auto edit = contextMenu.addAction("Edit");
    connect(edit, &QAction::triggered, this, [this, row]() {
      auto editor = new OrderItemDialog(this);
      // editor->setAttribute(Qt::WA_DeleteOnClose);
      auto priceLevel = ui->priceLevelComboBox->currentId();
      editor->setCustomerPriceLevel(priceLevel);
      // Pass a mutable copy; on accept we replace the item in the model
      // OrderItem item = m_model->itemAt(row);
      // auto& item = m_model->itemRef(row);
      auto item = m_model->itemCopy(row);
      editor->setOrder(item, row);
      connect(editor, &OrderItemDialog::editFinished, this,
              &OrderDialog::editOrderItemDialogFinished);
      editor->open();
    });

    auto del = contextMenu.addAction("Hapus");
    connect(del, &QAction::triggered, this, [this, row]() {
      auto res = QMessageBox::question(this, "Hapus Item",
                                       "Yakin ingin menghapus item ini?",
                                       QMessageBox::Yes | QMessageBox::No);
      if (res == QMessageBox::Yes) {
        m_model->removeItem(row);
        QTimer::singleShot(0, this, [this]() { updateCalculation(); });
      }
    });
  }
  contextMenu.exec(ui->orderItemList->viewport()->mapToGlobal(pos));
}

void OrderDialog::on_tambahItem_triggered() {
  auto editor = new OrderItemDialog(this);
  editor->setAttribute(Qt::WA_DeleteOnClose);
  // get level harga pelanggan
  auto priceLevel = ui->priceLevelComboBox->currentId();
  editor->setCustomerPriceLevel(priceLevel);
  connect(editor, &OrderItemDialog::itemCreated, this,
          &OrderDialog::addOrderItem);
  editor->open();
}

void OrderDialog::on_simpanButton_clicked() {
  ButtonGuard guard(ui->simpanButton);
  if (ui->konsumenLineEdit->text().isEmpty()) {
    ui->konsumenLineEdit->setStyleSheet(
        "background-color: rgb(255, 200, 250);");
    QMessageBox::warning(this, "Peringatan",
                         "Nama konsumen tidak boleh kosong");
    ui->konsumenLineEdit->setFocus(Qt::ActiveWindowFocusReason);
    QTimer::singleShot(500,
                       [this] { ui->konsumenLineEdit->setStyleSheet(""); });
    return;
  }

  if (m_model->rowCount() == 0) {
    QMessageBox::warning(this, "Peringatan",
                         "Tambahkan minimal satu item pesanan");
    return;
  }

  // Build the order header from the form fields
  OrderHeader header;
  header.admin_id = 1;  // oman.currentAdminId();
  header.order_number = ui->orderNumberLineEdit->text();
  header.customer_id = customerSet.id > 0 ? customerSet.id : -1;
  header.customer_name = ui->konsumenLineEdit->text().trimmed();
  header.customer_phone = ui->kontakLineEdit->text().trimmed();
  header.price_level_id = ui->priceLevelComboBox->currentId();
  header.discount_amount = ui->diskonRpSpinBox->value();
  header.discount_percentage =
      static_cast<int>(ui->diskonDoubleSpinBox->value());
  // header.tax_amount         = ui->pajakRpSpinBox->value();
  header.order_date = ui->tOrderDateTimeEdit->dateTime().toUTC();
  header.deadline_date = ui->dLineDateTimeEdit->dateTime().toUTC();
  // header.status             = "pending";
  header.priority = "normal";
  // header.payment_status     = "unpaid";
  header.notes = ui->catatan1TextEdit->toPlainText();
  header.internal_notes = ui->catatan2TextEdit->toPlainText();

  if (m_model->orderId() == -1) {
    m_model->setHeader(header);
  } else {
    m_model->setHeaderField(header);
  }

  auto db = BaseManager::connection;
  if (!m_model->commit(db)) {
    QMessageBox::critical(this, "Gagal",
                          "Gagal menyimpan pesanan ke database.");
    return;
  }

  emit orderCreated(m_model->orderId());
  accept();
}
