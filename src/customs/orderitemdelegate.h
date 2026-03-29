#pragma once

#include <QStyledItemDelegate>
class OrderItemDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    // 1. Calculate the dynamic height based on 3 lines + padding
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    // 2. Custom rendering logic
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};