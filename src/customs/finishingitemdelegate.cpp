#include "finishingitemdelegate.h"

#include <QPainter>
#include <QLocale>
#include <QStyleOptionViewItem>
#include <QApplication>

FinishingItemDelegate::FinishingItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

// ── helpers ──────────────────────────────────────────────────────────────────

QString FinishingItemDelegate::formatRp(int value)
{
    return QLocale(QLocale::Indonesian, QLocale::Indonesia)
               .toCurrencyString(value, "Rp ");
}

// ── sizeHint ─────────────────────────────────────────────────────────────────

QSize FinishingItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                      const QModelIndex &) const
{
    QFontMetrics fmNormal(option.font);
    QFont boldFont = option.font;
    boldFont.setBold(true);
    QFontMetrics fmBold(boldFont);

    int h = kPadV
            + fmBold.height()    // line 1
            + kSpacing
            + fmNormal.height()  // line 2
            + kPadV;

    return QSize(option.rect.width(), h);
}

// ── paint ────────────────────────────────────────────────────────────────────

void FinishingItemDelegate::paint(QPainter *painter,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    // ── pull data from model ──────────────────────────────────────────────────
    const QString name  = index.data(Qt::UserRole + 4).toString();
    const int     qty   = index.data(Qt::UserRole + 5).toInt();
    const int     price = index.data(Qt::UserRole + 6).toInt();
    const int     total = index.data(Qt::UserRole + 7).toInt();

    // ── setup style option (handles selection, hover, focus) ─────────────────
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    opt.text = "";   // we draw text ourselves
    opt.features = opt.features ^ QStyleOptionViewItem::HasCheckIndicator;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // draw background (selection / hover handled by the style)
    if (opt.widget)
        opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
    else
        QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);

    // ── fonts ─────────────────────────────────────────────────────────────────
    QFont boldFont = opt.font;
    boldFont.setBold(true);
    QFontMetrics fmBold(boldFont);
    QFontMetrics fmNormal(opt.font);

    // ── drawing area ──────────────────────────────────────────────────────────
    const QRect r = opt.rect.adjusted(kPadH, kPadV, -kPadH, -kPadV);

    // choose pen colour: selected items use highlight text, others use normal text
    const bool selected = opt.state & QStyle::State_Selected;
    const QColor primaryColor   = selected
        ? opt.palette.highlightedText().color()
        : opt.palette.text().color();
    const QColor secondaryColor = selected
        ? opt.palette.highlightedText().color().lighter(140)
        : opt.palette.placeholderText().color();

    // ── LINE 1: name (left) + subtotal (right) ────────────────────────────────
    const QRect line1 = QRect(r.left(), r.top(), r.width(), fmBold.height());

    painter->setFont(boldFont);
    painter->setPen(primaryColor);
    // name – left aligned, elide if too long
    const QString elidedName = fmBold.elidedText(
        name, Qt::ElideRight, r.width() - fmBold.horizontalAdvance("Rp 000.000.000"));
    painter->drawText(line1, Qt::AlignLeft | Qt::AlignVCenter, elidedName);

    // subtotal – right aligned, bold, accent tint when not selected
    const QColor totalColor = selected
        ? primaryColor
        : opt.palette.link().color();
    painter->setPen(totalColor);
    painter->drawText(line1, Qt::AlignRight | Qt::AlignVCenter, formatRp(total));

    // ── LINE 2: "qty unit × harga" (left) ────────────────────────────────────
    const QRect line2 = QRect(
        r.left(),
        r.top() + fmBold.height() + kSpacing,
        r.width(),
        fmNormal.height());

    painter->setFont(opt.font);
    painter->setPen(secondaryColor);

    const QString detail = QString("%1 lbr  ×  %2")
                               .arg(qty)
                               .arg(formatRp(price));
    painter->drawText(line2, Qt::AlignLeft | Qt::AlignVCenter, detail);

    painter->restore();
}