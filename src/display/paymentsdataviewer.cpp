#include "paymentsdataviewer.h"
#include "ui_dataviewer.h"

// TODO: Buat dialog untuk tambah/verifikasi pembayaran
// #include "src/dialogs/paymentdialog.h"

#include <QStyledItemDelegate>
#include <QMenu>
#include <QAction>

namespace {
    class PaymentDelegate : public QStyledItemDelegate {
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
                case 8: {
                    // payment_number, invoice_number, verification_status - tengah
                    option->displayAlignment = Qt::AlignCenter;
                    break;
                }
                case 3:
                case 5:
                case 6: {
                    // amount, cash_received, cash_change - rata kanan + format angka
                    option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
                    option->text = QLocale().toString(ix.data().toLongLong());
                    break;
                }
                case 9: {
                    // payment_date - format tanggal
                    option->displayAlignment = Qt::AlignCenter;
                    QDateTime dt = ix.data().toDateTime();
                    option->text = dt.toString("dd MMMM yyyy HH:mm");
                    break;
                }
                default:
                    break;
            }

            // Highlight warna berdasarkan status verifikasi (kolom 8)
            const QString status = ix.siblingAtColumn(8).data().toString();
            if (status == "cancelled") {
                option->backgroundBrush = QColor(255, 200, 200); // merah muda
            } else if (status == "verified") {
                option->backgroundBrush = QColor(200, 255, 200); // hijau muda
            }
        }
    };
}

PaymentsDataViewer::PaymentsDataViewer(QWidget *parent) : DataViewer(parent)
{
    ui = Ui();
    auto m = &model();

    setQueryArgs(R"--(
        SELECT p.id,
               p.payment_number,
               i.invoice_number,
               p.amount,
               at.nama        AS akun_transaksi,
               p.cash_received,
               p.cash_change,
               p.notes,
               p.verification_status,
               p.payment_date,
               a.nama_lengkap AS kasir
          FROM payments p
          JOIN invoices i        ON p.invoice_id = i.id
          JOIN akun_transaksi at ON p.akun_transaksi_id = at.id
          JOIN admins a          ON p.admin_id = a.id
    )--");

    setFilterColumnNames({"p.payment_number", "i.invoice_number"});
    ui->dataView->setItemDelegate(new PaymentDelegate(this));
    ui->dataView->verticalHeader()->hide();
    ui->dataView->setEditTriggers(QTableView::NoEditTriggers);

    m->setHeaderData(0,  Qt::Horizontal, "ID");
    m->setHeaderData(1,  Qt::Horizontal, "No. Pembayaran");
    m->setHeaderData(2,  Qt::Horizontal, "No. Invoice");
    m->setHeaderData(3,  Qt::Horizontal, "Jumlah");
    m->setHeaderData(4,  Qt::Horizontal, "Akun");
    m->setHeaderData(5,  Qt::Horizontal, "Tunai Diterima");
    m->setHeaderData(6,  Qt::Horizontal, "Kembalian");
    m->setHeaderData(7,  Qt::Horizontal, "Catatan");
    m->setHeaderData(8,  Qt::Horizontal, "Status");
    m->setHeaderData(9,  Qt::Horizontal, "Tanggal Bayar");
    m->setHeaderData(10, Qt::Horizontal, "Kasir");

    ui->dataView->resizeColumnsToContents();
    connect(this, &DataViewer::refreshed, ui->dataView, &QTableView::resizeColumnsToContents);

    ui->dataView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->dataView, &QTableView::customContextMenuRequested,
            this, &PaymentsDataViewer::on_dataView_customContextMenuRequested);

    addPaymentAction(); // inisiasi action
}

PaymentsDataViewer::~PaymentsDataViewer() {}

void PaymentsDataViewer::filterByInvoiceId(int invoiceId)
{
    model().setFilter(QString("p.invoice_id = %1").arg(invoiceId));
    refresh();
}

void PaymentsDataViewer::clearInvoiceFilter()
{
    model().setFilter(QString());
    refresh();
}

QAction *PaymentsDataViewer::addPaymentAction()
{
    if (m_addPaymentAction == nullptr) {
        m_addPaymentAction = new QAction("Pembayaran Baru", this);
        m_addPaymentAction->setObjectName("addPaymentAction");
        m_addPaymentAction->setToolTip("Tambah data pembayaran baru");
        connect(m_addPaymentAction, &QAction::triggered,
                this, &PaymentsDataViewer::onAddPaymentActionTriggered);
    }
    return m_addPaymentAction;
}

void PaymentsDataViewer::onAddPaymentActionTriggered()
{
    openCreatePaymentDialog();
}

void PaymentsDataViewer::openCreatePaymentDialog()
{
    // TODO: implementasi saat dialog tersedia
    // PaymentDialog dlg(this);
    // dlg.prepareCreate();
    // connect(&dlg, &QDialog::accepted, this, &DataViewer::refresh);
    // dlg.setWindowTitle("Form Pembayaran Baru");
    // dlg.exec();
}

void PaymentsDataViewer::openVerifyPaymentDialog(int paymentId)
{
    // TODO: implementasi dialog verifikasi pembayaran
    Q_UNUSED(paymentId)
}

void PaymentsDataViewer::on_dataView_customContextMenuRequested(const QPoint &pt)
{
    QMenu menu;
    menu.setToolTipsVisible(true);

    auto clickedIndex = ui->dataView->indexAt(pt);
    const bool hasSelection = clickedIndex.isValid();
    const QString status = hasSelection
        ? clickedIndex.siblingAtColumn(8).data().toString()
        : QString();

    auto verifyAction = menu.addAction("Verifikasi");
    verifyAction->setToolTip("Verifikasi pembayaran ini");
    verifyAction->setEnabled(hasSelection && status == "pending");

    menu.addSeparator();

    auto submenu = menu.addMenu("Data baru");
    submenu->setToolTipsVisible(true);
    submenu->addAction(addPaymentAction());

    connect(verifyAction, &QAction::triggered, [this, clickedIndex]() {
        if (clickedIndex.isValid())
            openVerifyPaymentDialog(clickedIndex.siblingAtColumn(0).data().toInt());
    });

    menu.exec(ui->dataView->viewport()->mapToGlobal(pt));
}
