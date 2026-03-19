#include "konsumenpickerdialog.h"
#include "ui_konsumenpickerdialog.h"
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


KonsumenPickerDialog::KonsumenPickerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KonsumenPickerDialog),
    model(new QSqlQueryModel(this))
{
    ui->setupUi(this);
    model->setQuery(R"--(
SELECT k.id,
       nama_lengkap,
       pl.id as pl_id,
       level_name,
       nomor_telp
  FROM konsumen k
       JOIN
       price_levels pl ON k.price_level_id = pl.id;)--", BaseManager::connection);
    
    while(model->canFetchMore()) model->fetchMore();
    
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Nama");
    model->setHeaderData(2, Qt::Horizontal, "Level ID");
    model->setHeaderData(3, Qt::Horizontal, "Level Harga");
    model->setHeaderData(4, Qt::Horizontal, "Nomor Telepon");
    
    auto proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(model);
    proxy->setFilterKeyColumn(1);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy->sort(1);
    ui->konsumenView->setModel(proxy);
    ui->konsumenView->hideColumn(0);
    ui->konsumenView->hideColumn(2);
    // ui->konsumenView->hideColumn(4);
    
    ui->konsumenView->horizontalHeader()->setStretchLastSection(true);
    ui->konsumenView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->konsumenView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->konsumenView->verticalHeader()->setMinimumSectionSize(20);
    ui->konsumenView->verticalHeader()->setDefaultSectionSize(22);
    ui->konsumenView->setAlternatingRowColors(true);
    
    auto delayer = new QTimer(this);
    delayer->setSingleShot(true);
    delayer->setInterval(500);
    
    connect(delayer, &QTimer::timeout, [this, proxy]() {
      proxy->setFilterFixedString(ui->lineEdit->text());
    });
    connect(ui->lineEdit, &QLineEdit::textChanged, delayer, [delayer](){delayer->start(); });
}

KonsumenPickerDialog::~KonsumenPickerDialog(){
    delete ui;
}

void KonsumenPickerDialog::on_konsumenView_clicked(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }
    auto proxy = findChild<QSortFilterProxyModel*>();
    if(!proxy) return;
    // Handle the selection of a customer
    auto record = model->record(proxy->mapToSource(index).row());
    emit konsumenPicked(record);
    accept(); // Close the dialog after selection    
}   