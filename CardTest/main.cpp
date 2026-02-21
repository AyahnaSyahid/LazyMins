#include <QApplication>
#include <QMainWindow>
#include <QListView>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

#include "cardmodel.h"
#include "carddelegate.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // ── Styling dasar untuk window & list ────────────────────────────────
    app.setStyleSheet(R"(
        QMainWindow, QWidget#central {
            background-color: #131926;
        }
        QListView {
            background-color: #131926;
            border: none;
            outline: none;
        }
        QListView::item {
            background: transparent;
            border: none;
        }
        QListView::item:selected {
            background: transparent;
        }
        QScrollBar:vertical {
            background: #1a2035;
            width: 6px;
            border-radius: 3px;
        }
        QScrollBar::handle:vertical {
            background: #2d3a52;
            border-radius: 3px;
        }
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical { height: 0; }
    )");

    // ── Window utama ─────────────────────────────────────────────────────
    QMainWindow window;
    window.setWindowTitle("POS Dashboard — Summary");
    window.resize(420, 720);

    auto *central = new QWidget();
    central->setObjectName("central");
    window.setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header label
    auto *header = new QLabel("  DASHBOARD");
    header->setStyleSheet(R"(
        color: #8892a4;
        font-size: 11px;
        font-weight: 700;
        letter-spacing: 3px;
        padding: 16px 20px 8px 20px;
        background: #131926;
    )");
    mainLayout->addWidget(header);

    // ── ListView ─────────────────────────────────────────────────────────
    auto *listView = new QListView();
    listView->setUniformItemSizes(false);
    listView->setSpacing(0);
    listView->setSelectionMode(QAbstractItemView::SingleSelection);
    listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // ── Model ─────────────────────────────────────────────────────────────
    auto *model = new CardModel(&window);

    // ── Isi data contoh ───────────────────────────────────────────────────

    // Section: Penjualan
    model->addItem(CardItem::make(ItemType::Divider)
        .setTitle("Penjualan Hari Ini"));

    model->addItem(CardItem::make(ItemType::Hero)
        .setTitle("Total Omset")
        .setValue("Rp 12.450.000")
        .setSubtext("↑ 12% lebih tinggi dari kemarin")
        .setColor("#10b981")
        .setIcon("💰"));

    model->addItem(CardItem::make(ItemType::Stat)
        .setTitle("Total Transaksi")
        .setValue("148 Trx")
        .setSubtext("Rata-rata Rp 84.120 / transaksi")
        .setColor("#3b82f6")
        .setIcon("🧾"));

    model->addItem(CardItem::make(ItemType::Stat)
        .setTitle("Total Item Terjual")
        .setValue("1.024 pcs")
        .setSubtext("Dari 87 produk berbeda")
        .setColor("#8b5cf6")
        .setIcon("📦"));

    // Section: Ringkasan
    model->addItem(CardItem::make(ItemType::Divider)
        .setTitle("Ringkasan Cepat"));

    model->addItem(CardItem::make(ItemType::Compact)
        .setTitle("Pelanggan Baru")
        .setValue("23")
        .setSubtext("vs 18 kemarin")
        .setColor("#f59e0b")
        .setIcon("👤"));

    model->addItem(CardItem::make(ItemType::Compact)
        .setTitle("Avg. Waktu Transaksi")
        .setValue("4m 32s")
        .setSubtext("Lebih cepat 30s")
        .setColor("#06b6d4")
        .setIcon("⏱"));

    model->addItem(CardItem::make(ItemType::Compact)
        .setTitle("Metode: QRIS")
        .setValue("61%")
        .setSubtext("90 dari 148 transaksi")
        .setColor("#a78bfa")
        .setIcon("📱"));

    model->addItem(CardItem::make(ItemType::Compact)
        .setTitle("Metode: Tunai")
        .setValue("31%")
        .setSubtext("46 dari 148 transaksi")
        .setColor("#34d399")
        .setIcon("💵"));

    // Section: Peringatan
    model->addItem(CardItem::make(ItemType::Divider)
        .setTitle("Perlu Perhatian"));

    model->addItem(CardItem::make(ItemType::Alert)
        .setTitle("Stok Hampir Habis")
        .setValue("7 Produk")
        .setSubtext("Segera lakukan restock sebelum kehabisan")
        .setColor("#ef4444")
        .setIcon("⚠️"));

    model->addItem(CardItem::make(ItemType::Alert)
        .setTitle("Pending Order")
        .setValue("3 Order")
        .setSubtext("Menunggu konfirmasi lebih dari 30 menit")
        .setColor("#f97316")
        .setIcon("🕐"));

    // ── Delegate ──────────────────────────────────────────────────────────
    auto *delegate = new CardDelegate(&window);

    listView->setModel(model);
    listView->setItemDelegate(delegate);
    mainLayout->addWidget(listView);

    window.show();
    return app.exec();
}
