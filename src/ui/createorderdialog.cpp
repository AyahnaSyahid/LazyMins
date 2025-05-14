#include "createorderdialog.h"
#include "files/ui_createorderdialog.h"
#include "models/createordermodel.h"
#include "createcustomerdialog.h"
#include "createproductdialog.h"
#include "createinvoicedialog.h"
#include "editorderdialog.h"

#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QItemSelectionModel>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QDate>
#include <QCalendarWidget>
#include <QMenu>
#include <QAction>
#include <QDialog>
#include <QVBoxLayout>
#include <QTableView>
#include <QMessageBox>
#include <QSpinBox>
#include <QCompleter>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QRect>

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
    if(ui->customerBox->completer())
        ui->customerBox->completer()->setCompletionMode(QCompleter::PopupCompletion);
    connect(ui->customerBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CreateOrderDialog::onCustomerChanged);
    connect(ui->customerBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CreateOrderDialog::updateOrdersModel);
    ui->customerBox->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &CreateOrderDialog::customerCreated, customerModel, &QSqlTableModel::select);

    QSqlTableModel* productModel = new QSqlTableModel(this);
    productModel->setObjectName("productModel");
    productModel->setTable("products");
    productModel->select();
    ui->productBox->setModel(productModel);
    ui->productBox->setModelColumn(1);
    ui->productBox->setCurrentIndex(-1);
    if(ui->productBox->completer())
        ui->productBox->completer()->setCompletionMode(QCompleter::PopupCompletion);
    QTableView* productBoxView = new QTableView(ui->productBox);
    ui->productBox->setView(productBoxView);
    productBoxView->verticalHeader()->setMinimumSectionSize(23);
    productBoxView->verticalHeader()->setDefaultSectionSize(23);
    productBoxView->verticalHeader()->hide();
    productBoxView->horizontalHeader()->setStretchLastSection(true);
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
    productBoxView->resizeColumnsToContents();
    productBoxView->setMinimumWidth(productBoxView->horizontalHeader()->length());
    ui->productBox->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->productBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CreateOrderDialog::changeProduct);
    connect(ui->spinWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CreateOrderDialog::updateSubTotal);
    connect(ui->spinHeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CreateOrderDialog::updateSubTotal);
    connect(ui->spinQty, QOverload<int>::of(&QSpinBox::valueChanged), this, &CreateOrderDialog::updateSubTotal);
    connect(ui->spinPrice, QOverload<int>::of(&QSpinBox::valueChanged), this, &CreateOrderDialog::updateSubTotal);
    connect(ui->spinDiscount, QOverload<int>::of(&QSpinBox::valueChanged), this, &CreateOrderDialog::updateSubTotal);
    connect(this, &CreateOrderDialog::productCreated, productModel, &QSqlTableModel::select);
    
    CreateOrderModel *orderModel = new CreateOrderModel(this);
    orderModel->setObjectName("orderModel");
    ui->unpaidTableView->setModel(orderModel);
    ui->unpaidTableView->hideColumn(0);
    connect(orderModel, &CreateOrderModel::reloaded, ui->unpaidTableView, &QTableView::resizeColumnsToContents);
    connect(orderModel, &CreateOrderModel::reloaded, this, &CreateOrderDialog::updateLSum);
    connect(ui->unpaidTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &CreateOrderDialog::updateLSum);
    ui->unpaidTableView->setContextMenuPolicy(Qt::CustomContextMenu);
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
        ui->customerInfo->setPlainText("--info--");
        return;
    }

    QSqlTableModel* customerModel = findChild<QSqlTableModel*>("customerModel");
    auto rc = customerModel->record(ix);
    ui->customerInfo->setPlainText(QString("%1\n%2").arg(rc.value("address").toString(), rc.value("phone").toString()));
    CreateOrderModel* orderModel = findChild<CreateOrderModel*>("orderModel");
    if(orderModel) 
        orderModel->setCustomerId(rc.value("customer_id").toInt());
}

