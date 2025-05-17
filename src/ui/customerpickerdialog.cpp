#include "customerpickerdialog.h"
#include "files/ui_customerpickerdialog.h"
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QModelIndex>

CustomerPickerDialog::CustomerPickerDialog(QAbstractItemModel* src, QWidget* parent)
    : proxy(new QSortFilterProxyModel), ui(new Ui::CustomerPickerDialog), QDialog(parent)
{
    proxy->setSourceModel(src);
    proxy->setFilterKeyColumn(-1);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui->setupUi(this);
    ui->customerView->setModel(proxy);
    ui->customerView->hideColumn(0);
    ui->customerView->hideColumn(4);
    ui->customerView->hideColumn(5);
    auto hModel = new QStandardItemModel(0, src->columnCount(), this);
    hModel->setHorizontalHeaderLabels(QString("CID,Name,Alamat,No. Telepon,CRT,UPT").split(','));
    ui->customerView->horizontalHeader()->setModel(hModel);
    ui->customerView->resizeColumnsToContents();
    connect(ui->filterEdit, &QLineEdit::textChanged, proxy, QOverload<const QString&>::of(&QSortFilterProxyModel::setFilterRegularExpression));
    
}

CustomerPickerDialog::~CustomerPickerDialog() {
    delete proxy;
    delete ui;
}

int CustomerPickerDialog::toSourceIndex(const QModelIndex& proxy_index) {
    auto six = proxy->mapToSource(proxy_index);
    return six.row();
}

void CustomerPickerDialog::on_customerView_clicked(const QModelIndex& mi) {
    emit activated(toSourceIndex(mi));
    accept();
}