#include "konsumenrankviewer.h"

#include <QSqlQueryModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QSqlError>
#include <QDebug>
#include <QShortcut>

KonsumenRankViewer::KonsumenRankViewer(QWidget *parent)
    : QWidget(parent), 
      _model(new QSqlQueryModel(this))
{
    // Inisialisasi UI
    auto layout = new QVBoxLayout(this);
    auto view = new QTableView(this);
    
    view->setModel(_model);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->verticalHeader()->hide();
    view->horizontalHeader()->setStretchLastSection(true);
    view->verticalHeader()->setMinimumSectionSize(18);
    view->verticalHeader()->setDefaultSectionSize(18);
    
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(view);

    auto refreshShortcut = new QShortcut(QKeySequence(Qt::Key_F5), this);
    connect(refreshShortcut, &QShortcut::activated, this, &KonsumenRankViewer::fetchData);
    
    fetchData();
}

KonsumenRankViewer::~KonsumenRankViewer()
{
    // _model akan dihapus otomatis karena parent-nya adalah 'this'
}

void KonsumenRankViewer::fetchData()
{
    // Query untuk meranking konsumen berdasarkan total nilai belanja/invoice
    // Sesuaikan kolom dan logika bisnis (misal: hanya hitung invoice yang sudah lunas)
    const QString queryText = R"-(
        SELECT 
            k.nama_lengkap AS "Nama Konsumen",
            k.customer_code AS "Kode",
            COUNT(i.id) AS "Jumlah Invoice",
            SUM(p.amount) AS "Total Pembayaran"
        FROM konsumen k
        JOIN invoices i ON k.id = i.customer_id
        JOIN payments p ON i.id = p.invoice_id
        WHERE p.verification_status = 'verified'
        GROUP BY k.id
        ORDER BY "Total Nilai Belanja" DESC
        LIMIT 50
    )-";

    _model->setQuery(queryText);

    if (_model->lastError().isValid()) {
        qDebug() << "KonsumenRankViewer Error:" << _model->lastError().text();
    } else {
        onDataReady();
    }
}

void KonsumenRankViewer::onDataReady()
{
    // Logika tambahan setelah data dimuat, seperti resize kolom
    // Kita perlu mencari QTableView di layout atau menyimpannya sebagai member
    auto view = findChild<QTableView*>();
    if (view) {
        view->resizeColumnsToContents();
    }
}
