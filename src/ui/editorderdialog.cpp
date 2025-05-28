#include "editorderdialog.h"
#include "database.h"
#include "usermanager.h"
#include "files/ui_editorderdialog.h"
#include <QVBoxLayout>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QMessageBox>
#include <QCalendarWidget>
#include <QDate>

#include <QtMath>
#include <QtDebug>

EditOrderDialog::EditOrderDialog(const QSqlRecord& rec, Database* _d, QWidget* parent) :
_record(rec), db(_d), ui(new Ui::EditOrderDialog), QDialog(parent) {
    ui->setupUi(this);
    ui->lDate->setText(QDate::fromString(_record.value("order_date").toString(), "yyyy-MM-dd").toString("dd/MM/yyyy"));
    ui->spinWidth->setValue(_record.value("width").toDouble());
    ui->spinHeight->setValue(_record.value("height").toDouble());
    ui->spinQty->setValue(_record.value("quantity").toInt());
    ui->spinDiscount->setValue(_record.value("discount").toLongLong());
    ui->spinPrice->setValue(_record.value("unit_price").toLongLong());
    ui->spinPrice->setMinimum(_record.value("production_cost").toLongLong());
    ui->nameEdit->setText(_record.value("name").toString());
    
    auto pmod = db->getTableModel("products");
    ui->productBox->setModel(pmod);
    ui->productBox->setModelColumn(1);
    
    QSqlQuery q;
    q.prepare("SELECT name, use_area FROM products WHERE product_id = ?");
    q.addBindValue(rec.value("product_id"));
    q.exec() && q.next();
    ui->sizeInput->setEnabled(q.value("use_area").toBool());
    ui->productBox->setCurrentIndex(ui->productBox->findText(q.value("name").toString()));
    
    connect(ui->spinDiscount, SIGNAL(valueChanged(int)), SLOT(updateSubTotal()));
    connect(ui->spinPrice, SIGNAL(valueChanged(int)), SLOT(updateSubTotal()));
    connect(ui->spinQty, SIGNAL(valueChanged(int)), SLOT(updateSubTotal()));
    connect(ui->spinWidth, SIGNAL(valueChanged(double)), SLOT(updateSubTotal()));
    connect(ui->spinHeight, SIGNAL(valueChanged(double)), SLOT(updateSubTotal()));

    updateSubTotal();
}

EditOrderDialog::~EditOrderDialog(){
    delete ui;
}


void EditOrderDialog::on_cancelButton_clicked(){
    reject();
}

void EditOrderDialog::on_saveButton_clicked(){
    setDisabled(true);
    auto rec = _record;
    // rec.setGenerated("order_id", false);
    rec.setValue("name", ui->nameEdit->text().simplified());
    rec.setValue("width", ui->spinWidth->value());
    rec.setValue("height", ui->spinHeight->value());
    rec.setValue("quantity", ui->spinQty->value());
    rec.setValue("unit_price", ui->spinPrice->value());
    rec.setValue("discount", ui->spinDiscount->value());
    rec.setValue("updated_utc", "`CURRENT_TIMESTAMP`");
    rec.setValue("user_id", db->findChild<UserManager*>("userManager")->currentUser());
    
    QSqlQuery q;
    q.prepare("UPDATE orders SET (name, width, height, quantity, unit_price, discount, updated_utc) = (?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP) WHERE order_id = ?");
    q.addBindValue(rec.value("name"));
    q.addBindValue(rec.value("width"));
    q.addBindValue(rec.value("height"));
    q.addBindValue(rec.value("quantity"));
    q.addBindValue(rec.value("unit_price"));
    q.addBindValue(rec.value("discount"));
    q.addBindValue(rec.value("order_id"));
    if(q.exec()) {
        auto model = db->getTableModel("orders");
        model->select();
        
        if(!_record.isNull("invoice_id")) {
            // do invoice update
            auto invId = _record.value("invoice_id");
            q.prepare(R"--(
                UPDATE invoices SET total_amount = (
                SELECT COALESCE(SUM(oc.price), 0) AS OCP
                       FROM orders_calc oc
                       LEFT JOIN orders od ON oc.order_id = od.order_id
                       LEFT JOIN invoices inv ON od.invoice_id = inv.invoice_id
                       WHERE inv.invoice_id = ?
                       GROUP BY inv.invoice_id ) WHERE invoice_id = ?
            )--");
            q.addBindValue(invId);
            q.addBindValue(invId);
            q.exec();
            db->getTableModel("invoices")->select();
        }
        accept();
    } else {
        QMessageBox::information(this, "Gagal Update", QString("Error :%1").arg(q.lastError().text()));
    }
}

void EditOrderDialog::on_pickDate_clicked() {
    QDialog d(this);
    d.setWindowFlag(Qt::FramelessWindowHint);
    QVBoxLayout ly;
    QCalendarWidget *caw = new QCalendarWidget(&d);
    caw->setSelectedDate(QDate::fromString(ui->lDate->text(), "d/MM/yyyy"));
    ly.addWidget(caw);
    d.setLayout(&ly);
    connect(caw, &QCalendarWidget::clicked, this, &EditOrderDialog::setCurrentOrderDate);
    connect(caw, &QCalendarWidget::clicked, &d, &QDialog::accept);
    d.exec();
}

void EditOrderDialog::updateSubTotal() {
    double width  = ui->spinWidth->value(), 
           height = ui->spinHeight->value();
    int qty   = ui->spinQty->value(),
        price = ui->spinPrice->value();
    ui->lSubT->setText(locale().toString((qCeil(width * height * qty * price/ 500) * 500) - ui->spinDiscount->value()));
}


void EditOrderDialog::setCurrentOrderDate(const QDate& cd) {
    ui->lDate->setText(cd.toString("dd-MM-yyyy"));
}

void EditOrderDialog::on_productBox_currentIndexChanged(int x) {
    auto pmod = qobject_cast<QSqlTableModel*>(ui->productBox->model());
    auto rc = pmod->record(x);
    ui->sizeInput->setEnabled(rc.value("use_area").toBool());
    auto spVal = ui->spinPrice->value();
    auto bpVal = rc.value("base_price").toInt();
    if(bpVal > spVal) {
        ui->spinPrice->setValue(bpVal);
        QMessageBox::information(this, tr("Informasi"), tr("Harga satuan lebih rendah dari harga dasar produk, telah disesuaikan secara otomatis"));
    }
}