void CreateOrderDialog::changeProduct(int ix) {
    if(ix < 0) {
        ui->spinPrice->setValue(0);
        ui->spinQty->setValue(1);
        ui->nameEdit->clear();
        ui->spinWidth->setValue(1);
        ui->spinHeight->setValue(1);
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
    ui->lSubT->setText(locale().toString((qCeil(width * height * qty * price/ 500) * 500) - ui->spinDiscount->value()));
}

void CreateOrderDialog::on_resetButton_clicked() {
    ui->customerBox->setCurrentIndex(-1);
    ui->productBox->setCurrentIndex(-1);
    findChild<CreateOrderModel*>("orderModel")->setCustomerId(-1);
    ui->lDate->setText(QDate::currentDate().toString("dd/MM/yyyy"));
}

// Implement Required
void CreateOrderDialog::on_createPaymentButton_clicked() {
    auto sm = ui->unpaidTableView->selectionModel();
    auto customerModel = qobject_cast<QSqlTableModel*>(ui->customerBox->model());
    auto rc = customerModel->record(ui->customerBox->currentIndex());
    // mari permudah dengan memberikan referensi ke dialog invoice
    CreateInvoiceDialog* cid = new CreateInvoiceDialog(sm, this);
    QLabel *nameLabel  = cid->findChild<QLabel*>("label_2"),
           *phoneLabel = cid->findChild<QLabel*>("label_5"),
           *addrLabel = cid->findChild<QLabel*>("label_6");
    nameLabel->setText(rc.value("name").toString());
    phoneLabel->setText(rc.value("phone").toString());
    addrLabel->setText(rc.value("address").toString());
    cid->setAttribute(Qt::WA_DeleteOnClose);
    cid->adjustSize();
    cid->open();
}

void CreateOrderDialog::on_draftButton_clicked() {
    if(ui->customerBox->currentIndex() < 0) {
        QMessageBox::information(this, tr("Kesalahan Input"), tr("Anda belum menentukan Konsumen"));
        ui->customerBox->setFocus(Qt::OtherFocusReason);
        return;
    }
    if(ui->productBox->currentIndex() < 0) {
        QMessageBox::information(this, tr("Kesalahan Input"), tr("Anda belum menentukan Produk"));
        ui->productBox->setFocus(Qt::OtherFocusReason);
        return;
    }
    if(ui->nameEdit->text().simplified().isEmpty()) {
        QMessageBox::information(this, tr("Kesalahan Input"), tr("Anda belum memasukan nama Pesanan"));
        ui->nameEdit->setFocus(Qt::OtherFocusReason);
        return;
    }
    QSqlTableModel model;
    model.setTable("orders");
    auto productModel = findChild<QSqlTableModel*>("productModel");
    auto customerModel = findChild<QSqlTableModel*>("customerModel");
    
    auto rPro = productModel->record(ui->productBox->currentIndex());
    auto rCus = customerModel->record(ui->customerBox->currentIndex());

    QSqlRecord rec = model.record();
    rec.setGenerated("order_id", false);
    rec.setValue("order_date", QDate::fromString(ui->lDate->text(), "dd/MM/yyyy").toString("yyyy-MM-dd"));
    rec.setValue("user_id", 1);
    rec.setValue("customer_id", rCus.value("customer_id"));
    rec.setValue("product_id", rPro.value("product_id"));
    rec.setValue("name", ui->nameEdit->text().simplified());
    rec.setValue("use_area", rPro.value("use_area"));
    rec.setValue("width", ui->spinWidth->value());
    rec.setValue("height", ui->spinHeight->value());
    rec.setValue("quantity", ui->spinQty->value());
    rec.setValue("width", ui->spinWidth->value());
    rec.setValue("production_cost", rPro.value("cost"));
    rec.setValue("unit_price", ui->spinPrice->value());
    rec.setValue("discount", ui->spinDiscount->value());
    rec.setGenerated("created_utc", false);
    rec.setGenerated("updated_utc", false);
    rec.setValue("status", "OK");
    setEnabled(false);
    emit queryInsert(rec);
}

void CreateOrderDialog::insertStatus(const QSqlError& err, const QSqlRecord& rec) {
    setEnabled(true);
    if(err.isValid()) {
        QMessageBox::information(this, "Gagal", QString("Error :%1").arg(err.text()));
        return;
    }
    ui->nameEdit->clear();
    ui->productBox->setCurrentIndex(-1);
    ui->productBox->setFocus(Qt::MouseFocusReason);
    CreateOrderModel* orderModel = findChild<CreateOrderModel*>("orderModel");
    ui->lDate->setText(QDate::currentDate().toString("dd/MM/yyyy"));
    orderModel->setCustomerId(rec.value("customer_id").toInt());
}

void CreateOrderDialog::updateOrdersModel() {
    auto model = qobject_cast<CreateOrderModel*>(ui->unpaidTableView->model());
    model->reload();
}

void CreateOrderDialog::on_customerBox_customContextMenuRequested(const QPoint& pp) {
    QMenu context(this);
    auto newCustomer = context.addAction(tr("Konsumen Baru"));
    newCustomer->connect(newCustomer, &QAction::triggered, this, &CreateOrderDialog::createCustomer);
    context.exec(ui->customerBox->mapToGlobal(pp));
}

void CreateOrderDialog::on_productBox_customContextMenuRequested(const QPoint& pp) {
    QMenu context(this);
    auto newProduct = context.addAction(tr("Produk Baru"));
    newProduct->connect(newProduct, &QAction::triggered, this, &CreateOrderDialog::createProduct);
    context.exec(ui->productBox->mapToGlobal(pp));
}

void CreateOrderDialog::on_unpaidTableView_doubleClicked(const QModelIndex& x) {
    setProperty("orderEditId", x.siblingAtColumn(0).data(Qt::EditRole).toLongLong());
    QMenu context(this);
    auto r = ui->unpaidTableView->visualRect(x);
    auto act = context.addAction("Edit");
    act->connect(act, &QAction::triggered, this, &CreateOrderDialog::editOrder);
    context.exec(ui->unpaidTableView->viewport()->mapToGlobal(r.topRight()));
}

void CreateOrderDialog::createCustomer() {
    CreateCustomerDialog* ccd = new CreateCustomerDialog(this);
    ccd->setAttribute(Qt::WA_DeleteOnClose);
    ccd->connect(ccd, &QDialog::accepted, this, &CreateOrderDialog::customerCreated);
    ccd->open();
}

void CreateOrderDialog::createProduct() {
    CreateProductDialog* cpd= new CreateProductDialog(this);
    cpd->setAttribute(Qt::WA_DeleteOnClose);
    cpd->connect(cpd, &QDialog::accepted, this, &CreateOrderDialog::productCreated);
    cpd->open();
}

void CreateOrderDialog::updateLSum() {
    QString ts("Total : %1 (Semua), %2 (Terpilih)");
    auto model = findChild<CreateOrderModel*>("orderModel");
    ui->lSum->setText(ts.arg(locale().toString(model->sum()), locale().toString(model->sum(ui->unpaidTableView->selectionModel()->selectedRows()))));
}

void CreateOrderDialog::editOrder() {
    auto order_id = property("orderEditId").toLongLong();
    if(order_id < 1) {
        QMessageBox::information(this, tr("Internal Error"), tr("Tidak dapat mengubah order_id < 1"));
        return;
    }
    EditOrderDialog* eod = EditOrderDialog::fromId(order_id, this);
    eod->connect(eod, &QDialog::accepted, this, &CreateOrderDialog::orderModified);
    auto orderModel = findChild<CreateOrderModel*>("orderModel");
    eod->connect(eod, &QDialog::accepted, orderModel, &CreateOrderModel::reload);
    eod->connect(eod, &EditOrderDialog::queryUpdate, this, &CreateOrderDialog::queryUpdate);
    eod->connect(this, &CreateOrderDialog::updateStatus, eod, &EditOrderDialog::updateStatus);
    eod->setAttribute(Qt::WA_DeleteOnClose);
    eod->adjustSize();
    eod->open();
}

