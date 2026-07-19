#include "finishingservicesviewer.h"
#include "ui_dataviewer.h"

#include "src/dialogs/createfinishingservicedialog.h"

#include <QStyledItemDelegate>

#include <QStyledItemDelegate>
#include <QSpinBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QLineEdit>
#include <QMenu>
#include <QAction>
#include <QPainter>

namespace {
    class Delegate : public QStyledItemDelegate
    {
    public:
        Delegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
        ~Delegate() override = default;

        QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &mi) const override {
            QWidget *ed = nullptr;
            switch (mi.column()) {
                case 0: {
                    auto *sb = new QSpinBox(parent);
                    sb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    ed = sb;
                    break;
                }
                case 4: {
                    auto *sb = new QSpinBox(parent);
                    sb->setSingleStep(500);
                    // Max standard int32. Gunakan QDoubleSpinBox jika butuh lebih dari 2 miliar
                    sb->setMaximum(2147483647); 
                    sb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    sb->setGroupSeparatorShown(true);
                    ed = sb;
                    break;
                }
                case 6: {
                    auto *cb = new QComboBox(parent);
                    cb->addItem("Ya", 1);
                    cb->addItem("Tidak", 0);
                    // Gunakan TextAlignmentRole untuk perataan teks
                    cb->setItemData(0, Qt::AlignCenter, Qt::TextAlignmentRole);
                    cb->setItemData(1, Qt::AlignCenter, Qt::TextAlignmentRole);
                    ed = cb;
                    break;
                }
                case 7:
                case 8: {
                    auto *dte = new QDateTimeEdit(parent);
                    dte->setCalendarPopup(true); // Memudahkan user memilih tanggal
                    ed = dte;
                    break;
                }
                default: {
                    ed = new QLineEdit(parent);
                    break;
                }
            }
            return ed;
        }

