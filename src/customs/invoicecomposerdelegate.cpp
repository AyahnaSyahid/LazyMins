#include "invoicecomposerdelegate.h"
#include "src/models/invoicecomposermodel.h" // Untuk mengakses enum Role

InvoiceComposerDelegate::InvoiceComposerDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

QSize InvoiceComposerDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // Memberikan tinggi baris yang cukup untuk dua baris teks (70 pixel)
    return QSize(option.rect.width(), 70);
}

void InvoiceComposerDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // Menggambar background selection/hover
    // if (option.state & QStyle::State_Selected) {
        // painter->fillRect(option.rect, option.palette.highlight());
    // } else if (option.state & QStyle::State_MouseOver) {
        // painter->fillRect(option.rect, option.palette.alternateBase());
    // }
    
    QStyledItemDelegate::paint(painter, option, index);
    
    // Area gambar dengan margin dalam (padding)
    QRect rect = option.rect.adjusted(10, 10, -10, -10);

    // --- 1. Menggambar Nomor Order (Bold) ---
    QFont fontOrder = painter->font();
    fontOrder.setBold(true);
    fontOrder.setPointSize(11);
    painter->setFont(fontOrder);
    
    // Mengambil data dari NumberRole
    QString orderNum = index.data(InvoiceComposerModel::NumberRole).toString();
    painter->drawText(rect.left(), rect.top() + 15, orderNum);

    // --- 2. Menggambar Subtotal & Discount (Kiri Bawah) ---
    QFont fontDetail = painter->font();
    fontDetail.setBold(false);
    fontDetail.setPointSize(9);
    painter->setFont(fontDetail);
    painter->setPen(Qt::gray);

    int subtotal = index.data(InvoiceComposerModel::SubtotalRole).toInt();
    int discount = index.data(InvoiceComposerModel::DiscountRole).toInt();
    QString details = QString("Subtotal: %L1 | Disc: %L2").arg(subtotal).arg(discount);
    painter->drawText(rect.left(), rect.bottom() - 5, details);

    // --- 3. Menggambar Total (Kanan Bawah) ---
    QFont fontTotal = painter->font();
    fontTotal.setBold(true);
    fontTotal.setPointSize(13);
    painter->setFont(fontTotal);
    
    // Warna hijau jika tidak sedang dipilih
    if (!(option.state & QStyle::State_Selected)) {
        painter->setPen(QColor("#27ae60"));
    } else {
        painter->setPen(option.palette.highlightedText().color());
    }

    QString totalStr = QString("Total: %L1").arg(index.data(InvoiceComposerModel::TotalRole).toInt());
    painter->drawText(rect, Qt::AlignRight | Qt::AlignBottom, totalStr);

    painter->restore();
}