#include "orderitemdialog.h"
#include "ui_orderitemdialog.h"

#include "finishingdialog.h"
#include "src/models/finishinglistmodel.h"
#include "src/customs/finishingitemdelegate.h"
#include "src/models/ordermodel.h"
#include "src/dialogs/productpickerdialog.h"
#include <QMessageBox>
#include <QMenu>
#include <QStyledItemDelegate>
#include <QTimer>
#include <memory>
#include <cmath>

void debugMap(const QVariantMap& );

namespace {
  void disableSignalAndSet(std::variant<QLineEdit*, QSpinBox*, QDoubleSpinBox*, QPlainTextEdit*> editor, const QVariant &value) {
    std::visit([&value](auto* editor) {
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
        }, editor);
    }
  
}

OrderItemDialog::OrderItemDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OrderItemDialog),
    m_mode(Create)
{
    ui->setupUi(this);
    ui->produkComboBox->setQuery("SELECT id, name, description, use_area, cost_price, sku FROM products");
    ui->produkComboBox->showColumn(0, false); // Sembunyikan kolom id
    ui->produkComboBox->showColumn(3, false); // Sembunyikan kolom use_area
    ui->produkComboBox->showColumn(4, false); // Sembunyikan kolom cost_price
    ui->produkComboBox->showColumn(5, false); // Sembunyikan kolom sku
    ui->produkComboBox->boxViewAutoResize();
    ui->produkComboBox->setCurrentIndex(-1);
    m_finishingListModel.setList(&m_newFinishingItems);
    ui->finishingView->setModel(&m_finishingListModel);
    ui->finishingView->setItemDelegate(new FinishingItemDelegate(this));

    // Context menu for finishing view: edit and remove
    ui->finishingView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->finishingView, &QWidget::customContextMenuRequested, this, &OrderItemDialog::on_finishingView_customContextMenuRequested);
    connect(&m_finishingListModel, &QAbstractListModel::rowsInserted, this, &OrderItemDialog::recalculateSubtotal);
    connect(&m_finishingListModel, &QAbstractListModel::rowsRemoved, this, &OrderItemDialog::recalculateSubtotal);
}

OrderItemDialog::~OrderItemDialog()
{
    delete ui;
}

void OrderItemDialog::setOrder(OrderItem *order) {
  m_mode = Modify;
  m_orderItem = order;

  // Block all input signals while we populate – avoids re-entrant
  // recalculation triggered by individual setValue/setText calls.
  const QList<QWidget*> inputs {
    ui->namaLineEdit, ui->qtySpinBox, ui->hargaSpinBox,
    ui->diskonDoubleSpinBox, ui->diskonRpSpinBox,
    ui->widthBox, ui->heightBox, ui->notesTextEdit, ui->totalSpinBox,
    ui->produkComboBox
  };

  // get the combobox index
  for (auto *w : inputs) w->blockSignals(true);
  
  int currentProductId = order->product_id;
  qDebug() << "Current Product ID:" << currentProductId;
  auto prmodel = ui->produkComboBox->model();
  auto indexes = prmodel->match(prmodel->index(0, 0), Qt::DisplayRole, currentProductId, 1, Qt::MatchExactly);
  auto currentIndex = indexes.at(0).row();
  ui->produkComboBox->setCurrentIndex(currentIndex);
  
  
  ui->namaLineEdit->setText(order->product_name);
  if (!order->use_area) {
    ui->heightBox->setValue(1);
    ui->heightBox->setEnabled(false);
    ui->widthBox->setValue(1);
    ui->widthBox->setEnabled(false);
  } else {
    ui->heightBox->setValue(order->size_height);
    ui->heightBox->setEnabled(true);
    ui->widthBox->setValue(order->size_width);
    ui->widthBox->setEnabled(true);
  }
  ui->hargaSpinBox->setValue(order->sale_price);
  ui->diskonDoubleSpinBox->setValue(order->discount_percentage);
  ui->diskonRpSpinBox->setValue(order->discount_amount);
  ui->notesTextEdit->setPlainText(order->notes);
  m_finishingListModel.setList(&order->finishings);

  for (auto *w : inputs) w->blockSignals(false);
  // Now do one clean recalculation with all values in place.
  ui->qtySpinBox->setValue(order->quantity);
}

void OrderItemDialog::resetForm()
{
  ui->produkComboBox->setCurrentIndex(-1);
}

