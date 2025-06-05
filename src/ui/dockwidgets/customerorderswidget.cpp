#include "customerorderswidget.h"
#include "ui/files/ui_customerorderswidget.h"
#include "database.h"
#include <QSqlQueryModel>
#include <QSqlQuery>

CustomerOrdersWidget::CustomerOrdersWidget(Database* _d, QWidget* parent) {
    : ui(new Ui::CustomerOrdersWidget), model(nullptr), QWidget(parent)
{
    ui->setupUi(this);
    connect(_d->getTableModel("orders"), &SqlTableModel::modelReset, this, &CustomerOrdersWidget::reloadData);
    connect(_d->getTableModel("invoices"), &SqlTableModel::modelReset, this, &CustomerOrdersWidget::reloadData);
    connect(_d->getTableModel("customers"), &SqlTableModel::modelReset, this, &CustomerOrdersWidget::reloadData);
    model = new QSqlQueryModel(this);
    
    model->setQuery(R"--(
        SELECT c.customer_name
    )--");
}

CustomerOrdersWidget::~CustomerOrdersWidget() {
    delete ui;
}

void CustomerOrdersWidget::reloadData() {
    auto q = model->query();
    q.exec();
    model->setQuery(q);
}