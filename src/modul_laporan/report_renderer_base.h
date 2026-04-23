#pragma once

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QTextDocument>
#include <QFont>
#include <QColor>
#include <QPen>
#include <QBrush>
#include <QString>

#include "reporttheme.h"
// ============================================================
//  THEME — satu tempat untuk semua warna & font
// ============================================================

// ============================================================
//  BASE RENDERER
// ============================================================

class ReportRendererBase {
public:
    explicit ReportRendererBase(QGraphicsScene* scene, const ReportTheme& theme = {})
        : m_scene(scene), m_theme(theme) {}

    virtual ~ReportRendererBase() = default;

    // Entry point — subclass implements this
    virtual qreal render(qreal startY = 0.0) = 0;

    const ReportTheme& theme() const { return m_theme; }

protected:
    QGraphicsScene* m_scene;
    ReportTheme     m_theme;

    // ---- primitive helpers ----

    QGraphicsTextItem* addText(const QString& text, qreal x, qreal y,
                               const QFont& font, const QColor& color = Qt::black,
                               qreal maxWidth = 0)
    {
        auto* item = m_scene->addText(text, font);
        item->setDefaultTextColor(color);
        item->setPos(x, y);
        if (maxWidth > 0)
            item->setTextWidth(maxWidth);
        return item;
    }

    void addLine(qreal x1, qreal y1, qreal x2, qreal y2,
                 const QColor& color = Qt::black, qreal width = 0.5)
    {
        auto* line = m_scene->addLine(x1, y1, x2, y2, QPen(color, width));
        Q_UNUSED(line)
    }

    void addRect(qreal x, qreal y, qreal w, qreal h,
                 const QColor& bg, const QColor& border = Qt::transparent,
                 qreal borderWidth = 0.5, qreal radius = 4.0)
    {
        auto* rect = m_scene->addRect(x, y, w, h,
                                      QPen(border, borderWidth),
                                      QBrush(bg));
        rect->setFlag(QGraphicsItem::ItemIsSelectable, false);
        if (radius > 0) {
            // QGraphicsRectItem doesn't support radius natively;
            // use QGraphicsPathItem for rounded corners
            QPainterPath path;
            path.addRoundedRect(QRectF(x, y, w, h), radius, radius);
            m_scene->addPath(path, QPen(border, borderWidth), QBrush(bg));
            m_scene->removeItem(rect);
            delete rect;
        }
    }

    // Badge (pill)
    void addBadge(const QString& text, qreal x, qreal y,
                  const QColor& bg, const QColor& fg)
    {
        QFontMetrics fm(m_theme.fontSmall);
        qreal tw = fm.horizontalAdvance(text);
        qreal bw = tw + 16;
        qreal bh = 14;

        QPainterPath path;
        path.addRoundedRect(QRectF(x, y, bw, bh), 7, 7);
        auto* p = m_scene->addPath(path, QPen(Qt::transparent), QBrush(bg));
        auto* t = m_scene->addText(text, m_theme.fontSmall);
        double x1 = p->boundingRect().x() + (p->boundingRect().width() -t->boundingRect().width()) / 2.0;
        double y1 = p->boundingRect().y() + (p->boundingRect().height() -t->boundingRect().height()) / 2.0;
        t->setDefaultTextColor(fg);
        t->setPos(x1 , y1);
    }

    // Metric card (summary box)
    void addMetricCard(qreal x, qreal y, qreal w,
                       const QString& label, const QString& value,
                       const QColor& valueColor = {})
    {
        addRect(x, y, w, m_theme.cardHeight, m_theme.colorCardBg,
                Qt::transparent, 0, 6);
        addText(label, x + 10, y + 6,
                m_theme.fontMeta, m_theme.colorSecondary);
        addText(value, x + 10, y + 22,
                m_theme.fontBold,
                valueColor.isValid() ? valueColor : m_theme.colorPrimary);
    }

    // Horizontal rule
    void addRule(qreal y, qreal width = 1.0) {
        addLine(m_theme.marginLeft, y,
                m_theme.marginLeft + m_theme.contentWidth, y,
                m_theme.colorPrimary, width);
    }

    void addThinRule(qreal y) {
        addLine(m_theme.marginLeft, y,
                m_theme.marginLeft + m_theme.contentWidth, y,
                m_theme.colorBorder, 0.5);
    }

    // Section title bar
    qreal addSectionTitle(const QString& title, qreal y) {
        addText(title.toUpper(), m_theme.marginLeft, y,
                m_theme.fontSection, m_theme.colorSecondary);
        addThinRule(y + 14 + 6);
        return y + 18;
    }

    // Table header row — cols: list of {label, x, width, align}
    struct ColDef {
        QString label;
        qreal   x;
        qreal   width;
        Qt::Alignment align = Qt::AlignLeft;
    };

    qreal addTableHeader(const QList<ColDef>& cols, qreal y) {
        for (auto& c : cols) {
            auto* t = m_scene->addText(c.label, m_theme.fontTableHead);
            t->setDefaultTextColor(m_theme.colorSecondary);
            t->setTextWidth(c.width);
            QTextOption opt;
            opt.setAlignment(c.align);
            t->document()->setDefaultTextOption(opt);
            t->setPos(c.x, y);
        }
        addThinRule(y + m_theme.rowHeight - 2);
        return y + m_theme.rowHeight;
    }

