#include "orderitemdialog.h"

#include <QMenu>
#include <QMessageBox>
#include <QStyledItemDelegate>
#include <QTimer>
#include <cmath>
#include <memory>

#include "finishingdialog.h"
#include "src/customs/buttonguard.h"
#include "src/customs/finishingitemdelegate.h"
#include "src/dialogs/productpickerdialog.h"
#include "src/models/finishinglistmodel.h"
#include "src/models/ordermodel.h"
#include "ui_orderitemdialog.h"

void debugMap(const QVariantMap&);

namespace {
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
constexpr int cColId = 0;
constexpr int cColArea = 3;
constexpr int cColCost = 4;
constexpr int cColSku = 5;
}  // namespace

OrderItemDialog::OrderItemDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::OrderItemDialog), m_mode(Create), m_itemEdit{} {
  ui->setupUi(this);
  setupProdukComboBox();
  setupFinishingView();
}

void OrderItemDialog::setupProdukComboBox() {
  ui->produkComboBox->setQuery(R"-(
    SELECT id, 
          name, 
          description, 
          use_area, 
          cost_price,
          sku
      FROM products
    WHERE is_active = 1
      ORDER BY name ASC;
  )-");
  ui->produkComboBox->showColumn(cColId, false);  // Sembunyikan kolom id
  ui->produkComboBox->showColumn(cColArea,
                                 false);  // Sembunyikan kolom use_area
  ui->produkComboBox->showColumn(cColCost,
                                 false);  // Sembunyikan kolom cost_price
  ui->produkComboBox->showColumn(cColSku, false);  // Sembunyikan kolom sku
  ui->produkComboBox->boxViewAutoResize();
  ui->produkComboBox->setCurrentIndex(-1);
}

OrderItemDialog::~OrderItemDialog() { delete ui; }

void OrderItemDialog::setOrder(OrderItem order, int row)
{
  m_mode = Modify;
  m_itemEdit = order;
  m_rowEdit = row;
  const QList<QWidget*> inputs{ui->namaLineEdit,    ui->qtySpinBox,
                               ui->hargaSpinBox,    ui->diskonDoubleSpinBox,
                               ui->diskonRpSpinBox, ui->widthBox,
                               ui->heightBox,       ui->notesTextEdit,
                               ui->totalSpinBox,    ui->produkComboBox};

  // get the combobox index
  for (auto* w : inputs) w->blockSignals(true);

  int currentProductId = m_itemEdit.product_id;
  auto prmodel = ui->produkComboBox->model();
  auto indexes = prmodel->match(prmodel->index(0, 0), Qt::DisplayRole,
                                currentProductId, 1, Qt::MatchExactly);
  if (indexes.isEmpty()) return;
  auto currentIndex = indexes.first().row();
  ui->produkComboBox->setCurrentIndex(currentIndex);
  ui->namaLineEdit->setText(m_itemEdit.product_name);

  if (!m_itemEdit.use_area) {
    ui->heightBox->setValue(1);
    ui->heightBox->setEnabled(false);
    ui->widthBox->setValue(1);
    ui->widthBox->setEnabled(false);
  } else {
    ui->heightBox->setValue(m_itemEdit.size_height);
    ui->heightBox->setEnabled(true);
    ui->widthBox->setValue(m_itemEdit.size_width);
    ui->widthBox->setEnabled(true);
  }
  ui->hargaSpinBox->setValue(m_itemEdit.sale_price);
  ui->diskonDoubleSpinBox->setValue(m_itemEdit.discount_percentage);
  ui->diskonRpSpinBox->setValue(m_itemEdit.discount_amount);
  ui->notesTextEdit->setPlainText(m_itemEdit.notes);
  m_finishingListModel.setList(&m_itemEdit.finishings);

  for (auto* w : inputs) w->blockSignals(false);

  // Now do one clean recalculation with all values in place.
  ui->qtySpinBox->setValue(m_itemEdit.quantity);
}

OrderItemDialog::OrderItemEditResult OrderItemDialog::editResult() const
{
    return OrderItemEditResult( m_itemEdit, m_rowEdit );
}

void OrderItemDialog::resetForm() { ui->produkComboBox->setCurrentIndex(-1); }

void OrderItemDialog::on_simpanButton_clicked() {
  ButtonGuard guard(ui->simpanButton);
  // validasi ui->produkComboBox harus >= 0
  if (ui->produkComboBox->currentIndex() < 0) {
    QMessageBox::warning(this, "Validasi", "Produk harus dipilih");
    return;
  }
  // validasi nama tidak boleh kosong
  if (ui->namaLineEdit->text().trimmed().isEmpty()) {
    QMessageBox::warning(this, "Validasi", "Nama produk tidak boleh kosong");
    return;
  }

  auto index = ui->produkComboBox->model()->index(
      ui->produkComboBox->currentIndex(), 0);
  auto opt_pr = m_productManager.getById(index.siblingAtColumn(0).data().toInt());
  if (!opt_pr) {
    QMessageBox::warning(this, "Validasi", "Produk tidak ditemukan");
    return;
  }
  OrderItem oi = buildOrderItemFromUi(*opt_pr);
  if (m_mode == Create) {
    emit itemCreated(oi);
  } else {
    m_itemEdit = oi;
    emit editFinished();
  }
  accept();
}

