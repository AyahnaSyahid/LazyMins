#include "akuntransaksidataviewer.h"
#include "ui_dataviewer.h"

// TODO: Buat dialog untuk tambah/edit akun transaksi
#include "src/dialogs/akuntransaksidialog.h"
#include "src/managers/managers.h"

#include <QStyledItemDelegate>
#include <QMenu>
#include <QMessageBox>
#include <QAction>

namespace {
    class AkunDelegate : public QStyledItemDelegate {
    public:
        using QStyledItemDelegate::QStyledItemDelegate;
    protected:
        void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &ix) const override {
            QStyledItemDelegate::initStyleOption(option, ix);
            switch (ix.column()) {
                case 0: {
                    // ID - rata kanan
                    option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
                    break;
                }
                case 1:
                case 2:
                case 3: {
                    // kode, nama, tipe - rata tengah
                    option->displayAlignment = Qt::AlignCenter;
                    break;
                }
                case 7: {
                    // saldo - rata kanan dengan format angka
                    option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
                    option->text = QLocale().toString(ix.data().toLongLong());
                    break;
                }
                case 8: {
                    // is_active - tampilkan Ya/Tidak
                    option->displayAlignment = Qt::AlignCenter;
                    option->text = ix.data().toInt() == 1 ? "Aktif" : "Non-aktif";
                    break;
                }
                default:
                    break;
            }
        }
    };
}

AkunTransaksiDataViewer::AkunTransaksiDataViewer(QWidget *parent) :  DataViewer(parent)
{
    ui = Ui();
    auto m = &model();

    setQueryArgs(R"--(
        SELECT id,
               kode,
               nama,
               tipe,
               nama_bank,
               nomor_rekening,
               atas_nama,
               saldo,
               is_active,
               description
          FROM akun_transaksi
    )--");

    setFilterColumnNames({"kode", "nama", "nama_bank"});
    ui->dataView->setItemDelegate(new AkunDelegate(this));
    ui->dataView->verticalHeader()->hide();
    ui->dataView->setEditTriggers(QTableView::NoEditTriggers);

    m->setHeaderData(0, Qt::Horizontal, "ID");
    m->setHeaderData(1, Qt::Horizontal, "Kode");
    m->setHeaderData(2, Qt::Horizontal, "Nama Akun");
    m->setHeaderData(3, Qt::Horizontal, "Tipe");
    m->setHeaderData(4, Qt::Horizontal, "Bank");
    m->setHeaderData(5, Qt::Horizontal, "No. Rekening");
    m->setHeaderData(6, Qt::Horizontal, "Atas Nama");
    m->setHeaderData(7, Qt::Horizontal, "Saldo");
    m->setHeaderData(8, Qt::Horizontal, "Status");
    m->setHeaderData(9, Qt::Horizontal, "Keterangan");

    ui->dataView->resizeColumnsToContents();
    connect(this, &DataViewer::refreshed, ui->dataView, &QTableView::resizeColumnsToContents);

    ui->dataView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->dataView, &QTableView::customContextMenuRequested,
            this, &AkunTransaksiDataViewer::on_dataView_customContextMenuRequested);

    addAkunAction(); // inisiasi action
}

AkunTransaksiDataViewer::~AkunTransaksiDataViewer() {}

QAction *AkunTransaksiDataViewer::addAkunAction()
{
    if (m_addAkunAction == nullptr) {
        m_addAkunAction = new QAction("Akun Baru", this);
        m_addAkunAction->setObjectName("addAkunAction");
        m_addAkunAction->setToolTip("Tambah akun transaksi baru");
        connect(m_addAkunAction, &QAction::triggered,
                this, &AkunTransaksiDataViewer::onAddAkunActionTriggered);
    }
    return m_addAkunAction;
}

void AkunTransaksiDataViewer::onAddAkunActionTriggered()
{
    openCreateAkunDialog();
}

void AkunTransaksiDataViewer::openCreateAkunDialog()
{
    // TODO: implementasi saat dialog tersedia
    AkunTransaksiDialog dlg(this);
    dlg.prepareCreate();
    connect(&dlg, &QDialog::accepted, this, &DataViewer::refresh);
    dlg.setWindowTitle("Form Akun Transaksi Baru");
    dlg.exec();
}

void AkunTransaksiDataViewer::openEditAkunDialog(int akunId)
{
    // TODO: implementasi saat dialog tersedia
    AkunTransaksiManager akunTransaksiManager;
    auto optAcc = akunTransaksiManager.getById(akunId);
    if (!optAcc.has_value()) {
      QMessageBox::warning(this, "Kesalahan", "Akun Transaksi tidak ditemukan");
      return ;
    }
    auto recAcc = *optAcc;
    AkunTransaksiDialog dlg(this);
    dlg.prepareModify(recAcc);
    connect(&dlg, &QDialog::accepted, this, &DataViewer::refresh);
    dlg.setWindowTitle("Edit Akun Transaksi");
    dlg.exec();
}

void AkunTransaksiDataViewer::on_dataView_customContextMenuRequested(const QPoint &pt)
{
    QMenu menu;
    menu.setToolTipsVisible(true);

    auto clickedIndex = ui->dataView->indexAt(pt);

    auto editAction = menu.addAction("Edit");
    editAction->setToolTip("Edit data akun transaksi");
    editAction->setEnabled(clickedIndex.isValid());

    menu.addSeparator();

    auto submenu = menu.addMenu("Data baru");
    submenu->setToolTipsVisible(true);
    submenu->addAction(addAkunAction());

    connect(editAction, &QAction::triggered, [this, clickedIndex]() {
        if (clickedIndex.isValid())
            openEditAkunDialog(clickedIndex.siblingAtColumn(0).data().toInt());
    });

    menu.exec(ui->dataView->viewport()->mapToGlobal(pt));
}
