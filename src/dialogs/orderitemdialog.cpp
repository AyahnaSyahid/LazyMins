#include "orderitemdialog.h"
#include "ui_orderitemdialog.h"

#include "finishingdialog.h"
#include "src/models/finishinglistmodel.h"
#include "src/models/ordermodel.h"
#include <QMessageBox>
#include <QTimer>

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
    ui(new Ui::OrderItemDialog),
    m_mode(Create),
    QDialog(parent)
{
    ui->setupUi(this);
    ui->produkComboBox->setQuery("SELECT id, name, description, use_area, cost_price FROM products");
    ui->produkComboBox->showColumn(0, false); // Sembunyikan kolom id
    ui->produkComboBox->showColumn(3, false); // Sembunyikan kolom use_area
    ui->produkComboBox->showColumn(4, false); // Sembunyikan kolom cost_price
    ui->produkComboBox->boxViewAutoResize();
    ui->produkComboBox->setCurrentIndex(-1);
    ui->finishingView->setModel(&m_finModel);
}

OrderItemDialog::~OrderItemDialog()
{
    delete ui;
}

void OrderItemDialog::setOrder(OrderItem *order) {
  // initializeFrom *OrderItem
  ui->namaLineEdit->setText(order->product_name);
  if(!order->use_area) {
    ui->heightBox->setValue(1);
    ui->heightBox->setEnabled(false);
    ui->widthBox->setValue(1);
    ui->widthBox->setEnabled(false);
  } else {
    ui->heightBox->setValue(order->size_width);
    ui->heightBox->setEnabled(true);
    ui->widthBox->setValue(order->size_height);
    ui->widthBox->setEnabled(true);  
  }
  ui->qtySpinBox->setValue(order->quantity);
  ui->hargaSpinBox->setValue(order->sale_price);
  ui->diskonDoubleSpinBox->setValue(order->discount_percentage);
  ui->diskonRpSpinBox->setValue(order->discount_amount);
  ui->notesTextEdit->setPlainText(order->notes);
  ui->totalSpinBox->setValue(order->total());
  m_finModel->setItems(&order->finishings);
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
}

void OrderItemDialog::on_produkComboBox_currentIndexChanged(int index) {
    if (index < 0) {
        ui->hargaSpinBox->setMinimum(0);
        ui->hargaSpinBox->setValue(0);
        return;
    }
    auto model = ui->produkComboBox->model();
    auto use_area = model->index(index, 3).data(Qt::EditRole);
    if (use_area.toInt() == 1) { // jika produk menggunakan area, aktifkan input width & height
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
    // dapatkan current product id
    int productId = model->index(index, 0).data(Qt::EditRole).toInt();
    
    // dapatkan harga berdasarkan price level customer
    auto optprice = m_priceManager.getPrice(productId, m_customerPriceLevel);
    if (optprice.has_value()) {
        ui->hargaSpinBox->setMinimum(*optprice);
        ui->hargaSpinBox->setValue(*optprice);
    } else {
        // fallback ke harga dasar dari tabel products
        int basePrice = model->index(index, 4).data(Qt::EditRole).toInt();
        ui->hargaSpinBox->setMinimum(basePrice);
        ui->hargaSpinBox->setValue(basePrice);
    }

    // diperlukan mekanisme untuk handle by nego
    if (m_customerPriceLevel == 3) {
        ui->hargaSpinBox->setMinimum(0);
    }
}

void OrderItemDialog::on_hargaSpinBox_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    recalculateSubtotal();
}

void OrderItemDialog::on_qtySpinBox_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    recalculateSubtotal();
}

void OrderItemDialog::on_diskonDoubleSpinBox_valueChanged(double arg1)
{
    int calcullatedSubtotal = calculatedPrice();
    double diskonRp = calcullatedSubtotal * (arg1 / 100.0);
    // diskon Rp harus dibulatkan menjadi kelipatan 100
    diskonRp = qRound(diskonRp / 100.0) * 100.0;
    ui->diskonRpSpinBox->setValue(diskonRp);
}

void OrderItemDialog::on_diskonRpSpinBox_valueChanged(int arg1)
{
    // kalkulasi berapa persen diskon
    int calcullatedSubtotal = calculatedPrice();
    double diskonPersen = (arg1 / calcullatedSubtotal) * 100.0;
    disableSignalAndSet(ui->diskonDoubleSpinBox, diskonPersen);
    recalculateSubtotal();
}

void OrderItemDialog::recalculateSubtotal()
{
    // diskonPersen dan diskonRp adalah entitas yang sama, 
    // jika user mengatur diskonPersen maka diskonRp harus dihitung ulang, begitu pula sebaliknya. 
    // Untuk menyederhanakan, kita asumsikan user hanya akan mengatur salah satu jenis diskon, dan kita prioritaskan diskonRp jika keduanya diisi.
    // double diskonPersen = ui->diskonDoubleSpinBox->value();
    int subtotal = calculatedPrice();
    ui->diskonRpSpinBox->setMaximum(subtotal);
    double diskonRp = ui->diskonRpSpinBox->value();
    subtotal -= diskonRp; // Diskon nominal
    ui->subtotalSpinBox->setValue(subtotal);
}

int OrderItemDialog::calculatedPrice() const
{
    if (ui->produkComboBox->currentIndex() < 0) {
        return 0.0;
    }
    auto model = ui->produkComboBox->model();
    int qty = ui->qtySpinBox->value();
    double width = ui->widthBox->value();
    double height = ui->heightBox->value();
    double harga = ui->hargaSpinBox->value();
    auto use_area = model->index(ui->produkComboBox->currentIndex(), 3).data(Qt::EditRole);
    double areaMultiplier = (width > 0 && height > 0) ? (width * height) : 1.0;
    auto pr = qCeil((harga * areaMultiplier * qty) / 100.0) * 100;
    return pr + m_finModel.total();
}

void OrderItemDialog::on_tambahButton_clicked() {
  auto fd = new FinishingDialog(this);
  connect(fd, &FinishingDialog::createItem, this, &OrderItemDialog::onCreateFinishing);
  fd->setAttribute(Qt::WA_DeleteOnClose);
  fd->open();
}

// handle createFinishing
void OrderItemDialog::onCreateFinishing(const FinishingItem& item) {
  m_finModel.addItem(item);
}

// handle editFinishing
void OrderItemDialog::onFinishingAccepted() {
  QTimer::singleShot(0, [this](){ recalculateSubtotal(); });
}