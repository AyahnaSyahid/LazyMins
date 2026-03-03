#include "orderdialog.h"
#include "ui_orderdialog.h"

#include <QHeaderView>

namespace {
  const QHash<int, QString> Column {
    { 0,  "id"                  },
    { 1,  "order_id"            },
    { 2,  "product_id"          },
    { 3,  "product_name"        },
    { 4,  "sku"                 },
    { 5,  "quantity"            },
    { 6,  "unit"                },
    { 7,  "base_price"          },
    { 8,  "discount_percentage" },
    { 9,  "discount_amount"     },
    {10,  "subtotal"            },
    {11,  "notes"               },
    {12,  "created_at"          },
    {13,  "updated_at"          }, 
  };
}

OrderDialog::OrderDialog(QWidget *p):
ui(new Ui::OrderDialog), emodel(new OrderItemEditorModel(this)), FormDialog(p)
{
  ui->setupUi(this);
 
  ui->orderItemView->setModel(emodel);
  ui->orderItemView->hideColumn(0);
  ui->orderItemView->hideColumn(1);
  ui->orderItemView->hideColumn(4);
  ui->orderItemView->hideColumn(8);
  ui->orderItemView->hideColumn(9);
  ui->orderItemView->hideColumn(11);
  ui->orderItemView->hideColumn(12);
  ui->orderItemView->hideColumn(13);
  
  ui->orderNumberLineEdit->setText(OrderManager::generateOrderNumber());
  ui->tOrderDateTimeEdit->setDateTime(QDateTime::currentDateTime());
  ui->dLineDateTimeEdit->setDateTime(QDateTime::currentDateTime());
  setupFields();
}

OrderDialog::~OrderDialog() { delete ui; }

void OrderDialog::setupFields() {
  setFields({
    { ui->orderNumberLineEdit,   "order_number" },
    { ui->konsumenLineEdit,      "customer_name" },
    { ui->kontakLineEdit,        "customer_phone" },
    { ui->statusLineEdit,        "status" },
    { ui->prioritasLineEdit,     "priority" },
    { ui->catatan1TextEdit,      "notes" },
    { ui->catatan2TextEdit,      "internal_notes" },
    { ui->subtotalSpinBox,       "subtotal" },
    { ui->diskonDoubleSpinBox,   "discount_percentage" },
    { ui->diskonRpSpinBox,       "discount_amount" },
    { ui->pajakRpSpinBox,        "tax_amount" },
    { ui->totalSpinBox,          "total_amount" }
    });
}

void OrderDialog::setupBoundFields() {
  addBoundField("order_date",
  [this](){ auto local = ui->tOrderDateTimeEdit->dateTime();
            return local.toTimeZone(QTimeZone::UTC);},
  [this](QVariant val) { 
            auto local = val.toDateTime();
            local.setTimeZone(QTimeZone::LocalTime);
            ui->tOrderDateTimeEdit->setDateTime(local);}
  );
  addBoundField("deadline_date",
  [this](){ auto local = ui->dLineDateTimeEdit->dateTime();
            return local.toTimeZone(QTimeZone::UTC);},
  [this](QVariant val) { 
            auto local = val.toDateTime();
            local.setTimeZone(QTimeZone::LocalTime);
            ui->dLineDateTimeEdit->setDateTime(local);}
  );
}

bool OrderDialog::onSave(const QVariantMap& mp) { return false; }

void OrderDialog::prepareModify(const QSqlRecord& orderRecord) {
  FormDialog::prepareModify(orderRecord);
  emodel->loadFromOrder(orderRecord.value("id").toInt());
}