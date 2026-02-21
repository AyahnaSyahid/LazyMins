#include "carddelegate.h"
#include "cardmodel.h"

#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QPainterPath>
#include <QLinearGradient>
#include <QFont>
#include <QFontMetrics>

// ════════════════════════════════════════════════════════════════════════════
//  Palette warna internal (dark dashboard style)
// ════════════════════════════════════════════════════════════════════════════
namespace Palette {
    // backgrounds
    static const QColor CardBg        { "#1e2433" };
    static const QColor CardBgHover   { "#252d40" };
    static const QColor CardBgHero    { "#1a2035" };
    static const QColor DividerBg     { "#131926" };

    // teks
    static const QColor TextPrimary   { "#e8eaf0" };
    static const QColor TextSecondary { "#8892a4" };
    static const QColor TextMuted     { "#555f73" };

    // selected overlay
    static const QColor SelectOverlay { 255, 255, 255, 15 };

    // shadow
    static const QColor Shadow        { 0, 0, 0, 80 };
}

// ════════════════════════════════════════════════════════════════════════════
CardDelegate::CardDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

// ────────────────────────────────────────────────────────────────────────────
//  Helpers
// ────────────────────────────────────────────────────────────────────────────
CardItem CardDelegate::itemFromIndex(const QModelIndex &index) const
{
    CardItem item;
    item.type        = index.data(CardRole::ItemTypeRole).value<ItemType>();
    item.title       = index.data(CardRole::TitleRole).toString();
    item.value       = index.data(CardRole::ValueRole).toString();
    item.subtext     = index.data(CardRole::SubtextRole).toString();
    item.accentColor = index.data(CardRole::AccentColorRole).value<QColor>();
    item.icon        = index.data(CardRole::IconRole).toString();
    return item;
}

void CardDelegate::drawShadow(QPainter *p, const QRect &cardRect) const
{
    // Simulasi drop shadow dengan beberapa layer transparan
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    for (int i = 4; i >= 1; --i) {
        QRect shadowRect = cardRect.adjusted(-i, -i + 3, i, i + 3);
        QColor sc = Palette::Shadow;
        sc.setAlpha(18 * i);
        p->setPen(Qt::NoPen);
        p->setBrush(sc);
        QPainterPath path;
        path.addRoundedRect(shadowRect, kRadius + i, kRadius + i);
        p->drawPath(path);
    }
    p->restore();
}

void CardDelegate::drawCardBackground(QPainter *p, const QRect &r,
                                      const QColor &bg, qreal radius,
                                      bool selected) const
{
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(r, radius, radius);

    p->setPen(Qt::NoPen);
    p->setBrush(bg);
    p->drawPath(path);

    // Subtle top-edge highlight (glass effect)
    QLinearGradient highlight(r.topLeft(), QPoint(r.left(), r.top() + 40));
    highlight.setColorAt(0, QColor(255, 255, 255, 18));
    highlight.setColorAt(1, QColor(255, 255, 255, 0));
    p->setBrush(highlight);
    p->drawPath(path);

    // Selected overlay
    if (selected) {
        p->setBrush(Palette::SelectOverlay);
        p->drawPath(path);
    }

    // Border tipis
    p->setPen(QPen(QColor(255, 255, 255, 20), 1));
    p->setBrush(Qt::NoBrush);
    p->drawPath(path);

    p->restore();
}

// ════════════════════════════════════════════════════════════════════════════
//  sizeHint — ukuran berbeda tiap type
// ════════════════════════════════════════════════════════════════════════════
QSize CardDelegate::sizeHint(const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    Q_UNUSED(option)
    auto type = index.data(CardRole::ItemTypeRole).value<ItemType>();
    int w = option.rect.width() > 0 ? option.rect.width() : 400;

    switch (type) {
    case ItemType::Hero:     return { w, 130 };
    case ItemType::Stat:     return { w, 100 };
    case ItemType::Compact:  return { w,  76 };
    case ItemType::Alert:    return { w,  90 };
    case ItemType::Divider:  return { w,  40 };
    }
    return { w, 100 };
}

