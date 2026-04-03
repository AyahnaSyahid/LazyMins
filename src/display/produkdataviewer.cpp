#include "produkdataviewer.h"
#include "ui_dataviewer.h"
#include <QStyledItemDelegate>
#include "src/dialogs/stockopnamedialog.h"

#include <QMenu>
#include <QAction>
#include <QMessageBox>

namespace {
    class PDelegate : public QStyledItemDelegate {
        public:
            using QStyledItemDelegate::QStyledItemDelegate;
        protected:
            void initStyleOption(QStyleOptionViewItem *option, const QModelIndex& ix) const override {
                QStyledItemDelegate::initStyleOption(option, ix);
                switch (ix.column()) {
                    case 0: {
                        option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
                        break;
                    }
                    case 1:
                    case 5: {
                        option->displayAlignment = Qt::AlignCenter;
                        break;
                    }
                    case 6:
                    case 7:
                    case 8: {
                        option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
                        option->text = QLocale().toString(ix.data().toInt());
                        break;
                    }
                    case 9:
                    case 10: {
                        option->displayAlignment = Qt::AlignCenter;
                        option->text = ix.data().toInt() == 0 ? "Tidak" : "Ya";
                        break;
                    }
                }
                if (ix.siblingAtColumn(6).data().toDouble() <= ix.siblingAtColumn(7).data().toDouble()) {
                    option->backgroundBrush = QColor(255, 200, 200);
                }
            }
    };
}

ProdukDataViewer::ProdukDataViewer(QWidget *parent)
{
    ui = Ui();
    auto m = &model();
    m->setQueryArgs(R"--(
SELECT p.id, sku, name, category_name,
       p.description, unit, stock, min_stock, 
       cost_price, CAST(p.is_active AS INTEGER) AS active, use_area
  FROM products p
       JOIN
       product_categories c ON category_id = c.id )--");

    ui->dataView->verticalHeader()->hide();
    ui->dataView->setItemDelegate(new PDelegate(this));
    setFilterColumnNames({"sku", "name", "category_name"});
    m->setHeaderData(0,  Qt::Horizontal, "ID");
    m->setHeaderData(1,  Qt::Horizontal, "SKU");
    m->setHeaderData(2,  Qt::Horizontal, "Nama");
    m->setHeaderData(3,  Qt::Horizontal, "Kategori");
    m->setHeaderData(4,  Qt::Horizontal, "Deskripsi");
    m->setHeaderData(5,  Qt::Horizontal, "Unit");
    m->setHeaderData(6,  Qt::Horizontal, "Stok");    
    m->setHeaderData(7,  Qt::Horizontal, "Min");
    m->setHeaderData(8,  Qt::Horizontal, "Cost");
    m->setHeaderData(9,  Qt::Horizontal, "Area");
    m->setHeaderData(10, Qt::Horizontal, "Aktif");
    
    ui->dataView->setEditTriggers(QTableView::NoEditTriggers);
    ui->dataView->resizeColumnsToContents();
    
    ui->dataView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->dataView, &QTableView::customContextMenuRequested, this, &ProdukDataViewer::on_dataView_customContextMenuRequested);
}

ProdukDataViewer::~ProdukDataViewer() {}

void ProdukDataViewer::on_dataView_customContextMenuRequested(const QPoint& pt)
{
  QMenu menu;
  auto clickedIndex = ui->dataView->indexAt(pt);
  auto opnameAction = menu.addAction("Stok Opname");
  connect(opnameAction, &QAction::triggered, [this, clickedIndex]() {
    if (clickedIndex.isValid())
      openStockOpname(clickedIndex.siblingAtColumn(0).data().toInt());
    });
  menu.exec(ui->dataView->mapToGlobal(pt));
}

void ProdukDataViewer::openStockOpname(int product_id)
{
  StockOpnameDialog dl(this);
  dl.setProductId(product_id);
  connect(&dl, &QDialog::accepted, this, &DataViewer::refresh);
  dl.exec();
}


