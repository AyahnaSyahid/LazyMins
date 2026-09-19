#pragma once

#include "report_data.h"
#include "report_renderer_base.h"

// Renders a period (range) expense report from RangeExpenseReport.
class RangeExpenseRenderer : public ReportRendererBase {
 public:
  explicit RangeExpenseRenderer(QGraphicsScene* scene,
                                const RangeExpenseReport& data,
                                const ReportTheme& theme = {})
      : ReportRendererBase(scene, theme), m_data(data) {}

  qreal render(qreal startY = 0.0) override {
    qreal y = startY + 10;

    // --- 1. Header ---
    QString period = "Periode: " +
        m_data.meta.periodRange.toString() +
        "  (" + QString::number(m_data.meta.periodRange.days()) + " hari)";

    y = renderDocHeader(m_data.company, m_data.meta,
                        "Laporan Belanja & Pengeluaran Periode", period, y);
    y += 8;

    // --- 2. Summary cards ---
    y = renderSummaryCards(y);
    y += m_theme.sectionGap;

    // --- 3. By category ---
    y = addSectionTitle("Rekap per kategori", y);
    y = renderCategoryTable(y);
    y += m_theme.sectionGap;

    // --- 4. By account ---
    y = addSectionTitle("Rekap per akun / sumber dana", y);
    y = renderAccountTable(y);
    y += m_theme.sectionGap;

    // --- 5. Notes ---
    if (!m_data.notes.isEmpty()) {
      addNoteBox(m_data.notes, y);
      y += 38;
    }

    // --- 6. Signatures ---
    y = addSignatureBlock(
        y, {m_data.meta.printedBy, "(                )", "(                )"},
        {"Dibuat oleh", "Diperiksa oleh", "Disetujui oleh"});
    y += 8;

    // --- 7. Footer ---
    addDocFooter(y, "Dicetak otomatis oleh sistem POS " + m_data.company.name,
                 "Halaman 1 dari 1");

    m_scene->setSceneRect(0, 0, m_theme.pageWidth, y + 30);
    return y + 30;
  }

 private:
  const RangeExpenseReport& m_data;

  qreal renderSummaryCards(qreal y) {
    qreal cw = (m_theme.contentWidth - m_theme.cardGap * 2) / 3;
    addMetricCard(m_theme.marginLeft, y, cw,
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

  qreal renderCategoryTable(qreal y) {
    const qreal L = m_theme.marginLeft;
    QList<ColDef> cols = {
        {"Kategori",         L,     220, Qt::AlignLeft  },
        {"Jumlah transaksi", L + 220, 140, Qt::AlignRight },
        {"Total (Rp)",       L + 370, 110, Qt::AlignRight },
        {"%",                L + 485,  75, Qt::AlignRight },
    };
    y = addTableHeader(cols, y);

    int grandCount = 0;
    qint64 grandTotal = 0;
    for (const auto& row : m_data.byCategory) {
      y = addTableRow(cols, {
          row.categoryName,
          QString::number(row.txCount),
          QLocale(QLocale::Indonesian).toString(row.amount),
          QString::number(qRound(row.percent)) + "%"},
          y);
      grandCount += row.txCount;
      grandTotal += row.amount;
    }
    addTableRow(cols, {"Total", QString::number(grandCount),
                       formatRp(grandTotal), "100%"}, y, false, true);
    return y + m_theme.rowHeight + 4;
  }

  qreal renderAccountTable(qreal y) {
    const qreal L = m_theme.marginLeft;
    QList<ColDef> cols = {
        {"Akun",              L,     200, Qt::AlignLeft  },
        {"Jumlah transaksi",  L + 220, 140, Qt::AlignRight },
        {"Total keluar (Rp)", L + 370, 120, Qt::AlignRight },
    };
    y = addTableHeader(cols, y);

    int grandCount = 0;
    qint64 grandTotal = 0;
    for (const auto& row : m_data.byAccount) {
      y = addTableRow(cols, {
          row.accountName,
          QString::number(row.txCount),
          QLocale(QLocale::Indonesian).toString(row.amount)},
          y);
      grandCount += row.txCount;
      grandTotal += row.amount;
    }
    addTableRow(cols, {"Total", QString::number(grandCount),
                       formatRp(grandTotal)}, y, false, true);
    return y + m_theme.rowHeight + 4;
  }
};