// ════════════════════════════════════════════════════════════════════════════
//  paint — dispatcher
// ════════════════════════════════════════════════════════════════════════════
void CardDelegate::paint(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);

    CardItem item = itemFromIndex(index);
    bool selected = option.state & QStyle::State_Selected;

    switch (item.type) {
    case ItemType::Hero:     paintHero   (painter, option.rect, item, selected); break;
    case ItemType::Stat:     paintStat   (painter, option.rect, item, selected); break;
    case ItemType::Compact:  paintCompact(painter, option.rect, item, selected); break;
    case ItemType::Alert:    paintAlert  (painter, option.rect, item, selected); break;
    case ItemType::Divider:  paintDivider(painter, option.rect, item);           break;
    }

    painter->restore();
}

// ════════════════════════════════════════════════════════════════════════════
//  HERO  — Card besar, prominent, dengan gradient accent di kiri
// ════════════════════════════════════════════════════════════════════════════
void CardDelegate::paintHero(QPainter *p, const QRect &r,
                              const CardItem &item, bool selected) const
{
    QRect card = r.adjusted(kMargin, kMargin / 2, -kMargin, -kMargin / 2);

    drawShadow(p, card);
    drawCardBackground(p, card, Palette::CardBgHero, kRadius, selected);

    // Left accent bar dengan gradient vertikal
    QRect accentBar(card.left(), card.top() + 16, 4, card.height() - 32);
    QPainterPath accentPath;
    accentPath.addRoundedRect(accentBar, 2, 2);
    QLinearGradient accentGrad(accentBar.topLeft(), accentBar.bottomLeft());
    accentGrad.setColorAt(0, item.accentColor.lighter(120));
    accentGrad.setColorAt(1, item.accentColor.darker(130));
    p->setPen(Qt::NoPen);
    p->setBrush(accentGrad);
    p->drawPath(accentPath);

    int x = card.left() + kPad + 10;
    int y = card.top() + kPad;

    // Icon (jika ada)
    if (!item.icon.isEmpty()) {
        QFont iconFont = p->font();
        iconFont.setPixelSize(28);
        p->setFont(iconFont);
        p->drawText(QRect(card.right() - 56, card.top() + 16, 40, 40),
                    Qt::AlignCenter, item.icon);
    }

    // Title
    QFont titleFont;
    titleFont.setPixelSize(11);
    titleFont.setWeight(QFont::DemiBold);
    titleFont.setLetterSpacing(QFont::AbsoluteSpacing, 1.2);
    p->setFont(titleFont);
    p->setPen(Palette::TextSecondary);
    p->drawText(QRect(x, y, card.width() - kPad * 2, 18),
                Qt::AlignLeft | Qt::AlignVCenter,
                item.title.toUpper());

    // Value
    QFont valueFont;
    valueFont.setPixelSize(34);
    valueFont.setWeight(QFont::Bold);
    p->setFont(valueFont);
    p->setPen(item.accentColor.lighter(115));
    p->drawText(QRect(x, y + 18, card.width() - kPad * 2 - 60, 46),
                Qt::AlignLeft | Qt::AlignVCenter, item.value);

    // Subtext
    if (!item.subtext.isEmpty()) {
        QFont subFont;
        subFont.setPixelSize(11);
        subFont.setItalic(true);
        p->setFont(subFont);
        p->setPen(Palette::TextMuted);
        p->drawText(QRect(x, y + 66, card.width() - kPad * 2, 18),
                    Qt::AlignLeft | Qt::AlignVCenter, item.subtext);
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  STAT  — Card medium standar
// ════════════════════════════════════════════════════════════════════════════
void CardDelegate::paintStat(QPainter *p, const QRect &r,
                              const CardItem &item, bool selected) const
{
    QRect card = r.adjusted(kMargin, kMargin / 2, -kMargin, -kMargin / 2);

    drawShadow(p, card);
    drawCardBackground(p, card, Palette::CardBg, kRadius, selected);

    // Accent dot di pojok kanan atas
    QRadialGradient dotGrad(card.right() - 20, card.top() + 20, 18);
    dotGrad.setColorAt(0, QColor(item.accentColor.red(),
                                  item.accentColor.green(),
                                  item.accentColor.blue(), 60));
    dotGrad.setColorAt(1, Qt::transparent);
    p->setPen(Qt::NoPen);
    p->setBrush(dotGrad);
    p->drawEllipse(card.right() - 38, card.top() + 2, 36, 36);

    // Icon
    if (!item.icon.isEmpty()) {
        QFont iconFont;
        iconFont.setPixelSize(20);
        p->setFont(iconFont);
        p->drawText(QRect(card.right() - 36, card.top() + 6, 28, 28),
                    Qt::AlignCenter, item.icon);
    }

    int x = card.left() + kPad;
    int y = card.top() + kPad;

    // Title
    QFont titleFont;
    titleFont.setPixelSize(10);
    titleFont.setWeight(QFont::DemiBold);
    titleFont.setLetterSpacing(QFont::AbsoluteSpacing, 1.0);
    p->setFont(titleFont);
    p->setPen(Palette::TextSecondary);
    p->drawText(QRect(x, y, card.width() - kPad * 2, 16),
                Qt::AlignLeft | Qt::AlignVCenter, item.title.toUpper());

    // Value
    QFont valueFont;
    valueFont.setPixelSize(24);
    valueFont.setWeight(QFont::Bold);
    p->setFont(valueFont);
    p->setPen(item.accentColor.lighter(110));
    p->drawText(QRect(x, y + 16, card.width() - kPad * 2 - 40, 36),
                Qt::AlignLeft | Qt::AlignVCenter, item.value);

    // Subtext
    if (!item.subtext.isEmpty()) {
        QFont subFont;
        subFont.setPixelSize(10);
        p->setFont(subFont);
        p->setPen(Palette::TextMuted);
        p->drawText(QRect(x, y + 52, card.width() - kPad * 2, 16),
                    Qt::AlignLeft | Qt::AlignVCenter, item.subtext);
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  COMPACT  — Card kecil ringkas (title kiri, value kanan)
// ════════════════════════════════════════════════════════════════════════════
void CardDelegate::paintCompact(QPainter *p, const QRect &r,
                                 const CardItem &item, bool selected) const
{
    QRect card = r.adjusted(kMargin, 4, -kMargin, -4);

    drawShadow(p, card);
    drawCardBackground(p, card, Palette::CardBg, 8, selected);

    // Bottom accent line
    QRect bottomLine(card.left() + kRadius, card.bottom() - 2,
                     card.width() - kRadius * 2, 2);
    QPainterPath linePath;
    linePath.addRoundedRect(bottomLine, 1, 1);
    p->setPen(Qt::NoPen);
    p->setBrush(QColor(item.accentColor.red(),
                       item.accentColor.green(),
                       item.accentColor.blue(), 120));
    p->drawPath(linePath);

    int mid = card.top() + card.height() / 2;

    // Icon + Title (kiri)
    int x = card.left() + kPad;
    if (!item.icon.isEmpty()) {
        QFont iconFont;
        iconFont.setPixelSize(16);
        p->setFont(iconFont);
        p->drawText(QRect(x, mid - 12, 22, 24), Qt::AlignCenter, item.icon);
        x += 26;
    }

    QFont titleFont;
    titleFont.setPixelSize(12);
    titleFont.setWeight(QFont::Medium);
    p->setFont(titleFont);
    p->setPen(Palette::TextPrimary);
    p->drawText(QRect(x, mid - 10, card.width() / 2, 20),
                Qt::AlignLeft | Qt::AlignVCenter, item.title);

    // Subtext di bawah title
    if (!item.subtext.isEmpty()) {
        QFont subFont;
        subFont.setPixelSize(9);
        p->setFont(subFont);
        p->setPen(Palette::TextMuted);
        p->drawText(QRect(x, mid + 8, card.width() / 2, 14),
                    Qt::AlignLeft | Qt::AlignVCenter, item.subtext);
    }

    // Value (kanan)
    QFont valueFont;
    valueFont.setPixelSize(16);
    valueFont.setWeight(QFont::Bold);
    p->setFont(valueFont);
    p->setPen(item.accentColor.lighter(110));
    p->drawText(QRect(card.right() - 120, mid - 12, 110, 24),
                Qt::AlignRight | Qt::AlignVCenter, item.value);
}

// ════════════════════════════════════════════════════════════════════════════
//  ALERT  — Card dengan thick left border berwarna
// ════════════════════════════════════════════════════════════════════════════
void CardDelegate::paintAlert(QPainter *p, const QRect &r,
                               const CardItem &item, bool selected) const
{
    QRect card = r.adjusted(kMargin, kMargin / 2, -kMargin, -kMargin / 2);

    drawShadow(p, card);

    // Background dengan subtle tint dari accent color
    QColor tintedBg = Palette::CardBg;
    tintedBg.setRed  (qMin(255, tintedBg.red()   + item.accentColor.red()   / 12));
    tintedBg.setGreen(qMin(255, tintedBg.green() + item.accentColor.green() / 12));
    tintedBg.setBlue (qMin(255, tintedBg.blue()  + item.accentColor.blue()  / 12));
    drawCardBackground(p, card, tintedBg, kRadius, selected);

    // Left accent border tebal
    QRect leftBorder(card.left(), card.top() + kRadius,
                     6, card.height() - kRadius * 2);
    QPainterPath lbPath;
    lbPath.addRoundedRect(leftBorder, 3, 3);
    p->setPen(Qt::NoPen);
    p->setBrush(item.accentColor);
    p->drawPath(lbPath);

    int x = card.left() + 18;
    int y = card.top() + kPad - 2;

    // Icon
    if (!item.icon.isEmpty()) {
        QFont iconFont;
        iconFont.setPixelSize(18);
        p->setFont(iconFont);
        p->drawText(QRect(x, y, 24, 24), Qt::AlignCenter, item.icon);
        x += 28;
    }

    // Title
    QFont titleFont;
    titleFont.setPixelSize(10);
    titleFont.setWeight(QFont::DemiBold);
    titleFont.setLetterSpacing(QFont::AbsoluteSpacing, 1.0);
    p->setFont(titleFont);
    p->setPen(item.accentColor.lighter(130));
    p->drawText(QRect(x, y, card.width() - x - kPad, 16),
                Qt::AlignLeft | Qt::AlignVCenter, item.title.toUpper());

    // Value
    QFont valueFont;
    valueFont.setPixelSize(20);
    valueFont.setWeight(QFont::Bold);
    p->setFont(valueFont);
    p->setPen(Palette::TextPrimary);
    p->drawText(QRect(x, y + 16, card.width() - x - kPad, 30),
                Qt::AlignLeft | Qt::AlignVCenter, item.value);

    // Subtext
    if (!item.subtext.isEmpty()) {
        QFont subFont;
        subFont.setPixelSize(10);
        p->setFont(subFont);
        p->setPen(Palette::TextSecondary);
        p->drawText(QRect(x, y + 46, card.width() - x - kPad, 16),
                    Qt::AlignLeft | Qt::AlignVCenter, item.subtext);
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  DIVIDER  — Section separator
// ════════════════════════════════════════════════════════════════════════════
void CardDelegate::paintDivider(QPainter *p, const QRect &r,
                                 const CardItem &item) const
{
    int mid = r.top() + r.height() / 2;
    int x   = r.left() + kMargin;
    int w   = r.width() - kMargin * 2;

    // Garis horizontal kiri dan kanan teks
    QFont labelFont;
    labelFont.setPixelSize(10);
    labelFont.setWeight(QFont::DemiBold);
    labelFont.setLetterSpacing(QFont::AbsoluteSpacing, 2.0);
    p->setFont(labelFont);

    QFontMetrics fm(labelFont);
    int textW = fm.horizontalAdvance(item.title.toUpper()) + 20;
    int textX = x + (w - textW) / 2;

    // Kiri
    p->setPen(QPen(Palette::TextMuted, 1));
    p->drawLine(x, mid, textX - 8, mid);

    // Label
    p->setPen(Palette::TextMuted);
    p->drawText(QRect(textX, mid - 10, textW, 20),
                Qt::AlignCenter, item.title.toUpper());

    // Kanan
    p->drawLine(textX + textW + 8, mid, x + w, mid);
}