void OrderItemDialog::on_simpanButton_clicked()
{   
  // validasi ui->produkComboBox harus >= 0
  if (ui->produkComboBox->currentIndex() < 0) {
      QMessageBox::warning(this, "Validasi", "Produk harus dipilih");
      return;
  }
  
  //validasi nama tidak boleh kosong
  if (ui->namaLineEdit->text().trimmed().isEmpty()) {
      QMessageBox::warning(this, "Validasi", "Nama produk tidak boleh kosong");
      return;
  }
  
  if (m_mode == Create) {
    auto index = ui->produkComboBox->model()->index(ui->produkComboBox->currentIndex(), 0);
    auto opt_pr = m_productManager.getById(index.siblingAtColumn(0).data().toInt());
    if (opt_pr) {
      auto pr = *opt_pr;
      OrderItem oi;
      oi.product_id = pr.value("id").toInt();
      oi.product_name = ui->namaLineEdit->text().trimmed();
      oi.sku = pr.value("sku").toString();
      oi.quantity = ui->qtySpinBox->value();
      oi.unit = pr.value("unit").toString();
      oi.use_area = pr.value("use_area").toBool();
      oi.size_width = oi.use_area ? ui->widthBox->value() : 1.0;
      oi.size_height = oi.use_area ? ui->heightBox->value() : 1.0;
      oi.sale_price = ui->hargaSpinBox->value();
      oi.base_price = pr.value("cost_price").toInt();
      oi.discount_percentage = ui->diskonDoubleSpinBox->value();
      oi.discount_amount = ui->diskonRpSpinBox->value();
      oi.finishing_total = m_finishingListModel.total();
      oi.notes = ui->notesTextEdit->toPlainText();
      auto p = m_finishingListModel.getItems();
      oi.finishings.clear();
      for(auto const fp : p) {
        oi.finishings << fp;
      }
      emit itemCreated(oi);
    }
  } else {
    // Edit mode: update the existing item in place
    if (m_orderItem) {
      auto index = ui->produkComboBox->model()->index(ui->produkComboBox->currentIndex(), 0);
      auto opt_pr = m_productManager.getById(index.siblingAtColumn(0).data().toInt());
      if (opt_pr) {
        auto pr = *opt_pr;
        m_orderItem->product_id          = pr.value("id").toInt();
        m_orderItem->product_name        = ui->namaLineEdit->text().trimmed();
        m_orderItem->sku                 = pr.value("sku").toString();
        m_orderItem->quantity            = ui->qtySpinBox->value();
        m_orderItem->unit                = pr.value("unit").toString();
        m_orderItem->use_area            = pr.value("use_area").toBool();
        m_orderItem->size_width          = m_orderItem->use_area ? ui->widthBox->value() : 1.0;
        m_orderItem->size_height         = m_orderItem->use_area ? ui->heightBox->value() : 1.0;
        m_orderItem->sale_price          = ui->hargaSpinBox->value();
        m_orderItem->base_price          = pr.value("cost_price").toInt();
        m_orderItem->discount_percentage = ui->diskonDoubleSpinBox->value();
        m_orderItem->discount_amount     = ui->diskonRpSpinBox->value();
        m_orderItem->finishing_total     = m_finishingListModel.total();
        m_orderItem->notes               = ui->notesTextEdit->toPlainText();
        emit editFinished();
      }
    }
  }
  accept();
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
      auto ans = QMessageBox::question(this, "Terdeteksi perubahan pada jenis produk", "Sesuaikan harga dengan harga produk baru ?");
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

    // Allow price=0 floor for negotiated (price level 3), otherwise use suggested.
    int minPrice = (m_customerPriceLevel == 3) ? 0 : suggestedPrice;
    ui->hargaSpinBox->setMinimum(minPrice);
    ui->hargaSpinBox->setValue(suggestedPrice);
}

void OrderItemDialog::on_hargaSpinBox_valueChanged(int arg1)
{
    recalculateSubtotal();
}

void OrderItemDialog::on_qtySpinBox_valueChanged(int arg1)
{
  recalculateSubtotal();
}

void OrderItemDialog::on_diskonDoubleSpinBox_valueChanged(double arg1)
{
    int calculatedSubtotal = calculatedPrice();
    double diskonRp = calculatedSubtotal * (arg1 / 100.0);
    // Round to nearest 100
    diskonRp = qRound(diskonRp / 100.0) * 100.0;
    // FIX: block signals to avoid triggering on_diskonRpSpinBox_valueChanged
    // which would re-enter and overwrite the percentage we just received.
    disableSignalAndSet(ui->diskonRpSpinBox, static_cast<int>(diskonRp));
    recalculateSubtotal();
}

