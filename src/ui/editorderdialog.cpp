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
    QSqlQuery q;
    q.prepare("SELECT name FROM products WHERE product_id = ?");
    q.addBindValue(rec.value("product_id"));
    q.exec() && q.next();
    ui->productBox->setCurrentText(q.value("name").toString());
    ui->spinWidth->setValue(_record.value("width").toDouble());
    ui->spinHeight->setValue(_record.value("height").toDouble());
    ui->spinQty->setValue(_record.value("quantity").toInt());
    ui->spinDiscount->setValue(_record.value("discount").toLongLong());
}

EditOrderDialog::~EditOrderDialog(){
    delete ui;
}


void EditOrderDialog::on_cancelButton_clicked(){
    reject();
}

void EditOrderDialog::on_saveButton_clicked(){
    hide();
}

void EditOrderDialog::queryStatus(const QSqlError& err, const QSqlRecord&) {
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