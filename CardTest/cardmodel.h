#pragma once

#include <QAbstractListModel>
#include <QList>
#include "carditem.h"

class CardModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit CardModel(QObject *parent = nullptr);

    // ── QAbstractListModel interface ──────────
    int         rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant    data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // ── Convenience API ───────────────────────
    void addItem(const CardItem &item);
    void setItems(const QList<CardItem> &items);
    void clear();

    // Akses langsung jika perlu update item tertentu
    void updateItem(int row, const CardItem &item);
    const CardItem& itemAt(int row) const;

private:
    QList<CardItem> m_items;
};
