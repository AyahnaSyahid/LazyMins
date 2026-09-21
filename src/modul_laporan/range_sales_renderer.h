#pragma once

#include "report_data.h"
#include "report_renderer_base.h"

// Renders a period (range) sales report from RangeSalesReport.
class RangeSalesRenderer : public ReportRendererBase {
 public:
  explicit RangeSalesRenderer(QGraphicsScene* scene,
                              const RangeSalesReport& data,
                              const ReportTheme& theme = {})
      : ReportRendererBase(scene, theme), m_data(data) {}

  qreal render(qreal startY = 0.0) override {
    qreal y = startY + 10;

    // --- 1. Header ---
    QString period = "Periode: " +
        m_data.meta.periodRange.toString() +
        "  (" + QString::number(m_data.meta.periodRange.days()) + " hari)";

    y = renderDocHeader(m_data.company, m_data.meta,
                        "Laporan Penjualan Periode", period, y);
    y += 8;

    // --- 2. Summary cards ---
    y = renderSummaryCards(y);
    y += m_theme.sectionGap;

    // --- 3. Top orders table ---
    y = addSectionTitle("Order terlaris periode ini", y);
    y = renderTopOrdersTable(y);
    y += m_theme.sectionGap;

    // --- 4. Payment method table ---
    y = addSectionTitle("Rekap pembayaran per metode", y);
    y = renderPaymentTable(y);
    y += m_theme.sectionGap;

    // --- 5. Top products table ---
    y = addSectionTitle("Produk terlaris periode ini", y);
    y = renderTopProductsTable(y);
    y += m_theme.sectionGap;

    // --- 6. Sales trend ---
    if (!m_data.trend.isEmpty()) {
      y = addSectionTitle("Tren penjualan harian", y);
      y = renderTrendTable(y);
      y += m_theme.sectionGap;
    }

    // --- 7. Notes ---
    if (!m_data.notes.isEmpty()) {
      addNoteBox(m_data.notes, y);
      y += 38;
    }

    // --- 8. Signatures ---
    y = addSignatureBlock(
        y, {m_data.meta.printedBy, "(                )", "(                )"},
        {"Dibuat oleh", "Diperiksa oleh", "Disetujui oleh"});
    y += 8;

    // --- 9. Footer ---
    addDocFooter(y, "Disusun oleh sistem POS " + m_data.company.name,
                 "Halaman 1 dari 1");

    m_scene->setSceneRect(0, 0, m_theme.pageWidth + m_theme.marginRight,
                          y + 30);
    return y + 30;
  }

 private:
  const RangeSalesReport& m_data;

  qreal renderSummaryCards(qreal y) {
    qreal cw = (m_theme.contentWidth - m_theme.cardGap * 3) / 4;
    auto card = [&](int i, const QString& label, const QString& value,
                    const QColor& vc = {}) {
      addMetricCard(m_theme.marginLeft + i * (cw + m_theme.cardGap), y, cw,
                    label, value, vc);
    };

    card(0, "Total order", QString::number(m_data.summary.totalOrders));
    card(1, "Total omzet", formatRp(m_data.summary.totalRevenue));
    card(2, "Sudah lunas", formatRp(m_data.summary.paidAmount),
         m_theme.colorSuccess);
    card(3, "Belum lunas", formatRp(m_data.summary.unpaidAmount),
         m_theme.colorWarning);

    return y + m_theme.cardHeight + 4;
  }

  qreal renderTopOrdersTable(qreal y) {
    const qreal L = m_theme.marginLeft;
    QList<ColDef> cols = {
        {"No. Order",       L,       100, Qt::AlignLeft  },
        {"Pelanggan",       L + 112, 110, Qt::AlignLeft  },
        {"Produk",          L + 234, 165, Qt::AlignLeft  },
        {"Status Produksi", L + 411,  90, Qt::AlignCenter},
        {"Total (Rp)",      L + 513,  90, Qt::AlignRight },
        {"Pembayaran",      L + 615,  90, Qt::AlignLeft  },
    };
    y = addTableHeader(cols, y);

    qint64 total = 0;
    for (const auto& row : m_data.topOrders) {
      addCell(row.orderNumber.mid(4), cols[0], y);
      addCell(row.customerName.mid(0, 15), cols[1], y, {}, {}, false);
      addCell(row.productSummary, cols[2], y);
      addStatusBadge(row.productionStatus, cols[3].x + 20, y + 3);
      addCell(QLocale(QLocale::Indonesian).toString(row.totalAmount),
              cols[4], y, {}, {});
      addPaymentBadge(row.paymentStatus, cols[5].x + 17, y + 3);
      addThinRule(y + m_theme.rowHeight);
      y += m_theme.rowHeight;
      total += row.totalAmount;
    }

    // "Lainnya" (orders beyond top 10)
    if (m_data.otherOrderCount > 0) {
      addCell("Lainnya (" + QString::number(m_data.otherOrderCount) + ")",
              cols[0], y, m_theme.fontBold, m_theme.colorSecondary);
      addCell("", cols[1], y);
      addCell("", cols[2], y);
      addCell("", cols[3], y);
      addCell(formatRp(m_data.otherOrderTotal), cols[4], y,
              m_theme.fontBold, m_theme.colorSecondary, true);
      addCell("", cols[5], y);
      addThinRule(y + m_theme.rowHeight);
      y += m_theme.rowHeight;
      total += m_data.otherOrderTotal;
    }

    addTableRow(cols, {"Total", "", "", "", formatRp(total), ""}, y, false, true);
    return y + m_theme.rowHeight + 4;
  }

