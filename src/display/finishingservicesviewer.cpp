#include "finishingservicesviewer.h"
#include "ui_dataviewer.h"
#include <QStyledItemDelegate>

#include <QStyledItemDelegate>
#include <QSpinBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QLineEdit>
#include <QPainter>

namespace {
    class DelegateFinishingServicesViewer : public QStyledItemDelegate
    {
    public:
        DelegateFinishingServicesViewer(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
        ~DelegateFinishingServicesViewer() override = default;

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
                    if (le) le->setText(mi.data(Qt::EditRole).toString());
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
        /**
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &mi) const override {
            // Copy option bawaan agar kita bisa memodifikasi teks dan perataannya
            QStyleOptionViewItem opt = option;
            initStyleOption(&opt, mi); // Memuat data default dari model ke 'opt'

            switch (mi.column()) {
                case 0: {
                    // Hanya perataan kanan
                    opt.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
                    break;
                }
                case 4: {
                    // Perataan kanan dan tampilkan pemisah ribuan (Group Separator)
                    opt.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
                    int value = mi.data(Qt::EditRole).toInt();
                    opt.text = QLocale().toString(value); 
                    break;
                }
                case 6: {
                    // Tampilkan "Ya" atau "Tidak" di tengah, sesuai nilai integer di model
                    opt.displayAlignment = Qt::AlignCenter;
                    int value = mi.data(Qt::EditRole).toInt();
                    opt.text = value ? "Ya" : "Tidak";
                    break;
                }
                case 7:
                case 8: {
                    // Konversi dari UTC (di model) ke Local Time untuk ditampilkan
                    QDateTime utcDate = mi.data(Qt::EditRole).toDateTime();
                    if (utcDate.isValid()) {
                        QDateTime localDate(utcDate.toTimeZone(QTimeZone::LocalTime));
                        // Kamu bisa menyesuaikan format string ini sesuai selera
                        opt.text = localDate.toString("dd/MM/yyyy HH:mm"); 
                    } else {
                        opt.text = "-"; // Jika tanggal kosong/tidak valid
                    }
                    break;
                }
                default: {
                    // Biarkan default untuk QLineEdit (biasanya AlignLeft)
                    break;
                }
            }

            // Biarkan base class yang menggambar semuanya dengan 'opt' yang sudah kita modifikasi
            QStyledItemDelegate::paint(painter, opt, mi);
        }
        **/
        
        void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &mi) const override {
            // 1. Panggil base class untuk memuat data default (termasuk warna seleksi dll)
            QStyledItemDelegate::initStyleOption(option, mi);

            // 2. Timpa teks dan perataan sesuai kebutuhan kolom
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
  ui->dataView->setItemDelegate(new DelegateFinishingServicesViewer(this));
}
FinishingServicesViewer::~FinishingServicesViewer() {}