void OrderItemDialog::on_diskonRpSpinBox_valueChanged(int arg1)
{
    int calcullatedSubtotal = calculatedPrice();
    // FIX: guard against division by zero
    if (calcullatedSubtotal <= 0) {
        disableSignalAndSet(ui->diskonDoubleSpinBox, 0.0);
        recalculateSubtotal();
        return;
    }
    double diskonPersen = (static_cast<double>(arg1) / static_cast<double>(calcullatedSubtotal)) * 100.0;
    disableSignalAndSet(ui->diskonDoubleSpinBox, diskonPersen);
    recalculateSubtotal();
}

void OrderItemDialog::recalculateSubtotal()
{
    int subtotal = calculatedPrice();
    ui->diskonRpSpinBox->setMaximum(subtotal);
    int diskonRp = ui->diskonRpSpinBox->value();
    int total = subtotal - diskonRp;
    ui->totalSpinBox->setValue(total);
}

int OrderItemDialog::calculatedPrice() const
{
    if (ui->produkComboBox->currentIndex() < 0) {
        return 0;
    }
    auto model = ui->produkComboBox->model();
    int qty = ui->qtySpinBox->value();
    double width  = ui->widthBox->value();
    double height = ui->heightBox->value();
    double harga  = ui->hargaSpinBox->value();
    // FIX: actually use the use_area flag to decide the multiplier
    bool use_area = model->index(ui->produkComboBox->currentIndex(), 3).data(Qt::EditRole).toBool();
    double areaMultiplier = (use_area && width > 0 && height > 0) ? (width * height) : 1.0;
    int pr = qCeil((harga * areaMultiplier * qty) * 100.0) / 100;
    return pr + m_finishingListModel.total();
}

void OrderItemDialog::on_tambahButton_clicked() {
  auto fd = new FinishingDialog(this);
  connect(fd, &FinishingDialog::createItem, this, &OrderItemDialog::onCreateFinishing);
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
  if(!dl) return;
  auto row = dl->property("itemRowNumber").toInt();
  auto modelIx = m_finishingListModel.index(row, 0);
  m_finishingListModel.setData(modelIx, fi.finishing_id,    Qt::UserRole + 3);
  m_finishingListModel.setData(modelIx, fi.finishing_name,  Qt::UserRole + 4);
  m_finishingListModel.setData(modelIx, fi.quantity,        Qt::UserRole + 5);
  m_finishingListModel.setData(modelIx, fi.finishing_price, Qt::UserRole + 6);
}

void OrderItemDialog::on_finishingView_customContextMenuRequested(const QPoint& pos) 
{
  auto ix = ui->finishingView->indexAt(pos);
  if (!ix.isValid()) return;
  int row = ix.row();
  QMenu menu;
  auto editAct = menu.addAction("Edit");
  auto delAct  = menu.addAction("Hapus");
  auto chosen  = menu.exec(ui->finishingView->viewport()->mapToGlobal(pos));
  if (chosen == editAct) {
    auto items = m_finishingListModel.getItems();
    // Use shared_ptr so the dialog's pointer and the accepted-lambda
    // both refer to the same FinishingItem instance.
    auto fi = items[row];
    auto fd = new FinishingDialog(this);
    fd->setAttribute(Qt::WA_DeleteOnClose);
    fd->setItem(fi);
    fd->setProperty("itemRowNumber", row);
    connect(fd, &FinishingDialog::itemModified, this, &OrderItemDialog::onFinishingEdited);
    fd->open();
  } else if (chosen == delAct) {
      m_finishingListModel.removeItem(row);
  }
}

void OrderItemDialog::on_pilihButton_clicked() {
  ProductPickerDialog ppd(this);
  // ppd.setWindowFlag(Qt::FramelessWindowHint, true);
  connect(&ppd, &ProductPickerDialog::productPicked, this, &OrderItemDialog::setCurrentProduct);
  connect(&ppd, &ProductPickerDialog::productPicked, &ppd, &QDialog::accept);
  auto tr = ui->pilihButton->geometry().topRight();
  ppd.move(mapToGlobal(tr));
  ppd.exec();
}

void OrderItemDialog::setCurrentProduct(int p) {
  int prix = ui->produkComboBox->findIndex(p, 0);
  if (prix < 0) {
    QMessageBox::warning(this, "Kesalahan Internal", "Tidak dapat menyetel produk");
    return ;
  }
  ui->produkComboBox->setCurrentIndex(prix);
}