#include "editorderdialog.h"
#include "ui_editorderdialog.h"
#include "orderitem.h"
#include "customeritem.h"
#include "productitem.h"
#include "databasenotifier.h"
#include "helper.h"
#include "config.h"

#include <QSqlQueryModel>
#include <QModelIndex>
#include <QSqlRecord>
#include <QDateTime>
#include <QtMath>
#include <QtDebug>

EditOrderDialog::EditOrderDialog(QWidget* parent)
: ui(new Ui::EditOrderDialog), QDialog(parent)
{
    ui->setupUi(this);
    auto *cm = new QSqlQueryModel(this);
    cm->setQuery("SELECT customer_id, customer_name FROM customers");
    ui->comboBox->setModel(cm);
    ui->comboBox->setModelColumn(1);
    auto cm2 = new QSqlQueryModel(this);
    cm2->setQuery("SELECT product_id, code FROM products");
    ui->comboBox2->setModel(cm2);
    ui->comboBox2->setModelColumn(1);
    connect(ui->doubleSpinBoxP, SIGNAL(valueChanged(double)), this, SLOT(updateHarga()));
    connect(ui->doubleSpinBoxT, SIGNAL(valueChanged(double)), this, SLOT(updateHarga()));
    connect(ui->doubleSpinBoxQ, SIGNAL(valueChanged(double)), this, SLOT(updateHarga()));
    connect(ui->doubleSpinBoxH, SIGNAL(valueChanged(double)), this, SLOT(updateHarga()));
    connect(ui->spinBox, SIGNAL(valueChanged(int)), this, SLOT(updateHarga()));
}

EditOrderDialog::~EditOrderDialog() {
    delete ui;
}

void EditOrderDialog::updateHarga() {
    qreal h = 0;
    h = ui->doubleSpinBoxP->value() *
        ui->doubleSpinBoxT->value() *
        ui->doubleSpinBoxH->value() *
        ui->doubleSpinBoxQ->value();
    ui->label_6->setText(locale().toString((qreal)(qCeil(h / 100.0) * 100) - ui->spinBox->value(), 'g', 16));
}

void EditOrderDialog::setOrder(qint64 oid) {
    OrderItem oi(oid);
    CustomerItem ci(oi.customer_id);
    ProductItem pi(oi.product_id);
    ui->comboBox->setCurrentText(ci.customer_name);
    // qDebug() << ci.customer_name;
    ui->comboBox2->setCurrentText(pi.code);
    ui->lineEditN->setText(oi.order_name);
    ui->doubleSpinBoxP->setValue(oi.width_mm ? oi.width_mm / 1000.0 : 1);
    ui->doubleSpinBoxT->setValue(oi.height_mm ? oi.height_mm / 1000.0 : 1);
    ui->doubleSpinBoxQ->setValue(oi.quantity);
    ui->doubleSpinBoxH->setValue(oi.selling_price);
    ui->spinBox->setValue(oi.discount_amount);
    ui->dateTimeEdit->setDateTime(oi.order_date);
    order_id = oi.order_id;
    updateHarga();
}

void EditOrderDialog::on_pushButton_clicked() {
    OrderItem o(order_id);
    auto combo = ui->comboBox;
    o.customer_id = combo->model()->index(combo->currentIndex(), 0).data().toLongLong();
    combo = ui->comboBox2;
    o.product_id = combo->model()->index(combo->currentIndex(), 0).data().toLongLong();
    o.order_name = ui->lineEditN->text().trimmed();
    o.width_mm = ui->doubleSpinBoxP->value() * 1000;
    o.height_mm = ui->doubleSpinBoxT->value() * 1000;
    o.quantity = ui->doubleSpinBoxQ->value();
    o.selling_price = ui->doubleSpinBoxH->value();
    o.discount_amount = ui->spinBox->value();
    o.order_date = ui->dateTimeEdit->dateTime();
    if(!o.save()) {
        MessageHelper::information(this, "Gagal", "Perubahan gagal disimpan");
        return;
    }
    dbNot->emitChanged("orders");
    accept();
}

void EditOrderDialog::on_comboBox2_currentIndexChanged(int ix) {
    ProductItem pi(ui->comboBox2->model()->index(ix, 0).data().toLongLong());
    OrderItem oi(order_id);
    auto enable = pi.use_area;
    ui->label_7->setEnabled(enable);
    ui->label_8->setEnabled(enable);
    ui->label_9->setEnabled(enable);
    ui->doubleSpinBoxP->setEnabled(enable);
    ui->doubleSpinBoxT->setEnabled(enable);
    if(enable) {
        ui->doubleSpinBoxP->setValue(oi.width_mm ? oi.width_mm / 1000 : 1);
        ui->doubleSpinBoxT->setValue(oi.height_mm ? oi.height_mm / 1000 : 1);
    } else {
        ui->doubleSpinBoxP->setValue(1);
        ui->doubleSpinBoxT->setValue(1);    
    }
}

void EditOrderDialog::setProdukSelectorEnabled(bool b)
{
    ui->comboBox2->setEnabled(b);
}
