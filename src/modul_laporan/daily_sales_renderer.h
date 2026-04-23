#pragma once

#include "report_renderer_base.h"
#include "report_data.h"

class DailySalesRenderer : public ReportRendererBase {
public:
    explicit DailySalesRenderer(QGraphicsScene*       scene,
                                const DailySalesReport& data,
                                const ReportTheme&    theme = {})
        : ReportRendererBase(scene, theme), m_data(data) {}

    qreal render(qreal startY = 0.0) override
    {
        qreal y = startY + 10;

        // --- 1. Header ---
        QString period = "Periode: " +
            QLocale(QLocale::Indonesian)
                .toString(m_data.meta.periodDate, "dddd, d MMMM yyyy") +
            "  (00:00 – 23:59 WIB)";

        y = renderDocHeader(m_data.company, m_data.meta,
                            "Laporan Penjualan Harian", period, y);
        y += 8;

        // --- 2. Summary cards ---
        y = renderSummaryCards(y);
        y += m_theme.sectionGap;

        // --- 3. Order detail table ---
        y = addSectionTitle("Detail order hari ini", y);
        y = renderOrderTable(y);
        y += m_theme.sectionGap;

        // --- 4. Payment method table ---
        y = addSectionTitle("Rekap pembayaran per metode", y);
        y = renderPaymentTable(y);
        y += m_theme.sectionGap;

        // --- 5. Top products table ---
        y = addSectionTitle("Produk terlaris hari ini", y);
        y = renderTopProductsTable(y);
        y += m_theme.sectionGap;

        // --- 6. Notes ---
        if (!m_data.notes.isEmpty()) {
            addNoteBox(m_data.notes, y);
            y += 38;
        }

        // --- 7. Signatures ---
        y = addSignatureBlock(y,
            { "Nur Holis K.", "(                )", "(                )" },
            { "Dibuat oleh",  "Diperiksa oleh",    "Disetujui oleh"     });
        y += 8;

        // --- 8. Footer ---
        addDocFooter(y,
            "Dicetak otomatis oleh sistem POS Percetakan Maju Jaya",
            "Halaman 1 dari 1");

        m_scene->setSceneRect(0, 0, m_theme.pageWidth, y + 30);
        return y + 30;
    }

private:
    const DailySalesReport& m_data;

    qreal renderSummaryCards(qreal y)
    {
        qreal cw = (m_theme.contentWidth - m_theme.cardGap * 3) / 4;
        auto card = [&](int i, const QString& label,
                        const QString& value, const QColor& vc = {}) {
            addMetricCard(
                m_theme.marginLeft + i * (cw + m_theme.cardGap), y, cw,
                label, value, vc);
        };

        card(0, "Total order",
             QString::number(m_data.summary.totalOrders));
        card(1, "Total omzet",
             formatRp(m_data.summary.totalRevenue));
        card(2, "Sudah lunas",
             formatRp(m_data.summary.paidAmount),
             m_theme.colorSuccess);
        card(3, "Belum lunas",
             formatRp(m_data.summary.unpaidAmount),
             m_theme.colorWarning);

        return y + m_theme.cardHeight + 4;
    }

    qreal renderOrderTable(qreal y)
    {
        const qreal L = m_theme.marginLeft;
        QList<ColDef> cols = {
            { "No. order",        L,       100, Qt::AlignLeft  },
            { "Pelanggan",        L+105,   110, Qt::AlignLeft  },
            { "Produk",           L+220,   140, Qt::AlignLeft  },
            { "Status produksi",  L+365,    80, Qt::AlignLeft  },
            { "Total (Rp)",       L+450,    90, Qt::AlignRight },
            { "Pembayaran",       L+545,    70, Qt::AlignLeft  },
        };
        y = addTableHeader(cols, y);

        for (const auto& row : m_data.orders) {
            // production status badge
            addCell(row.orderNumber,    cols[0], y);
            addCell(row.customerName,   cols[1], y);
            addCell(row.productSummary, cols[2], y);

            // production badge
            addStatusBadge(row.productionStatus,
                           cols[3].x + 2, y + 3);

            addCell(QLocale(QLocale::Indonesian).toString(row.totalAmount),
                    cols[4], y, {}, {});

            // payment badge
            addPaymentBadge(row.paymentStatus,
                            cols[5].x + 2, y + 3);

            addThinRule(y + m_theme.rowHeight);
            y += m_theme.rowHeight;
        }

        // total row
        qint64 total = 0;
        for (const auto& r : m_data.orders) total += r.totalAmount;
        addTableRow(cols,
                    { "Total", "", "", "", formatRp(total), "" },
                    y, false, true);
        return y + m_theme.rowHeight + 4;
    }

    qreal renderPaymentTable(qreal y)
    {
        const qreal L = m_theme.marginLeft;
        QList<ColDef> cols = {
            { "Metode",             L,     250, Qt::AlignLeft  },
            { "Jumlah transaksi",   L+255, 150, Qt::AlignRight },
            { "Total (Rp)",         L+410, 100, Qt::AlignRight },
        };
        y = addTableHeader(cols, y);

        qint64 grandTotal = 0;
        int    grandCount = 0;
        for (const auto& row : m_data.paymentMethods) {
            y = addTableRow(cols, {
                row.methodName,
                QString::number(row.txCount),
                QLocale(QLocale::Indonesian).toString(row.amount)
            }, y);
            grandTotal += row.amount;
            grandCount += row.txCount;
        }
        addTableRow(cols,
                    { "Total diterima",
                      QString::number(grandCount),
                      formatRp(grandTotal) },
                    y, false, true);
        return y + m_theme.rowHeight + 4;
    }

    qreal renderTopProductsTable(qreal y)
    {
        const qreal L = m_theme.marginLeft;
        QList<ColDef> cols = {
            { "Produk",       L,     160, Qt::AlignLeft  },
            { "Kategori",     L+165, 130, Qt::AlignLeft  },
            { "Qty terjual",  L+300, 100, Qt::AlignRight },
            { "Omzet (Rp)",   L+405, 100, Qt::AlignRight },
        };
        y = addTableHeader(cols, y);

        for (const auto& row : m_data.topProducts) {
            y = addTableRow(cols, {
                row.productName,
                row.categoryName,
                row.qtySold,
                QLocale(QLocale::Indonesian).toString(row.revenue)
            }, y);
        }
        return y + 4;
    }

    void addStatusBadge(const QString& status, qreal x, qreal y) {
        QString label;
        QColor bg, fg;
        if (status == "completed") {
            label = "Selesai";
            bg = m_theme.colorBadgeDoneBg; fg = m_theme.colorBadgeDoneFg;
        } else if (status == "processing" || status == "ready") {
            label = "Proses";
            bg = m_theme.colorBadgeProcBg; fg = m_theme.colorBadgeProcFg;
        } else {
            label = "Pending";
            bg = m_theme.colorBadgePendBg; fg = m_theme.colorBadgePendFg;
        }
        addBadge(label, x, y, bg, fg);
    }

    void addPaymentBadge(const QString& status, qreal x, qreal y) {
        QString label;
        QColor bg, fg;
        if (status == "paid") {
            label = "Lunas";
            bg = m_theme.colorBadgeDoneBg; fg = m_theme.colorBadgeDoneFg;
        } else if (status == "partial") {
            label = "Sebagian";
            bg = m_theme.colorBadgeProcBg; fg = m_theme.colorBadgeProcFg;
        } else {
            label = "Belum";
            bg = m_theme.colorBadgePendBg; fg = m_theme.colorBadgePendFg;
        }
        addBadge(label, x, y, bg, fg);
    }
};