  qreal renderPaymentTable(qreal y) {
    const qreal L = m_theme.marginLeft;
    QList<ColDef> cols = {
        {"Metode",        L,       250, Qt::AlignLeft  },
        {"Jumlah transaksi", L + 255, 150, Qt::AlignRight },
        {"Total (Rp)",    L + 410, 100, Qt::AlignRight },
    };
    y = addTableHeader(cols, y);

    qint64 grandTotal = 0;
    int grandCount = 0;
    for (const auto& row : m_data.paymentMethods) {
      y = addTableRow(cols,
                      {row.methodName, QString::number(row.txCount),
                       QLocale(QLocale::Indonesian).toString(row.amount)},
                      y);
      grandTotal += row.amount;
      grandCount += row.txCount;
    }
    addTableRow(cols, {"Total diterima", QString::number(grandCount),
                       formatRp(grandTotal)}, y, false, true);
    return y + m_theme.rowHeight + 4;
  }

  qreal renderTopProductsTable(qreal y) {
    const qreal L = m_theme.marginLeft;
    QList<ColDef> cols = {
        {"Produk",     L,       160, Qt::AlignLeft  },
        {"Kategori",   L + 165, 130, Qt::AlignLeft  },
        {"Qty terjual", L + 300, 100, Qt::AlignRight },
        {"Omzet (Rp)", L + 405, 100, Qt::AlignRight },
    };
    y = addTableHeader(cols, y);

    for (const auto& row : m_data.topProducts) {
      y = addTableRow(cols, {row.productName, row.categoryName, row.qtySold,
                             QLocale(QLocale::Indonesian).toString(row.revenue)},
                      y);
    }
    return y + 4;
  }

  qreal renderTrendTable(qreal y) {
    const qreal L = m_theme.marginLeft;
    QList<ColDef> cols = {
        {"Tanggal",     L,       180, Qt::AlignLeft  },
        {"Order",       L + 185, 100, Qt::AlignRight },
        {"Omzet (Rp)",  L + 290, 100, Qt::AlignRight },
    };
    y = addTableHeader(cols, y);

    qint64 grandTotal = 0;
    int grandCount = 0;
    for (const auto& row : m_data.trend) {
      y = addTableRow(cols,
                      {QLocale(QLocale::Indonesian).toString(row.date,
                                                             "ddd, d MMM yyyy"),
                       QString::number(row.orderCount),
                       QLocale(QLocale::Indonesian).toString(row.revenue)},
                      y);
      grandTotal += row.revenue;
      grandCount += row.orderCount;
    }
    addTableRow(cols, {"Total", QString::number(grandCount),
                       formatRp(grandTotal)}, y, false, true);
    return y + m_theme.rowHeight + 4;
  }

  // ---- helpers (mirrors DailySalesRenderer badges) ----
  void addStatusBadge(const QString& status, qreal x, qreal y) {
    QString label;
    QColor bg, fg;
    if (status == "completed") {
      label = "Selesai";
      bg = m_theme.colorBadgeDoneBg;
      fg = m_theme.colorBadgeDoneFg;
    } else if (status == "processing" || status == "ready") {
      label = "Proses";
      bg = m_theme.colorBadgeProcBg;
      fg = m_theme.colorBadgeProcFg;
    } else {
      label = "Pending";
      bg = m_theme.colorBadgePendBg;
      fg = m_theme.colorBadgePendFg;
    }
    addBadge(label, x, y, bg, fg);
  }

  void addPaymentBadge(const QString& status, qreal x, qreal y) {
    QString label;
    QColor bg, fg;
    if (status == "paid") {
      label = "Lunas";
      bg = m_theme.colorBadgeDoneBg;
      fg = m_theme.colorBadgeDoneFg;
    } else if (status == "partial") {
      label = "Sebagian";
      bg = m_theme.colorBadgeProcBg;
      fg = m_theme.colorBadgeProcFg;
    } else {
      label = "Belum";
      bg = m_theme.colorBadgePendBg;
      fg = m_theme.colorBadgePendFg;
    }
    addBadge(label, x, y, bg, fg);
  }
};
