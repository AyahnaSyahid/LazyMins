#include "orderitemdelegate.h"
#include "src/models/ordermodel.h"

#include <QPainter>

// 1. Calculate the dynamic height based on 3 lines + padding
QSize OrderItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    
    QFontMetrics fm(option.font);
    int lineHeight = fm.height();
    int padding = 10;
    
    // Total height = (3 lines * height) + top/bottom padding + spacing between lines
    int totalHeight = (lineHeight * 3) + (padding * 2);
    
    size.setHeight(totalHeight);
    return size;
}

// 2. Custom rendering logic
void OrderItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    auto model = qobject_cast<const OrderModel*>(index.model());
    if (!model) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }
    auto order_item = model->itemAt(index.row());
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    
    opt.text = "";
    painter->save();
    
    // Draw the background (handles selection and hover colors automatically)
    painter->setRenderHint(QPainter::Antialiasing);
    opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

    // Setup the drawing area (apply some horizontal padding)
    QRect rect = opt.rect.adjusted(10, 5, -10, -5); 
    QFontMetrics fm(opt.font);
    int lineHeight = fm.height();

    // --- LINE 1: Nama Jual ---
    painter->setPen(opt.palette.text().color());
    QFont boldFont = opt.font;
    boldFont.setBold(true);
    painter->setFont(boldFont);
    
    QRect line1Rect = rect.adjusted(0, 0, 0, -(lineHeight * 2));
    painter->drawText(line1Rect, Qt::AlignLeft | Qt::AlignVCenter, index.data(Qt::UserRole + 3).toString());

    // --- LINE 2: Ukuran & finishing ---
    painter->setFont(opt.font); // Reset to normal font
    QRect line2Rect = rect.adjusted(0, lineHeight, 0, -lineHeight);
    painter->drawText(line2Rect, Qt::AlignLeft | Qt::AlignVCenter, order_item.descriptionText());

    // --- LINE 3: Price summary (subtotal) ---
    painter->setPen(opt.palette.placeholderText().color()); // Muted color
    QRect line3Rect = rect.adjusted(0, lineHeight * 2, 0, 0);
    const int itemTotal = order_item.total();
    const QString priceText = QString("Rp %L1").arg(itemTotal);
    painter->drawText(line3Rect, Qt::AlignLeft | Qt::AlignVCenter, priceText);

    painter->restore();
}