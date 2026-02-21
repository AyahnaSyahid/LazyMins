#pragma once

#include <QStyledItemDelegate>
#include <QPainter>
#include "carditem.h"

class CardDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit CardDelegate(QObject *parent = nullptr);

    void    paint(QPainter *painter,
                  const QStyleOptionViewItem &option,
                  const QModelIndex &index) const override;

    QSize   sizeHint(const QStyleOptionViewItem &option,
                     const QModelIndex &index) const override;

private:
    // ── Per-type painters ─────────────────────
    void paintHero   (QPainter *p, const QRect &r, const CardItem &item, bool selected) const;
    void paintStat   (QPainter *p, const QRect &r, const CardItem &item, bool selected) const;
    void paintCompact(QPainter *p, const QRect &r, const CardItem &item, bool selected) const;
    void paintAlert  (QPainter *p, const QRect &r, const CardItem &item, bool selected) const;
    void paintDivider(QPainter *p, const QRect &r, const CardItem &item) const;

    // ── Helpers ───────────────────────────────
    void drawShadow(QPainter *p, const QRect &cardRect) const;
    void drawCardBackground(QPainter *p, const QRect &r, const QColor &bg,
                            qreal radius, bool selected) const;
    CardItem itemFromIndex(const QModelIndex &index) const;

    // ── Spacing constants ─────────────────────
    static constexpr int kMargin    = 12;   // margin luar card dari rect item
    static constexpr int kPad       = 16;   // padding dalam card
    static constexpr int kRadius    = 12;   // border radius card
};
