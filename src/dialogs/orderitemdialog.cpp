#include "orderitemdialog.h"
#include "ui_orderitemdialog.h"

OrderItemDialog::OrderItemDialog(QWidget *parent) :
    FormDialog(parent),
    ui(new Ui::OrderItemDialog)
{
    ui->setupUi(this);
    setupFields();
    ui->produkComboBox->setQuery("SELECT id, name, description FROM products");
    ui->produkComboBox->boxViewAutoResize();
    ui->produkComboBox->setCurrentIndex(-1);
}

OrderItemDialog::~OrderItemDialog()
{
    delete ui;
}

bool OrderItemDialog::onSave(const QVariantMap& changes) {
    qDebug() << changes;
    return true;
}

void OrderItemDialog::setupFields() {
    setFields({
        { ui->namaLineEdit, "product_name" },
        { ui->hargaSpinBox, "base_price" },
        { ui->qtySpinBox, "quantity" },
        { ui->subtotalSpinBox, "subtotal" },
        { ui->diskonDoubleSpinBox, "discount_percentage" },
        { ui->diskonRpSpinBox, "discount_amount" },
        { ui->notesTextEdit, "notes" }
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
