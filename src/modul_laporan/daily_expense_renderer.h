#pragma once

#include "report_renderer_base.h"
#include "report_data.h"

class DailyExpenseRenderer : public ReportRendererBase {
public:
    explicit DailyExpenseRenderer(QGraphicsScene*         scene,
                                  const DailyExpenseReport& data,
                                  const ReportTheme&      theme = {})
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
                            "Laporan Belanja & Pengeluaran Harian",
                            period, y);
        y += 8;

        // --- 2. Summary cards ---
        y = renderSummaryCards(y);
        y += m_theme.sectionGap;

        // --- 3. Expense detail table ---
        y = addSectionTitle("Detail transaksi pengeluaran", y);
        y = renderExpenseTable(y);
        y += m_theme.sectionGap;

        // --- 4. By category ---
        y = addSectionTitle("Rekap per kategori", y);
        y = renderCategoryTable(y);
        y += m_theme.sectionGap;

        // --- 5. By account ---
        y = addSectionTitle("Rekap per akun / sumber dana", y);
        y = renderAccountTable(y);
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
    const DailyExpenseReport& m_data;

    qreal renderSummaryCards(qreal y)
    {
        qreal cw = (m_theme.contentWidth - m_theme.cardGap * 2) / 3;

        addMetricCard(m_theme.marginLeft,                     y, cw,
                      "Total pengeluaran",
                      formatRp(m_data.summary.totalExpense),
                      m_theme.colorDanger);

        addMetricCard(m_theme.marginLeft + cw + m_theme.cardGap, y, cw,
                      "Beli bahan baku",
                      formatRp(m_data.summary.materialExpense),
                      m_theme.colorWarning);

        addMetricCard(m_theme.marginLeft + (cw + m_theme.cardGap) * 2, y, cw,
                      "Biaya operasional",
                      formatRp(m_data.summary.opsExpense));

        return y + m_theme.cardHeight + 4;
    }

    qreal renderExpenseTable(qreal y)
    {
        const qreal L = m_theme.marginLeft;
        QList<ColDef> cols = {
            { "No. transaksi", L,      100, Qt::AlignLeft  },
            { "Deskripsi",     L+105,  130, Qt::AlignLeft  },
            { "Kategori",      L+240,   90, Qt::AlignLeft  },
            { "Akun",          L+335,   90, Qt::AlignLeft  },
            { "Dicatat oleh",  L+430,   70, Qt::AlignLeft  },
            { "Jumlah (Rp)",   L+505,   95, Qt::AlignRight },
        };
        y = addTableHeader(cols, y);

        // Group rows by categoryType
        QString lastGroup;
        qint64  grandTotal = 0;

        for (const auto& row : m_data.expenses) {
            QString groupLabel = groupLabelFor(row.categoryType);
            if (groupLabel != lastGroup) {
                // group header
                for (int i = 0; i < cols.size(); ++i)
                    addCell(i == 0 ? groupLabel : "",
                            cols[i], y, m_theme.fontBold,
                            m_theme.colorSecondary, true);
                addThinRule(y + m_theme.rowHeight);
                y += m_theme.rowHeight;
                lastGroup = groupLabel;
            }

            addCell(row.txNumber,    cols[0], y);
            addCell(row.description, cols[1], y);

            // category badge
            auto [bg, fg] = badgeColors(row.categoryType);
            addBadge(row.categoryName, cols[2].x + 2, y + 3, bg, fg);

            addCell(row.accountName,  cols[3], y);
            addCell(row.recordedBy,   cols[4], y);
            addCell(QLocale(QLocale::Indonesian).toString(row.amount),
                    cols[5], y);

            addThinRule(y + m_theme.rowHeight);
            y += m_theme.rowHeight;
            grandTotal += row.amount;
        }

        // total row
        addTableRow(cols,
                    { "Total pengeluaran", "", "", "", "",
                      formatRp(grandTotal) },
                    y, false, true);
        return y + m_theme.rowHeight + 4;
    }

    qreal renderCategoryTable(qreal y)
    {
        const qreal L = m_theme.marginLeft;
        QList<ColDef> cols = {
            { "Kategori",          L,     220, Qt::AlignLeft  },
            { "Jumlah transaksi",  L+225, 140, Qt::AlignRight },
            { "Total (Rp)",        L+370, 110, Qt::AlignRight },
            { "%",                 L+485,  75, Qt::AlignRight },
        };
        y = addTableHeader(cols, y);

        int    grandCount = 0;
        qint64 grandTotal = 0;

        for (const auto& row : m_data.byCategory) {
            y = addTableRow(cols, {
                row.categoryName,
                QString::number(row.txCount),
                QLocale(QLocale::Indonesian).toString(row.amount),
                QString::number(qRound(row.percent)) + "%"
            }, y);
            grandCount += row.txCount;
            grandTotal += row.amount;
        }
        addTableRow(cols,
                    { "Total",
                      QString::number(grandCount),
                      formatRp(grandTotal),
                      "100%" },
                    y, false, true);
        return y + m_theme.rowHeight + 4;
    }

    qreal renderAccountTable(qreal y)
    {
        const qreal L = m_theme.marginLeft;
        QList<ColDef> cols = {
            { "Akun",              L,     240, Qt::AlignLeft  },
            { "Jumlah transaksi",  L+245, 150, Qt::AlignRight },
            { "Total keluar (Rp)", L+400, 120, Qt::AlignRight },
        };
        y = addTableHeader(cols, y);

        int    grandCount = 0;
        qint64 grandTotal = 0;

        for (const auto& row : m_data.byAccount) {
            y = addTableRow(cols, {
                row.accountName,
                QString::number(row.txCount),
                QLocale(QLocale::Indonesian).toString(row.amount)
            }, y);
            grandCount += row.txCount;
            grandTotal += row.amount;
        }
        addTableRow(cols,
                    { "Total",
                      QString::number(grandCount),
                      formatRp(grandTotal) },
                    y, false, true);
        return y + m_theme.rowHeight + 4;
    }

    // ---- helpers ----

    static QString groupLabelFor(const QString& type) {
        if (type == "material") return "Bahan baku / stok";
        if (type == "ops")      return "Operasional";
        return "Lain-lain";
    }

    std::pair<QColor,QColor> badgeColors(const QString& type) const {
        if (type == "material")
            return { m_theme.colorBadgeMaterialBg, m_theme.colorBadgeMaterialFg };
        if (type == "ops")
            return { m_theme.colorBadgeOpsBg, m_theme.colorBadgeOpsFg };
        return { m_theme.colorBadgeOtherBg, m_theme.colorBadgeOtherFg };
    }
};
