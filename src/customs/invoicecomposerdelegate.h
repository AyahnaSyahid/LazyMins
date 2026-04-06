#pragma once

#include <QStyledItemDelegate>
#include <QPainter>

class InvoiceComposerDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit InvoiceComposerDelegate(QObject *parent = nullptr);

    // Menentukan ukuran dimensi setiap baris/item
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    // Menangani penggambaran visual item
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
