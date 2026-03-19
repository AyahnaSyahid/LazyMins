#pragma once

#include <QStyledItemDelegate>

/**
 * FinishingItemDelegate
 *
 * Renders each finishing row as a two-line card:
 *
 *   ┌─────────────────────────────────────────────┐
 *   │  Laminasi Doff                    Rp 50.000 │
 *   │  2 lembar  ×  Rp 25.000                     │
 *   └─────────────────────────────────────────────┘
 *
 * Roles read from FinishingListModel:
 *   Qt::UserRole + 4  – finishing_name  (QString)
 *   Qt::UserRole + 5  – quantity        (int)
 *   Qt::UserRole + 6  – finishing_price (int)
 *   Qt::UserRole + 7  – subtotal        (int)
 */
class FinishingItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit FinishingItemDelegate(QObject *parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

private:
    static constexpr int kPadH    = 10;  // horizontal padding
    static constexpr int kPadV    = 7;   // vertical padding
    static constexpr int kSpacing = 3;   // gap between line 1 and line 2

    static QString formatRp(int value);
};