#include "customerpickerdialog.h"
#include "ui_customerpickerdialog.h"
#include "src/managers/basemanager.h"
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include <QTimer>

namespace {
  class Delegate: public QStyledItemDelegate
  {
    public:
      Delegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
      ~Delegate() = default;
      void initStyleOption(QStyleOptionViewItem *option, const QModelIndex& index) const override {
        
      }
  };
}


CustomerPickerDialog::CustomerPickerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CustomerPickerDialog),
    model(new QSqlQueryModel(this))
{
    ui->setupUi(this);
    model->setQuery(R"--(
    SELECT k.id,
           nama_lengkap,
           pl.id AS pl_id,
           level_name,
           nomor_telp
      FROM konsumen k
           JOIN price_levels pl ON k.price_level_id = pl.id
    )--", BaseManager::connection);
    
    while(model->canFetchMore()) model->fetchMore();
    
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Nama");
    model->setHeaderData(2, Qt::Horizontal, "Level ID");
    model->setHeaderData(3, Qt::Horizontal, "Level Harga");
    model->setHeaderData(4, Qt::Horizontal, "Nomor Telepon");
    
    auto proxy = new QSortFilterProxyModel(this);
    proxy->setObjectName("proxy");
    proxy->setSourceModel(model);
    proxy->setFilterKeyColumn(-1);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy->sort(1);
    ui->customerView->setModel(proxy);
    ui->customerView->hideColumn(0);
    ui->customerView->hideColumn(2);
    // ui->customerView->hideColumn(4);
    
    ui->customerView->horizontalHeader()->setStretchLastSection(true);
    ui->customerView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->customerView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->customerView->verticalHeader()->setMinimumSectionSize(20);
    ui->customerView->verticalHeader()->setDefaultSectionSize(22);
    ui->customerView->setAlternatingRowColors(true);
    
    auto delayer = new QTimer(this);
    delayer->setSingleShot(true);
    delayer->setInterval(500);
    
    connect(delayer, &QTimer::timeout, [this, proxy]() {
      proxy->setFilterFixedString(ui->lineEdit->text());
    });
    connect(ui->lineEdit, &QLineEdit::textChanged, delayer, [delayer](){delayer->start(); });
}

CustomerPickerDialog::~CustomerPickerDialog(){
    delete ui;
}

void CustomerPickerDialog::on_customerView_clicked(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }
    auto proxy = findChild<QSortFilterProxyModel*>("proxy");
    if(!proxy) return;
    // Handle the selection of a customer
    auto record = model->record(proxy->mapToSource(index).row());
    emit customerPicked(record);
    accept(); // Close the dialog after selection    
}

void CustomerPickerDialog::setModelQuery(const QString& name)
{
  QSqlQuery q(BaseManager::connection);
  q.prepare(name);
  if (!q.exec()) {
    qWarning() << "CustomerPickerDialog : setModelQuery Error :" << q.lastError().text() ;
    return ;
  }
  model->setQuery(std::move(q));
}