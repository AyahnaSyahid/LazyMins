#include "priceleveleditordialog.h"

#include <QAction>
#include <QCloseEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSpinBox>
#include <QStyledItemDelegate>

#include "priceleveldialog.h"
#include "src/customs/buttonguard.h"
#include "src/managers/managers.h"
#include "src/models/priceleveleditormodel.h"
#include "ui_priceleveleditordialog.h"

namespace {
class PriceLevelDelegate : public QStyledItemDelegate {
 public:
  using QStyledItemDelegate::QStyledItemDelegate;
  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                        const QModelIndex& index) const override {
    if (index.column() == 1) {
      auto editor = new QSpinBox(parent);
      editor->setGroupSeparatorShown(true);
      editor->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      editor->setButtonSymbols(QSpinBox::NoButtons);
      editor->setMaximum(9999999);
      editor->setSingleStep(500);
      editor->setAccelerated(true);
      return editor;
    }
    return QStyledItemDelegate::createEditor(parent, option, index);
  }

  void setEditorData(QWidget* editor, const QModelIndex& ix) const override {
    if (ix.column() == 1) {
      auto sp = qobject_cast<QSpinBox*>(editor);
      if (sp) {
        // 1. Ambil batasan minimum dari role khusus Anda
        QVariant minData = ix.data(PriceLevelEditorModel::DefaultPriceRole);
        if (minData.isValid()) {
          sp->setMinimum(minData.toInt());
        } else {
          sp->setMinimum(0);  // Fallback jika role tidak ditemukan
        }

        sp->setMaximum(9999999);

        // 2. WAJIB: Masukkan nilai saat ini ke editor
        // Tanpa ini, saat double-klik, angka di box akan reset ke minimum
        QVariant currentVal = ix.data(Qt::EditRole);
        sp->setValue(currentVal.isValid() ? currentVal.toInt() : sp->minimum());
      }
      return;
    }
    QStyledItemDelegate::setEditorData(editor, ix);
  }

  void setModelData(QWidget* editor, QAbstractItemModel* md,
                    const QModelIndex& ix) const override {
    if (ix.column() == 1) {
      auto sp = qobject_cast<QSpinBox*>(editor);
      if (sp) {
        QVariant minData = ix.data(PriceLevelEditorModel::DefaultPriceRole);
        // if(minData.toInt() == sp->value()) return; // just dont set data if
        // value same as minData
        md->setData(ix, sp->value(), Qt::EditRole);
        return;
      }
    }
    // Perbaikan: Nama fungsi dasar adalah setModelData, bukan setData
    QStyledItemDelegate::setModelData(editor, md, ix);
  }

 protected:
  void initStyleOption(QStyleOptionViewItem* option,
                       const QModelIndex& ix) const override {
    QStyledItemDelegate::initStyleOption(option, ix);
    if (ix.column() == 1) {
      auto pr_data = ix.data(Qt::EditRole);
      if (pr_data.isNull()) {
        option->text = "Belum di setel";
        option->displayAlignment = Qt::AlignCenter;
      } else {
        option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
        option->text = QLocale().toString(pr_data.toInt());
      }
    }
  }
};
}  // namespace

PriceLevelEditorDialog::PriceLevelEditorDialog(QWidget* p)
    : ui(new Ui::PriceLevelEditorDialog),
      m_model(new PriceLevelEditorModel(this)),
      QDialog(p) {
  ui->setupUi(this);
  ui->tableView->setItemDelegate(new PriceLevelDelegate(this));
  ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
}

PriceLevelEditorDialog::~PriceLevelEditorDialog() { delete ui; }

bool PriceLevelEditorDialog::setProductId(int pid) {
  ProductManager productManager;
  auto opt_rec = productManager.getById(pid);
  if (!opt_rec.has_value()) {
    QMessageBox::warning(
        nullptr, "Kesalahan",
        QString("Produk dengan id %1 tidak ditemukan").arg(pid));
    return false;
  }
  m_productId = pid;
  auto prod = *opt_rec;
  ui->labelSku->setText(prod.value("sku").toString());
  ui->labelNama->setText(prod.value("name").toString());
  ui->labelDeskripsi->setText(prod.value("description").toString());
  m_defaultCost = prod.value("cost_price").toInt();
  ui->costSpinBox->setValue(m_defaultCost);
  m_model->setProductId(pid);
  ui->tableView->setModel(m_model);
  return true;
}

bool PriceLevelEditorDialog::isAnythingDirty() const {
  // Cek perubahan harga modal
  bool costChanged = (ui->costSpinBox->value() != m_defaultCost);

  // Cek perubahan pada model level harga
  return costChanged || (m_model && m_model->isDirty());
}

bool PriceLevelEditorDialog::saveAll() {
  // 1. Cek perubahan Harga Modal (Cost Price)
  if (ui->costSpinBox->value() != m_defaultCost) {
    ProductManager pmgr;
    bool updateOk =
        pmgr.update(m_productId, {{"cost_price", ui->costSpinBox->value()}});
    if (!updateOk) {
      QMessageBox::information(this, "Kesalahan",
                               QString("Update harga default produk gagal:\n%1")
                                   .arg(pmgr.errorString()));
      return false;
    }
    m_defaultCost = ui->costSpinBox->value();
    emit dataCommited();
  }

  // 2. Cek perubahan pada Model Level Harga
  if (m_model->isDirty()) {
    if (!m_model->commit()) {
      QMessageBox::critical(this, "Error", "Gagal menyimpan data harga level.");
      return false;
    }
    emit dataCommited();
  }

  return true;  // Semua berhasil disimpan atau tidak ada perubahan
}

void PriceLevelEditorDialog::on_simpanButton_clicked() {
  ButtonGuard guard(ui->simpanButton);
  if (saveAll()) {
    accept();
  }
}

void PriceLevelEditorDialog::reject() {
  // Cek apakah ada perubahan di costSpinBox atau di tabel
  if (isAnythingDirty()) {
    QMessageBox::StandardButton resBtn = QMessageBox::question(
        this, "Konfirmasi", "Perubahan belum disimpan. Simpan sekarang?",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (resBtn == QMessageBox::Save) {
      if (saveAll()) {
        QDialog::accept();  // Keluar dengan status Accepted
      }
      // Jika saveAll gagal, dialog tetap terbuka
    } else if (resBtn == QMessageBox::Discard) {
      QDialog::reject();  // Keluar tanpa simpan
    }
    // Jika Cancel, jangan lakukan apa-apa (dialog tetap terbuka)
  } else {
    QDialog::reject();  // Tidak ada perubahan, keluar normal
  }
}

void PriceLevelEditorDialog::on_tableView_customContextMenuRequested(
    const QPoint& p) {
  QMenu ctx;
  ctx.setToolTipsVisible(true);
  auto newLevel = ctx.addAction("Level baru");
  newLevel->setToolTip("Buat level baru");
  connect(newLevel, &QAction::triggered, this,
          &PriceLevelEditorDialog::onCreateNewLevel);
  ctx.exec(ui->tableView->viewport()->mapToGlobal(p));
}

void PriceLevelEditorDialog::onCreateNewLevel() {
  PriceLevelDialog pld(this);
  pld.setWindowTitle("Buat Level baru");
  connect(&pld, &QDialog::accepted,
          [this]() { m_model->setProductId(m_productId); });
  pld.exec();
}