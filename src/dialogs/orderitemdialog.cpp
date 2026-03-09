#include "orderitemdialog.h"
#include "ui_orderitemdialog.h"
#include <QMessageBox>

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
    m_autoCommit(true),
    FormDialog(parent)
{
    ui->setupUi(this);
    setupFields();
    ui->produkComboBox->setQuery("SELECT id, name, description, use_area, cost_price FROM products");
    ui->produkComboBox->showColumn(3, false); // Sembunyikan kolom use_area
    ui->produkComboBox->showColumn(4, false); // Sembunyikan kolom cost_price
    ui->produkComboBox->boxViewAutoResize();
    ui->produkComboBox->setCurrentIndex(-1);
}

OrderItemDialog::~OrderItemDialog()
{
    delete ui;
}

bool OrderItemDialog::onSave(const QVariantMap& changes) {
    return true;
}

void OrderItemDialog::setupFields() {
    setFields({
        { ui->namaLineEdit, "product_name" },
        { ui->hargaSpinBox, "sale_price" },
        { ui->qtySpinBox, "quantity" },
        { ui->subtotalSpinBox, "subtotal" },
        { ui->diskonDoubleSpinBox, "discount_percentage" },
        { ui->diskonRpSpinBox, "discount_amount" },
        { ui->notesTextEdit, "notes" },
        { ui->widthBox, "size_width" },
        { ui->heightBox, "size_height" }
    });
}

void OrderItemDialog::setupBoundFields() {
    addBoundField( "product_id",
            [&]() -> QVariant {
                return ui->produkComboBox->model()->index(ui->produkComboBox->currentIndex(), 0).data(Qt::EditRole);}, 
            [&](const QVariant& value) {
                int id = value.toInt();
                int index = ui->produkComboBox->findValue(id);
                ui->produkComboBox->setCurrentIndex(index);
            });
}

void OrderItemDialog::resetForm()
{
    for (const auto& f : m_fields) {
        std::visit([](auto* editor) {
            using T = std::decay_t<decltype(*editor)>;
            if constexpr (std::is_same_v<T, QLineEdit>)
                editor->clear();
            else if constexpr (std::is_same_v<T, QDoubleSpinBox>)
                editor->setValue(0);
            else if constexpr (std::is_same_v<T, QSpinBox>)
                editor->setValue(0);
            else if constexpr (std::is_same_v<T, QPlainTextEdit>)
                editor->clear();
        }, f.editor);
    }
    ui->produkComboBox->setCurrentIndex(-1);
}

QVariantMap OrderItemDialog::collect() const
{
    auto data = FormDialog::collect();
    // tambahkan field base_price yang diambil dari produkComboBox
    auto index = ui->produkComboBox->model()->index(ui->produkComboBox->currentIndex(), 0);
    auto costPrice = index.siblingAtColumn(4).data(Qt::EditRole).toInt();
    auto opt = m_priceManager.getPrice(index.data().toInt(), m_customerPriceLevel);
    data["base_price"] = costPrice;
    if (opt.has_value())
        data["base_price"] = *opt;
    
    int total, subtotal;
    total = data["subtotal"].toInt();
    subtotal = total + data["discount_amount"].toInt();
    
    data["total"] = total;
    data["subtotal"] = subtotal;
    auto ropt = m_productManager.getById(data["product_id"].toInt());
    if(ropt.has_value())
      data["use_area"] = (*ropt).value("use_area");
    
    return data;
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
    if (m_autoCommit) {
        accept();
        return;
    }
    emit editFinished(collect());
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
    double calcullatedSubtotal = calculatedPrice();
    double diskonRp = calcullatedSubtotal * (arg1 / 100.0);
    // diskon Rp harus dibulatkan menjadi kelipatan 100
    diskonRp = qRound(diskonRp / 100.0) * 100.0;
    ui->diskonRpSpinBox->setValue(diskonRp);
}

void OrderItemDialog::on_diskonRpSpinBox_valueChanged(int arg1)
{
    // kalkulasi berapa persen diskon
    double calcullatedSubtotal = calculatedPrice();
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
    double subtotal = calculatedPrice();
    ui->diskonRpSpinBox->setMaximum(subtotal);
    double diskonRp = ui->diskonRpSpinBox->value();
    subtotal -= diskonRp; // Diskon nominal
    ui->subtotalSpinBox->setValue(subtotal);
}

double OrderItemDialog::calculatedPrice() const
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
    return harga * areaMultiplier * qty;
}
