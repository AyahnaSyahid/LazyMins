#include "productpickerdialog.h"
#include "files/ui_productpickerdialog.h"
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QModelIndex>

class ProductProxy : public QSortFilterProxyModel {
    public:
        ProductProxy(QObject* parent=nullptr) : QSortFilterProxyModel(parent) {};
        ~ProductProxy(){};
        QVariant headerData(int sec, Qt::Orientation, int role=Qt::DisplayRole) const;
        QVariant data(const QModelIndex&, int role=Qt::DisplayRole) const;
};

ProductPickerDialog::ProductPickerDialog(QAbstractItemModel* src, QWidget* parent)
    : proxy(new ProductProxy), ui(new Ui::ProductPickerDialog), QDialog(parent)
{
    proxy->setSourceModel(src);
    proxy->setFilterKeyColumn(-1);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui->setupUi(this);
    ui->productView->setModel(proxy);
    ui->productView->hideColumn(0);
    ui->productView->hideColumn(3);
    ui->productView->hideColumn(8);
    ui->productView->hideColumn(9);
    // auto hModel = new QStandardItemModel(0, src->columnCount(), this);
    // hModel->setHorizontalHeaderLabels(QString("CID,Name,Alamat,No. Telepon,CRT,UPT").split(','));
    // ui->productView->horizontalHeader()->setModel(hModel);
    ui->productView->resizeColumnsToContents();
    connect(ui->filterEdit, &QLineEdit::textChanged, proxy, QOverload<const QString&>::of(&QSortFilterProxyModel::setFilterRegularExpression));
}

ProductPickerDialog::~ProductPickerDialog() {
    delete proxy;
    delete ui;
}

int ProductPickerDialog::toSourceIndex(const QModelIndex& proxy_index) {
    auto six = proxy->mapToSource(proxy_index);
    return six.row();
}

void ProductPickerDialog::on_productView_clicked(const QModelIndex& mi) {
    emit activated(toSourceIndex(mi));
    accept();
}


QVariant ProductProxy::headerData(int sec, Qt::Orientation ori, int role) const {
    QStringList vHL = QString("PID,Nama,Deskripsi,Hitung Luas,H Produksi,H Dasar,Material,Kategori,CRT,UPT").split(',');
    if(ori == Qt::Horizontal && role == Qt::DisplayRole) {
        return vHL.at(sec);
    }
    return QSortFilterProxyModel::headerData(sec, ori, role);
}

QVariant ProductProxy::data(const QModelIndex& mi, int role) const {
    if(role == Qt::TextAlignmentRole) {
        switch(mi.column()) {
            case 0:
            case 1:
            case 3:
            case 6:
            case 8:
            case 9:
                return Qt::AlignCenter;
            case 5:
            case 4:
                return int(Qt::AlignRight | Qt::AlignVCenter);
            
        }
    }
    if(role == Qt::DisplayRole) {
        QLocale loc;
        switch(mi.column()) {
            case 4:
            case 5:
                return loc.toString(QSortFilterProxyModel::data(mi, Qt::EditRole).toLongLong());
        }
    }
    return QSortFilterProxyModel::data(mi, role);
}