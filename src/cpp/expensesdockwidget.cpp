#include "expensesdockwidget.h"
#include "ui_expensesdockwidget.h"


ExpensesDockWidget::ExpensesDockWidget(QWidget* parent)
: ui(new Ui::ExpensesDockWidget), QDockWidget(parent) {
    ui->setupUi(this);
}

ExpensesDockWidget::~ExpensesDockWidget() {
    delete ui;
}