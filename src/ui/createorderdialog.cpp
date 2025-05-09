#include "createorderdialog.h"
#include "files/ui_createorderdialog.h"

#include <QDate>
#include <QCalendarWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QSqlTableModel>
#include <QTableView>
#include <QSqlRecord>
#include <QMessageBox>
#include <QSpinBox>
#include <QDoubleSpinBox>

#include <QtMath>

#include <QtDebug>


CreateOrderDialog::CreateOrderDialog(QWidget* parent) :
ui(new Ui::CreateOrderDialog), QDialog(parent) {
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    ui->lDate->setText(QDate::currentDate().toString("dd/MM/yyyy"));
    ui->lDate->setToolTip(locale().toString(QDate::currentDate(),"dddd, dd MMMM"));
    
    QSqlTableModel* customerModel = new QSqlTableModel(this);
    customerModel->setObjectName("customerModel");
    customerModel->setTable("customers");
    customerModel->select();
    ui->customerBox->setModel(customerModel);
    ui->customerBox->setModelColumn(1);
    ui->customerBox->setCurrentIndex(-1);
    auto cv = ui->customerBox->view();
    connect(ui->customerBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CreateOrderDialog::onCustomerChanged);

    QSqlTableModel* productModel = new QSqlTableModel(this);
    productModel->setObjectName("productModel");
    productModel->setTable("products");
    productModel->select();
    ui->productBox->setModel(productModel);
    ui->productBox->setModelColumn(1);
    ui->productBox->setCurrentIndex(-1);
    QTableView* productBoxView = new QTableView(ui->productBox);
    ui->productBox->setView(productBoxView);
    productBoxView->verticalHeader()->setMinimumSectionSize(23);
    productBoxView->verticalHeader()->setDefaultSectionSize(23);
    productBoxView->verticalHeader()->hide();
    productBoxView->horizontalHeader()->setStretchLastSection(true);
    productBoxView->resizeColumnsToContents();
    productBoxView->horizontalHeader()->hide();
    productBoxView->hideColumn(0);
    productBoxView->hideColumn(3);
    productBoxView->hideColumn(4);
    productBoxView->hideColumn(5);
    productBoxView->hideColumn(6);
    productBoxView->hideColumn(7);
    productBoxView->hideColumn(8);
    productBoxView->hideColumn(9);
    productBoxView->setSelectionBehavior(QTableView::SelectRows);
    productBoxView->setVerticalScrollMode(QTableView::ScrollPerPixel);
    productBoxView->setHorizontalScrollMode(QTableView::ScrollPerPixel);
    productBoxView->setMinimumWidth(productBoxView->horizontalHeader()->length());
    connect(ui->productBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CreateOrderDialog::changeProduct);
    connect(ui->spinWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CreateOrderDialog::updateSubTotal);
    connect(ui->spinHeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CreateOrderDialog::updateSubTotal);
    connect(ui->spinQty, QOverload<int>::of(&QSpinBox::valueChanged), this, &CreateOrderDialog::updateSubTotal);
    connect(ui->spinPrice, QOverload<int>::of(&QSpinBox::valueChanged), this, &CreateOrderDialog::updateSubTotal);
}

CreateOrderDialog::~CreateOrderDialog() {
    delete ui;
}

void CreateOrderDialog::setCurrentOrderDate(const QDate& cDate) {
    ui->lDate->setText(cDate.toString("dd/MM/yyyy"));
    ui->lDate->setToolTip(locale().toString(cDate,"dddd, dd MMMM"));
}

void CreateOrderDialog::on_pickDate_clicked() {
    QDialog d(this);
    d.setWindowFlag(Qt::FramelessWindowHint);
    QVBoxLayout ly;
    QCalendarWidget *caw = new QCalendarWidget(&d);
    caw->setSelectedDate(QDate::fromString(ui->lDate->text(), "d/MM/yyyy"));
    ly.addWidget(caw);
    d.setLayout(&ly);
    connect(caw, &QCalendarWidget::clicked, this, &CreateOrderDialog::setCurrentOrderDate);
    connect(caw, &QCalendarWidget::clicked, &d, &QDialog::accept);
    d.exec();
}

void CreateOrderDialog::onCustomerChanged(int ix) {
    ui->keteranganGroup->setEnabled(ix > -1);
    ui->buttonContainer->setEnabled(ix > -1);
    ui->createPaymentButton->setEnabled(ix > -1);
    if(ix < 0) {
        ui->customerInfo->setText("--info--");
        return;
    }

    QSqlTableModel* customerModel = findChild<QSqlTableModel*>("customerModel");
    auto rc = customerModel->record(ix);
    ui->customerInfo->setText(QString("%1\n%2").arg(rc.value("address").toString(), rc.value("phone").toString()));
}

void CreateOrderDialog::changeProduct(int ix) {
    if(ix < 0) {
        ui->spinPrice->setValue(0);
        return;
    }
    QSqlTableModel* productModel = findChild<QSqlTableModel*>("productModel");
    auto rc = productModel->record(ix);
    if(rc.isEmpty()) {
        QMessageBox::information(this, "Error", "Kesalahan, Produk yang dipilih tidak tersedia");
        return;
    }
    // i think 'name' should be empty to force the user to provide input
    // ui->nameEdit->setText(rc.value("name").toString());
    ui->sizeInput->setEnabled(rc.value("use_area").toBool());
    ui->spinPrice->setValue(rc.value("base_price").toInt());
}

void CreateOrderDialog::updateSubTotal() {
    double width  = ui->spinWidth->value(), 
           height = ui->spinHeight->value();
    int qty   = ui->spinQty->value(),
        price = ui->spinPrice->value();
    
    ui->lSubT->setText(locale().toString(qCeil(width * height * qty * price/ 500) * 500));
}

void CreateOrderDialog::on_resetButton_clicked() {
    ui->customerBox->setCurrentIndex(-1);
    ui->productBox->setCurrentIndex(-1);
}