#include "produkdataviewer.h"
#include "ui_dataviewer.h"
#include <QStyledItemDelegate>
#include "src/dialogs/stockopnamedialog.h"
#include "src/dialogs/stockrefilldialog.h"
#include "src/dialogs/productdialog.h"
#include "src/dialogs/producteditordialog.h"
#include "src/dialogs/kategoriprodukdialog.h"
#include "src/dialogs/priceleveleditordialog.h"

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
                    case 6: {
                        option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
                        int use_area = ix.siblingAtColumn(10).data().toInt();
                        if(use_area == 1) {
                          option->text = QString("%L1").arg(ix.data().toDouble(), 0, 'f', 2);
                        } else {
                          option->text = QString("%L1").arg(ix.data().toInt());                   
                        }
                        break;
                    }
                    case 7:
                    case 8: {
                        option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
                        option->text = QString("%L1").arg(ix.data().toInt());                   
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
    setObjectName("productDataViewer");
    setQueryArgs(R"--(
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
    m->setHeaderData(9,  Qt::Horizontal, "Aktif");
    m->setHeaderData(10, Qt::Horizontal, "Area");
    
    ui->dataView->setEditTriggers(QTableView::NoEditTriggers);
    ui->dataView->resizeColumnsToContents();
    
    ui->dataView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->dataView, &QTableView::customContextMenuRequested, this, &ProdukDataViewer::on_dataView_customContextMenuRequested);
    addAction(addProductAction());
    addAction(addCategoryProductAction());
    
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &ProdukDataViewer::customContextMenuRequested, this, &ProdukDataViewer::onContextMenu);
}

ProdukDataViewer::~ProdukDataViewer() {}

void ProdukDataViewer::prependContextAction(QAction* act)
{
  m_prependedActions.append(act);
}


void ProdukDataViewer::on_dataView_customContextMenuRequested(const QPoint& pt)
{
  QMenu menu;
  menu.setToolTipsVisible(true);
  for(auto pact : m_prependedActions) {
    menu.addAction(pact);
  }
  if (m_prependedActions.size() > 0) {
    menu.addSeparator();
  }

  auto clickedIndex = ui->dataView->indexAt(pt);
  
  auto editDataAction = menu.addAction("Edit");
  editDataAction->setToolTip("Edit data produk");
  menu.addSeparator();
  auto setPriceAction = menu.addAction("Harga");
  setPriceAction->setToolTip("Tetapkan Harga Minimal");
  menu.addSeparator();
  auto refillAction = menu.addAction("ReStok");
  refillAction->setToolTip("Tambahkan data stok saat masuk");
  auto opnameAction = menu.addAction("Opname");
  opnameAction->setToolTip("Sesuaikan data stok dengan gudang");
  
  connect(opnameAction, &QAction::triggered, [this, clickedIndex]() {
    if (clickedIndex.isValid())
      openStockOpname(clickedIndex.siblingAtColumn(0).data().toInt());
    });
  connect(refillAction, &QAction::triggered, [this, clickedIndex]() {
    if (clickedIndex.isValid())
      openRefillDialog(clickedIndex.siblingAtColumn(0).data().toInt());
    });
  connect(setPriceAction, &QAction::triggered, [this, clickedIndex]() {
    if (clickedIndex.isValid())
      openPriceEditorDialog(clickedIndex.siblingAtColumn(0).data().toInt());
    });
  connect(editDataAction, &QAction::triggered, [this, clickedIndex]() {
    if (clickedIndex.isValid())
      openProductEditor(clickedIndex.siblingAtColumn(0).data().toInt());
    });
  
  
  menu.addSeparator();
  
  menu.addSeparator();
  auto submenu = menu.addMenu("Data baru");
  submenu->setToolTipsVisible(true);
  submenu->addAction(addProductAction());
  submenu->addAction(addCategoryProductAction());
  menu.exec(ui->dataView->mapToGlobal(pt));
}

void ProdukDataViewer::openStockOpname(int product_id)
{
  StockOpnameDialog dl(this);
  dl.setProductId(product_id);
  connect(&dl, &QDialog::accepted, this, &DataViewer::refresh);
  dl.exec();
}

void ProdukDataViewer::openRefillDialog(int produkId)
{
  StockRefillDialog dl(this);
  if(!dl.setProductId(produkId)) {
    return ;
  }
  connect(&dl, &QDialog::accepted, this, &DataViewer::refresh);
  dl.exec();
}

void ProdukDataViewer::openPriceEditorDialog(int productId)
{
  PriceLevelEditorDialog pd(this);
  if (!pd.setProductId(productId)) return ;
  connect(&pd, &PriceLevelEditorDialog::dataCommited, this, &DataViewer::refresh);
  pd.exec();
}

QAction* ProdukDataViewer::addProductAction()
{
  if(!m_addProductAction) {
    m_addProductAction = new QAction("Produk", this);
    m_addProductAction->setIcon(QIcon(":/svg/svg/add-product.svg"));
    m_addProductAction->setObjectName("addProductAction");
    m_addProductAction->setToolTip("Tambah Produk baru");
  }
  return m_addProductAction;
}

void ProdukDataViewer::onAddProductActionTriggered() {
  ProductDialog pd(this);
  pd.prepareCreate();
  connect(&pd, &QDialog::accepted, this, &DataViewer::refresh);
  pd.setWindowTitle("Tambah Produk");
  pd.exec();
}

QAction* ProdukDataViewer::addCategoryProductAction() {
  if(!m_addCategoryProductAction) {
    m_addCategoryProductAction = new QAction("Kategori Produk", this);
    m_addCategoryProductAction->setObjectName("addCategoryProductAction");
    m_addCategoryProductAction->setToolTip("Tambah Kategori Produk baru");
  }
  return m_addCategoryProductAction;
}

bool ProdukDataViewer::initialize(MainWindowContext *ctx)
{
    auto docked = ctx->addDock(this, "Data Produk", Qt::TopDockWidgetArea, "top_left", true);
    ctx->addMenuAction("Data/Produk", addProductAction(), [this] { onAddProductActionTriggered(); });
    ctx->addMenuAction("Data/Produk", addCategoryProductAction(), [this] { onAddCategoryProductActionTriggered(); });
    ctx->addDockToggleMenu(docked, "View");
    ctx->onEvent("order_created", this, [this](QVariant va){ refresh(); });
    return true;
}

void ProdukDataViewer::onAddCategoryProductActionTriggered() {
  KategoriProdukDialog dl(this);
  dl.prepareCreate();
  connect(&dl, &QDialog::accepted, this, &DataViewer::refresh);
  dl.setWindowTitle("Form Kategori Produk Baru");
  dl.exec();
}

void ProdukDataViewer::onContextMenu(const QPoint& p) {
  QMenu ctx;
  auto submenu = ctx.addMenu("Data baru");
  submenu->setToolTipsVisible(true);
  submenu->addActions(this->actions());
  ctx.exec(mapToGlobal(p));
}

void ProdukDataViewer::openProductEditor(int pid) {
  ProductEditorDialog ped;
  if (!ped.setProductId(pid)) {
    QMessageBox::critical(this, "Kesalahan", "Tidak dapat membuka editor:\nProduk " + QString::number(pid) + " tidak ditemukan");
    return ;
  }
  connect(&ped, &QDialog::accepted, this, &DataViewer::refresh);
  ped.exec();
}