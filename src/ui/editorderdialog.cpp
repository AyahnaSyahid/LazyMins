#include "editorderdialog.h"
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

EditOrderDialog::EditOrderDialog(const QSqlRecord& rec, QWidget* parent) :
_record(rec), ui(new Ui::EditOrderDialog), QDialog(parent) {
    ui->setupUi(this);
    ui->lDate->setText(QDate::fromString(_record.value("order_date").toString(), "yyyy-MM-dd").toString("dd/MM/yyyy"));
    ui->spinWidth->setValue(_record.value("width").toDouble());
    ui->spinHeight->setValue(_record.value("height").toDouble());
    ui->spinQty->setValue(_record.value("quantity").toInt());
    ui->spinDiscount->setValue(_record.value("discount").toLongLong());
    ui->spinPrice->setValue(_record.value("unit_price").toLongLong());
    ui->nameEdit->setText(_record.value("name").toString());
    
    auto pmod = new QSqlTableModel(this);
    pmod->setObjectName("productModel");
    pmod->setTable("products");
    ui->productBox->setModel(pmod);
    ui->productBox->setModelColumn(1);
    pmod->select();
    
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

EditOrderDialog* EditOrderDialog::fromId(qint64 l, QWidget* p) {
    QSqlQuery q;
    q.prepare("SELECT * FROM orders WHERE order_id = ?");
    q.addBindValue(l);
    q.exec() && q.next();
    return new EditOrderDialog(q.record(), p);
}

EditOrderDialog::~EditOrderDialog(){
    delete ui;
}


void EditOrderDialog::on_cancelButton_clicked(){
    reject();
}

void EditOrderDialog::on_saveButton_clicked(){
    auto rec = _record;
    rec.setGenerated("order_id", false);
    rec.setValue("name", ui->nameEdit->text().simplified());
    rec.setValue("width", ui->spinWidth->value());
    rec.setValue("height", ui->spinHeight->value());
    rec.setValue("quantity", ui->spinQty->value());
    rec.setValue("unit_price", ui->spinPrice->value());
    rec.setValue("discount", ui->spinDiscount->value());
    hide();
    emit queryUpdate(rec);
}

void EditOrderDialog::updateStatus(const QSqlError& err, const QSqlRecord&) {
    show();
    if(not err.isValid()) {
        accept();
        return;
    }
    QMessageBox::information(this, tr("Gagal menyimpan"), QString("Error : ").arg(err.text()));
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
    auto pmod = findChild<QSqlTableModel*>("productModel");
    auto rc = pmod->record(x);
    ui->sizeInput->setEnabled(rc.value("use_area").toBool());
    auto spVal = ui->spinPrice->value();
    auto bpVal = rc.value("base_price").toInt();
    if(bpVal > spVal) {
        ui->spinPrice->setValue(bpVal);
        QMessageBox::information(this, tr("Informasi"), tr("Harga satuan lebih rendah dari harga dasar produk, telah disesuaikan secara otomatis"));
    }
}