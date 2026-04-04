#include "priceleveleditordialog.h"
#include "ui_priceleveleditordialog.h"
#include "src/models/priceleveleditormodel.h"
#include "src/managers/managers.h"

#include <QStyledItemDelegate>
#include <QMessageBox>

namespace {
  class PriceLevelDelegate : public QStyledItemDelegate 
  {
    public:
      using QStyledItemDelegate::QStyledItemDelegate;
    
    protected:
      void initStyleOption(QStyleOptionViewItem *option, const QModelIndex& ix ) const override {
        QStyledItemDelegate::initStyleOption(option, ix);
        if (ix.column() == 1) {
          auto pr_data = ix.data(Qt::EditRole);
          if(pr_data.isNull()) {
            option->text = "Belum di setel";
            option->displayAlignment = Qt::AlignCenter;
          } else {
            option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
            option->text = QLocale().toString(pr_data.toInt());
          }
        }
      }
   };
}

PriceLevelEditorDialog::PriceLevelEditorDialog(QWidget *p):
  ui(new Ui::PriceLevelEditorDialog), model(new PriceLevelEditorModel(this)), QDialog(p)
{
  ui->setupUi(this);
  ui->tableView->setItemDelegate(new PriceLevelDelegate(this));
}

PriceLevelEditorDialog::~PriceLevelEditorDialog() { delete ui; }

bool PriceLevelEditorDialog::setProductId(int pid)
{
  ProductManager productManager;
  auto opt_rec = productManager.getById(pid);
  if (!opt_rec.has_value()) {
    QMessageBox::warning(nullptr, "Kesalahan", QString("Produk dengan id %1 tidak ditemukan").arg(pid));
    return false;
  }
  auto prod = *opt_rec;
  ui->labelSku->setText(prod.value("sku").toString());
  ui->labelNama->setText(prod.value("name").toString());
  ui->labelDeskripsi->setText(prod.value("description").toString());
  ui->costSpinBox->setValue(prod.value("cost_price").toInt());
  
  model->setProductId(pid);
  ui->tableView->setModel(model);
  return true;
}