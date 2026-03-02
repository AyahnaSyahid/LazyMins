#include "orderdialog.h"
#include "ui_orderdialog.h"

OrderDialog::OrderDialog(QWidget *p):
ui(new Ui::OrderDialog), emodel(new OrderItemEditorModel(this)), FormDialog(p)
{
  ui->setupUi(this);
  ui->orderItemView->setModel(emodel);
  ui->orderNumberLineEdit->setText(OrderManager::generateOrderNumber());
  setupFields();
}

OrderDialog::~OrderDialog() { delete ui; }

void OrderDialog::setupBoundFields() {
  addBoundField("order_date",
  [this](){ auto local = ui->tOrderDateTimeEdit->dateTime();
            return local.toTimeZone(QTimeZone::UTC);}
  );
}

void OrderDialog::setupFields() {
  setFields({
    {ui->orderNumberLineEdit,   "order_number"},
    {ui->konsumenLineEdit,      "customer_name"},
    {ui->kontakLineEdit,        "customer_phone"},
    {ui->statusLineEdit,        "status"},
    {ui->prioritasLineEdit,     "priority"},
    {ui->catatan1TextEdit,      "notes"},
    {ui->catatan2TextEdit,      "internal_notes"},
    {ui->subtotalSpinBox,       "subtotal"},
    {ui->diskonDoubleSpinBox,   "discount_percentage"},
    {ui->diskonRpSpinBox,       "discount_amount"},
    {ui->pajakRpSpinBox,        "tax_amount"},
    {ui->totalSpinBox,          "total_amount"},
    });
}

bool OrderDialog::onSave(const QVariantMap& mp) { return false; }

void OrderDialog::prepareModify(const QSqlRecord& orderRecord) {
  FormDialog::prepareModify(orderRecord);
  emodel->loadFromOrder(orderRecord.value("id").toInt());
}