        void setEditorData(QWidget *editor, const QModelIndex &mi) const override {
            switch (mi.column()) {
                case 0:
                case 4: {
                    auto *sb = qobject_cast<QSpinBox*>(editor);
                    if (sb) sb->setValue(mi.data(Qt::EditRole).toInt());
                    break;
                }
                case 6: {
                    auto *cb = qobject_cast<QComboBox*>(editor);
                    // Gunakan findData karena kita menyimpan 1 dan 0 sebagai Qt::UserRole
                    if (cb) cb->setCurrentIndex(cb->findData(mi.data(Qt::EditRole).toInt()));
                    break;
                }
                case 7:
                case 8: {
                    auto *dte = qobject_cast<QDateTimeEdit*>(editor);
                    if (dte) {
                        auto udate = mi.data(Qt::EditRole).toDateTime();
                        QDateTime local(udate.toTimeZone(QTimeZone::LocalTime));
                        dte->setDateTime(local); // setDateTime lebih aman daripada setCurrentDate
                    }
                    break;
                }
                default: {
                    auto *le = qobject_cast<QLineEdit*>(editor);
                    if (le) {
                      le->setText(mi.data(Qt::EditRole).toString());
                      if (mi.column() == 5) {
                        le->setAlignment(Qt::AlignCenter);
                      }
                    }
                    break;
                }
            }
        }

        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &mi) const override {
            switch (mi.column()) {
                case 0:
                case 4: {
                    auto *sb = qobject_cast<QSpinBox*>(editor);
                    if (sb) model->setData(mi, sb->value(), Qt::EditRole);
                    break;
                }
                case 6: {
                    auto *cb = qobject_cast<QComboBox*>(editor);
                    if (cb) model->setData(mi, cb->currentData(), Qt::EditRole);
                    break;
                }
                case 7:
                case 8: {
                    auto *dte = qobject_cast<QDateTimeEdit*>(editor);
                    if (dte) {
                        // Ambil waktu dari editor, lalu konversi ke UTC
                        QDateTime local = dte->dateTime();
                        QDateTime utc(local.toTimeZone(QTimeZone::UTC));
                        model->setData(mi, utc, Qt::EditRole);
                    }
                    break;
                }
                default: {
                    auto *le = qobject_cast<QLineEdit*>(editor);
                    if (le) model->setData(mi, le->text(), Qt::EditRole);
                    break;
                }
            }
        }
        
        void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &mi) const override {
            // 1. Panggil base class untuk memuat data default (termasuk warna seleksi dll)
            QStyledItemDelegate::initStyleOption(option, mi);

            // 2. Timpa teks dan perataan sesuai kebutuhan kolom
            if(mi.data(Qt::UserRole + 102).toBool()) option->backgroundBrush = QColor(255, 255, 200);
            switch (mi.column()) {
                case 0: {
                    option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
                    break;
                }
                case 4: {
                    option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
                    int value = mi.data(Qt::EditRole).toInt();
                    option->text = QLocale().toString(value); 
                    break;
                }
                case 5: {
                    option->displayAlignment = Qt::AlignCenter;
                    break;
                }
                case 6: {
                    option->displayAlignment = Qt::AlignCenter;
                    // Ambil nilai integer, lalu paksa teks yang tampil menjadi Ya/Tidak
                    int value = mi.data(Qt::EditRole).toInt();
                    option->text = (value == 1) ? "Ya" : "Tidak";
                    break;
                }
                case 7:
                case 8: {
                    QDateTime utcDate = mi.data(Qt::EditRole).toDateTime();
                    if (utcDate.isValid()) {
                        QDateTime localDate(utcDate.toTimeZone(QTimeZone::LocalTime));
                        option->text = localDate.toString("dd/MM/yyyy HH:mm");
                    } else {
                        option->text = "-";
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }
        
        // Definisi dari fungsi yang kamu deklarasikan di akhir
        void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const override {
            editor->setGeometry(option.rect);
        }
    };
}

FinishingServicesViewer::FinishingServicesViewer(QWidget *p) : DataViewer(p) {
  auto ui = Ui();
  auto mod = &model();

  setQueryArgs("SELECT * FROM finishing_services");
  setFilterColumnNames({"code", "name", "description"});
  ui->dataView->setItemDelegate(new Delegate(this));
  
  auto record = mod->record();
  mod->setPrimaryKeyColumn("id");
  mod->setHeaderData(0, Qt::Horizontal, "ID", Qt::DisplayRole);
  mod->setHeaderData(1, Qt::Horizontal, "Kode", Qt::DisplayRole);
  mod->setHeaderData(2, Qt::Horizontal, "Nama", Qt::DisplayRole);
  mod->setHeaderData(3, Qt::Horizontal, "Deskripsi", Qt::DisplayRole);
  mod->setHeaderData(4, Qt::Horizontal, "Harga", Qt::DisplayRole);
  mod->setHeaderData(5, Qt::Horizontal, "Unit", Qt::DisplayRole);
  mod->setHeaderData(6, Qt::Horizontal, "Aktif", Qt::DisplayRole);
  
  setColumnVisible("updated_at", false);
  setColumnVisible("created_at", false);
  setColumnVisible("id", false);
  
  ui->dataView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->dataView, &QTableView::customContextMenuRequested, this, &FinishingServicesViewer::on_dataView_customContextMenuRequested);
  connect(this, &DataViewer::refreshed, ui->dataView, &QTableView::resizeColumnsToContents);
}

FinishingServicesViewer::~FinishingServicesViewer() {}

bool FinishingServicesViewer::initialize(MainWindowContext *ctx)
{
    ctx->addDock(this, "Data Finishing", Qt::TopDockWidgetArea, "top_left");
    return true;
}

void FinishingServicesViewer::on_dataView_customContextMenuRequested(const QPoint& p) {
  auto &m = model();
  QMenu context;
  auto createOne = context.addAction("Buat baru", this, &FinishingServicesViewer::openCreateFinishingDialog);
  context.addSeparator();
  auto simpan = context.addAction("Simpan", &m, &AdvancedQueryModel::submitAll);
  auto revert = context.addAction("Reset",  &m, &AdvancedQueryModel::revertAll);
  context.exec(Ui()->dataView->viewport()->mapToGlobal(p));
}

void FinishingServicesViewer::openCreateFinishingDialog() {
  auto *cfsd = new CreateFinishingServiceDialog(this);
  cfsd->setAttribute(Qt::WA_DeleteOnClose);
  connect(cfsd, &QDialog::accepted, this, &DataViewer::refresh);
  cfsd->open();
}