void OrderItemDialog::setupFinishingView() {
  m_finishingListModel.setList(&m_newFinishingItems);
  ui->finishingView->setModel(&m_finishingListModel);
  ui->finishingView->setItemDelegate(new FinishingItemDelegate(this));

  // Context menu for finishing view: edit and remove
  ui->finishingView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->finishingView, &QWidget::customContextMenuRequested, this,
          &OrderItemDialog::on_finishingView_customContextMenuRequested);
  connect(&m_finishingListModel, &QAbstractListModel::rowsInserted, this,
          &OrderItemDialog::recalculateSubtotal);
  connect(&m_finishingListModel, &QAbstractListModel::rowsRemoved, this,
          &OrderItemDialog::recalculateSubtotal);
}

OrderItem OrderItemDialog::buildOrderItemFromUi(
  const QSqlRecord& rec) const {
  OrderItem oi;
  oi.product_id = rec.value("id").toInt();
  oi.product_name = ui->namaLineEdit->text().trimmed();
  oi.sku = rec.value("sku").toString();
  oi.quantity = ui->qtySpinBox->value();
  oi.unit = rec.value("unit").toString();
  oi.use_area = rec.value("use_area").toBool();
  oi.size_width = oi.use_area ? ui->widthBox->value() : 1.0;
  oi.size_height = oi.use_area ? ui->heightBox->value() : 1.0;
  oi.sale_price = ui->hargaSpinBox->value();
  oi.base_price = rec.value("cost_price").toInt();
  oi.discount_percentage = ui->diskonDoubleSpinBox->value();
  oi.discount_amount = ui->diskonRpSpinBox->value();
  oi.finishing_total = m_finishingListModel.total();
  oi.notes = ui->notesTextEdit->toPlainText();

  if (m_mode == Create) {
    oi.finishings.clear();
    auto p = m_finishingListModel.getItems();
    for (auto const fp : p) {
      oi.finishings << fp;
    }
  }

  return oi;
}

void OrderItemDialog::on_produkComboBox_currentIndexChanged(int index) {
  if (index < 0) {
    ui->hargaSpinBox->setMinimum(0);
    ui->hargaSpinBox->setValue(0);
    return;
  }
  auto model = ui->produkComboBox->model();
  bool use_area = model->index(index, 3).data(Qt::EditRole).toBool();
  if (use_area) {
    ui->widthBox->setEnabled(true);
    ui->heightBox->setEnabled(true);
    ui->widthBox->setMinimum(0.01);
    ui->heightBox->setMinimum(0.01);
    ui->widthBox->setValue(1);
    ui->heightBox->setValue(1);
  } else {
    ui->widthBox->setEnabled(false);
    ui->heightBox->setEnabled(false);
    ui->widthBox->setValue(1);
    ui->heightBox->setValue(1);
  }

  // In Modify mode the price is already set correctly; don't override it.
  if (m_mode == Modify) {
    auto ans =
        QMessageBox::question(this, "Terdeteksi perubahan pada jenis produk",
                              "Sesuaikan harga dengan harga produk baru ?");
    if (ans != QMessageBox::Yes) return;
  }

  int productId = model->index(index, 0).data(Qt::EditRole).toInt();
  auto optprice = m_priceManager.getPrice(productId, m_customerPriceLevel);
  int suggestedPrice = 0;
  if (optprice.has_value()) {
    suggestedPrice = optprice->value("price").toInt();
  } else {
    // Fallback to cost_price column (index 4 in the query)
    suggestedPrice = model->index(index, 4).data(Qt::EditRole).toInt();
  }

  // Allow price=0 floor for negotiated (price level 3), otherwise use
  // suggested.
  int minPrice = (m_customerPriceLevel == 3) ? 0 : suggestedPrice;
  ui->hargaSpinBox->setMinimum(minPrice);
  ui->hargaSpinBox->setValue(suggestedPrice);
}

void OrderItemDialog::on_hargaSpinBox_valueChanged(int arg1) {
  recalculateSubtotal();
}

void OrderItemDialog::on_qtySpinBox_valueChanged(int arg1) {
  recalculateSubtotal();
}

void OrderItemDialog::on_diskonDoubleSpinBox_valueChanged(double arg1) {
  int calculatedSubtotal = calculatedPrice();
  double diskonRp = calculatedSubtotal * (arg1 / 100.0);
  // Round to nearest 100
  diskonRp = qRound(diskonRp / 100.0) * 100.0;
  // FIX: block signals to avoid triggering on_diskonRpSpinBox_valueChanged
  // which would re-enter and overwrite the percentage we just received.
  disableSignalAndSet(ui->diskonRpSpinBox, static_cast<int>(diskonRp));
  recalculateSubtotal();
}

