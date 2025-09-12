#include "ordersofinvoicedialog.h"
#include "files/ui_ordersofinvoicedialog.h"

OrdersOfInvoiceDialog::OrdersOfInvoiceDialog(int inv, Database *database, QWidget *parent)
: ui(new Ui::OrdersOfInvoiceDialog), db(database), mod(new OrdersOfInvoiceModel(inv, database, this)), QDialog(parent) {
    ui->setupUi(this);
    ui->tableView->setModel(mod);
    ui->tableView->hideColumn(0);
    ui->tableView->hideColumn(2);
    ui->tableView->hideColumn(3);
    ui->tableView->hideColumn(6);
    ui->tableView->hideColumn(8);
    ui->tableView->hideColumn(10);
    ui->tableView->hideColumn(13);
    ui->tableView->hideColumn(14);
    ui->tableView->hideColumn(15);
    ui->tableView->hideColumn(16);
}

OrdersOfInvoiceDialog::~OrdersOfInvoiceDialog() {
  delete ui;
}