    // Single table cell
    void addCell(const QString& text, const ColDef& col, qreal y,
                 const QFont& font = {}, const QColor& color = {},
                 bool isGroupRow = false)
    {
        if (isGroupRow)
            addRect(m_theme.marginLeft, y,
                    m_theme.contentWidth, m_theme.rowHeight,
                    m_theme.colorGroupRowBg, Qt::transparent, 0, 0);

        QFont f = font.family().isEmpty() ? m_theme.fontTableBody : font;
        QColor c = color.isValid() ? color : m_theme.colorPrimary;

        auto* t = m_scene->addText(text, f);
        t->setDefaultTextColor(c);
        t->setTextWidth(col.width);
        QTextOption opt;
        opt.setAlignment(col.align);
        t->document()->setDefaultTextOption(opt);
        t->setPos(col.x, y + 2 - 5);
    }

    qreal addTableRow(const QList<ColDef>& cols,
                      const QList<QString>& values,
                      qreal y, bool groupRow = false,
                      bool totalRow = false,
                      const QList<QColor>& colors = {})
    {
        if (totalRow)
            addLine(m_theme.marginLeft, y,
                    m_theme.marginLeft + m_theme.contentWidth, y,
                    m_theme.colorBorderStrong, 1.0);

        for (int i = 0; i < cols.size() && i < values.size(); ++i) {
            QColor c = (colors.size() > i && colors[i].isValid())
                           ? colors[i] : QColor();
            QFont  f = totalRow ? m_theme.fontBold : QFont{};
            addCell(values[i], cols[i], totalRow ? y + 4: y, f, c, groupRow);
        }
        addThinRule(y + m_theme.rowHeight);
        return y + m_theme.rowHeight;
    }

    // Signature block
    qreal addSignatureBlock(qreal y, const QStringList& names,
                            const QStringList& roles)
    {
        qreal colW = m_theme.contentWidth / names.size();
        for (int i = 0; i < names.size(); ++i) {
            qreal cx = m_theme.marginLeft + i * colW + colW * 0.1;
            qreal cw = colW * 0.8;
            addLine(cx, y + 36, cx + cw, y + 36,
                    m_theme.colorBorderStrong, 0.5);
            addText(names[i], cx, y + 40,
                    m_theme.fontBold, m_theme.colorPrimary);
            addText(roles[i], cx, y + 52,
                    m_theme.fontMeta, m_theme.colorSecondary);
        }
        return y + 68;
    }

    // Footer bar
    void addDocFooter(qreal y, const QString& left, const QString& right) {
        addThinRule(y);
        addText(left,  m_theme.marginLeft, y + 4,
                m_theme.fontSmall, m_theme.colorTertiary);
        QFontMetrics fm(m_theme.fontSmall);
        qreal rw = fm.horizontalAdvance(right);
        addText(right,
                m_theme.marginLeft + m_theme.contentWidth - rw,
                y + 4, m_theme.fontSmall, m_theme.colorTertiary);
    }

    // Note box (left-border accent)
    void addNoteBox(const QString& text, qreal y) {
        qreal boxH = 28;
        addRect(m_theme.marginLeft, y,
                m_theme.contentWidth, boxH,
                m_theme.colorCardBg, Qt::transparent, 0, 0);
        // left accent bar
        QPainterPath bar;
        bar.addRect(m_theme.marginLeft, y, 3, boxH);
        m_scene->addPath(bar, QPen(Qt::transparent),
                         QBrush(m_theme.colorBorderStrong));

        auto* t = m_scene->addText(text, m_theme.fontMeta);
        t->setDefaultTextColor(m_theme.colorSecondary);
        t->setTextWidth(m_theme.contentWidth - 16);
        t->setPos(m_theme.marginLeft + 10, y + 6);
    }

    // Render document header (shared by both reports)
    qreal renderDocHeader(const CompanyInfo& co, const ReportMeta& meta,
                          const QString& title, const QString& period,
                          qreal y)
    {
        // Company block (left)
        addText(co.name, m_theme.marginLeft, y,
                m_theme.fontCompany, m_theme.colorPrimary);
        addText(co.address, m_theme.marginLeft, y + 18,
                m_theme.fontSub, m_theme.colorSecondary);
        addText(co.phone + "  |  " + co.email,
                m_theme.marginLeft, y + 30,
                m_theme.fontSub, m_theme.colorSecondary);

        // Meta block (right)
        QStringList metaLines = {
            "No. Dokumen: " + meta.documentNumber,
            "Tanggal cetak: " +
                meta.printedAt.addSecs(7 * 3600)
                    .toString("d MMMM yyyy, HH:mm") + " WIB",
            "Dibuat oleh: " + meta.printedBy
        };
        for (int i = 0; i < metaLines.size(); ++i) {
            QFontMetrics fm(m_theme.fontMeta);
            qreal tw = fm.horizontalAdvance(metaLines[i]);
            addText(metaLines[i],
                    m_theme.marginLeft + m_theme.contentWidth - tw,
                    y + i * 12,
                    m_theme.fontMeta, m_theme.colorSecondary);
        }

        y += 44;
        addRule(y + 8, 1.5);
        y += 8;

        // Title & period (centered)
        {
            QFontMetrics fm(m_theme.fontTitle);
            qreal tw = fm.horizontalAdvance(title);
            addText(title,
                    m_theme.marginLeft + (m_theme.contentWidth - tw) / 2,
                    y, m_theme.fontTitle, m_theme.colorPrimary);
        }
        y += 18;
        {
            QFontMetrics fm(m_theme.fontPeriod);
            qreal tw = fm.horizontalAdvance(period);
            addText(period,
                    m_theme.marginLeft + (m_theme.contentWidth - tw) / 2,
                    y, m_theme.fontPeriod, m_theme.colorSecondary);
        }
        return y + 20;
    }

    static QString formatRp(qint64 amount) {
        return "Rp " + QLocale(QLocale::Indonesian).toString(amount);
    }
};