void OrderItemDialog::on_diskonRpSpinBox_valueChanged(int arg1) {
  int calcullatedSubtotal = calculatedPrice();
  // FIX: guard against division by zero
  if (calcullatedSubtotal <= 0) {
    disableSignalAndSet(ui->diskonDoubleSpinBox, 0.0);
    recalculateSubtotal();
    return;
  }
  double diskonPersen =
      (static_cast<double>(arg1) / static_cast<double>(calcullatedSubtotal)) *
      100.0;
  disableSignalAndSet(ui->diskonDoubleSpinBox, diskonPersen);
  recalculateSubtotal();
}

void OrderItemDialog::recalculateSubtotal() {
  int subtotal = calculatedPrice();
  ui->diskonRpSpinBox->setMaximum(subtotal);
  int diskonRp = ui->diskonRpSpinBox->value();
  int total = subtotal - diskonRp;
  ui->totalSpinBox->setValue(total);
}

int OrderItemDialog::calculatedPrice() const {
  if (ui->produkComboBox->currentIndex() < 0) {
    return 0;
  }
  auto model = ui->produkComboBox->model();
  int qty = ui->qtySpinBox->value();
  double width = ui->widthBox->value();
  double height = ui->heightBox->value();
  double harga = ui->hargaSpinBox->value();
  // FIX: actually use the use_area flag to decide the multiplier
  bool use_area = model->index(ui->produkComboBox->currentIndex(), 3)
                      .data(Qt::EditRole)
                      .toBool();
  double areaMultiplier =
      (use_area && width > 0 && height > 0) ? (width * height) : 1.0;
  int pr = qCeil((harga * areaMultiplier * qty) * 100.0) / 100;
  return pr + m_finishingListModel.total();
}

void OrderItemDialog::on_tambahButton_clicked() {
  auto fd = new FinishingDialog(this);
  connect(fd, &FinishingDialog::createItem, this,
          &OrderItemDialog::onCreateFinishing);
  fd->setAttribute(Qt::WA_DeleteOnClose);
  fd->open();
}

void OrderItemDialog::onCreateFinishing(const FinishingItem& item) {
  m_finishingListModel.addItem(item);
  // Recalculate: calculatedPrice() includes m_finishingListModel.total()
  QTimer::singleShot(0, this, &OrderItemDialog::recalculateSubtotal);
}

// handle editFinishing
void OrderItemDialog::onFinishingEdited(const FinishingItem& fi) {
  auto dl = qobject_cast<FinishingDialog*>(sender());
  if (!dl) return;
  auto row = dl->property("itemRowNumber").toInt();
  auto modelIx = m_finishingListModel.index(row, 0);
  m_finishingListModel.setData(modelIx, fi.finishing_id, Qt::UserRole + 3);
  m_finishingListModel.setData(modelIx, fi.finishing_name, Qt::UserRole + 4);
  m_finishingListModel.setData(modelIx, fi.quantity, Qt::UserRole + 5);
  m_finishingListModel.setData(modelIx, fi.finishing_price, Qt::UserRole + 6);
}

void OrderItemDialog::on_finishingView_customContextMenuRequested(
    const QPoint& pos) {
  auto ix = ui->finishingView->indexAt(pos);
  if (!ix.isValid()) return;
  int row = ix.row();
  QMenu menu;
  auto editAct = menu.addAction("Edit");
  auto delAct = menu.addAction("Hapus");
  auto chosen = menu.exec(ui->finishingView->viewport()->mapToGlobal(pos));
  if (chosen == editAct) {
    auto items = m_finishingListModel.getItems();
    // Use shared_ptr so the dialog's pointer and the accepted-lambda
    // both refer to the same FinishingItem instance.
    auto fi = items[row];
    auto fd = new FinishingDialog(this);
    fd->setAttribute(Qt::WA_DeleteOnClose);
    fd->setItem(fi);
    fd->setProperty("itemRowNumber", row);
    connect(fd, &FinishingDialog::itemModified, this,
            &OrderItemDialog::onFinishingEdited);
    fd->open();
  } else if (chosen == delAct) {
    m_finishingListModel.removeItem(row);
  }
}

void OrderItemDialog::on_pilihButton_clicked() {
  ProductPickerDialog ppd(this);
  // ppd.setWindowFlag(Qt::FramelessWindowHint, true);
  connect(&ppd, &ProductPickerDialog::productPicked, this,
          &OrderItemDialog::setCurrentProduct);
  connect(&ppd, &ProductPickerDialog::productPicked, &ppd, &QDialog::accept);
  auto tr = ui->pilihButton->geometry().topRight();
  ppd.move(mapToGlobal(tr));
  ppd.exec();
}

void OrderItemDialog::setCurrentProduct(int p) {
  int prix = ui->produkComboBox->findIndex(p, 0);
  if (prix < 0) {
    QMessageBox::warning(this, "Kesalahan Internal",
                         "Tidak dapat menyetel produk");
    return;
  }
  ui->produkComboBox->setCurrentIndex(prix);
}