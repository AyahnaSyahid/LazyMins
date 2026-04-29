#include "konsumendataviewer.h"

#include "ui_dataviewer.h"

// TODO: Buat dialog untuk tambah/edit konsumen
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QStyledItemDelegate>

#include "src/dialogs/konsumendialog.h"
#include "src/managers/konsumenmanager.h"

namespace {
class KonsumenDelegate : public QStyledItemDelegate {
 public:
  using QStyledItemDelegate::QStyledItemDelegate;

 protected:
  void initStyleOption(QStyleOptionViewItem* option,
                       const QModelIndex& ix) const override {
    QStyledItemDelegate::initStyleOption(option, ix);
    switch (ix.column()) {
      case 0: {
        // ID - rata kanan
        option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
        break;
      }
      case 1:
      case 3: {
        // customer_code, customer_type - tengah
        option->displayAlignment = Qt::AlignCenter;
        break;
      }
      case 12: {
        // is_active - tampilkan Aktif/Non-aktif
        option->displayAlignment = Qt::AlignCenter;
        option->text = ix.data().toInt() == 1 ? "Aktif" : "Non-aktif";
        break;
      }
      case 13: {
        // last_seen - format tanggal
        option->displayAlignment = Qt::AlignCenter;
        QDateTime dt = ix.data().toDateTime();
        if (dt.isValid()) option->text = dt.toString("dd MMMM yyyy");
        break;
      }
      default:
        break;
    }
  }
};
}  // namespace

KonsumenDataViewer::KonsumenDataViewer(QWidget* parent) : DataViewer(parent) {
  ui = Ui();
  auto m = &model();

  setQueryArgs(R"--(
        SELECT k.id,
               k.customer_code,
               k.nama_lengkap,
               k.customer_type,
               k.email,
               k.nomor_telp,
               k.alamat,
               k.kota,
               k.kode_pos,
               k.npwp,
               k.catatan,
               pl.level_name  AS price_level,
               k.is_active,
               k.last_seen
          FROM konsumen k
          JOIN price_levels pl ON k.price_level_id = pl.id
    )--");

  setFilterColumnNames(
      {"k.customer_code", "k.nama_lengkap", "k.nomor_telp", "k.email"});
  ui->dataView->setItemDelegate(new KonsumenDelegate(this));
  ui->dataView->verticalHeader()->hide();
  ui->dataView->setEditTriggers(QTableView::NoEditTriggers);

  m->setHeaderData(0, Qt::Horizontal, "ID");
  m->setHeaderData(1, Qt::Horizontal, "Kode");
  m->setHeaderData(2, Qt::Horizontal, "Nama");
  m->setHeaderData(3, Qt::Horizontal, "Tipe");
  m->setHeaderData(4, Qt::Horizontal, "Email");
  m->setHeaderData(5, Qt::Horizontal, "Telepon");
  m->setHeaderData(6, Qt::Horizontal, "Alamat");
  m->setHeaderData(7, Qt::Horizontal, "Kota");
  m->setHeaderData(8, Qt::Horizontal, "Kode Pos");
  m->setHeaderData(9, Qt::Horizontal, "NPWP");
  m->setHeaderData(10, Qt::Horizontal, "Catatan");
  m->setHeaderData(11, Qt::Horizontal, "Level Harga");
  m->setHeaderData(12, Qt::Horizontal, "Status");
  m->setHeaderData(13, Qt::Horizontal, "Terakhir Order");

  ui->dataView->resizeColumnsToContents();
  connect(this, &DataViewer::refreshed, ui->dataView,
          &QTableView::resizeColumnsToContents);

  ui->dataView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->dataView, &QTableView::customContextMenuRequested, this,
          &KonsumenDataViewer::on_dataView_customContextMenuRequested);

  addKonsumenAction();  // inisiasi action
}

KonsumenDataViewer::~KonsumenDataViewer() {}

QAction* KonsumenDataViewer::addKonsumenAction() {
  if (m_addKonsumenAction == nullptr) {
    m_addKonsumenAction = new QAction("Konsumen Baru", this);
    m_addKonsumenAction->setObjectName("addKonsumenAction");
    m_addKonsumenAction->setToolTip("Tambah data konsumen baru");
    connect(m_addKonsumenAction, &QAction::triggered, this,
            &KonsumenDataViewer::onAddKonsumenActionTriggered);
  }
  return m_addKonsumenAction;
}

void KonsumenDataViewer::onAddKonsumenActionTriggered() {
  openCreateKonsumenDialog();
}

void KonsumenDataViewer::openCreateKonsumenDialog() {
  KonsumenDialog dlg(this);
  dlg.prepareCreate();
  connect(&dlg, &QDialog::accepted, this, &DataViewer::refresh);
  connect(&dlg, &QDialog::accepted, [this]() {
    auto resp = QMessageBox::question(this, "Notifikasi",
                                      "Data konsumen telah disimpan\nmasih ada "
                                      "data konsumen yang perlu ditambahkan ?");
    if (resp == QMessageBox::Yes) openCreateKonsumenDialog();
  });
  dlg.setWindowTitle("Form Konsumen Baru");
  dlg.exec();
}

void KonsumenDataViewer::openEditKonsumenDialog(int konsumenId) {
  KonsumenManager konsumenManager;
  auto optKon = konsumenManager.getById(konsumenId);
  if (!optKon.has_value()) {
    QMessageBox::warning(this, "Kesalahan", "Data Konsumen tidak ditemukan");
    return;
  }

  auto recKon = *optKon;
  // TODO: implementasi saat dialog tersedia
  KonsumenDialog dlg(this);
  dlg.prepareModify(recKon);
  connect(&dlg, &QDialog::accepted, this, &DataViewer::refresh);
  dlg.setWindowTitle("Edit Data Konsumen");
  dlg.exec();
}

void KonsumenDataViewer::on_dataView_customContextMenuRequested(
    const QPoint& pt) {
  QMenu menu;
  menu.setToolTipsVisible(true);

  auto clickedIndex = ui->dataView->indexAt(pt);
  const bool hasSelection = clickedIndex.isValid();

  auto editAction = menu.addAction("Edit");
  editAction->setToolTip("Edit data konsumen");
  editAction->setEnabled(hasSelection);

  menu.addSeparator();

  auto submenu = menu.addMenu("Data baru");
  submenu->setToolTipsVisible(true);
  submenu->addAction(addKonsumenAction());

  connect(editAction, &QAction::triggered, [this, clickedIndex]() {
    if (clickedIndex.isValid())
      openEditKonsumenDialog(clickedIndex.siblingAtColumn(0).data().toInt());
  });

  menu.exec(ui->dataView->viewport()->mapToGlobal(pt));
}
