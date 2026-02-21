#include "cardmodel.h"

CardModel::CardModel(QObject *parent)
    : QAbstractListModel(parent)
{}

int CardModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_items.count();
}

QVariant CardModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.count())
        return {};

    const CardItem &item = m_items.at(index.row());

    switch (role) {
    case CardRole::ItemTypeRole:    return QVariant::fromValue(item.type);
    case CardRole::TitleRole:       return item.title;
    case CardRole::ValueRole:       return item.value;
    case CardRole::SubtextRole:     return item.subtext;
    case CardRole::AccentColorRole: return item.accentColor;
    case CardRole::IconRole:        return item.icon;
    case Qt::DisplayRole:           return item.title; // fallback
    default:                        return {};
    }
}

QHash<int, QByteArray> CardModel::roleNames() const
{
    return {
        { CardRole::ItemTypeRole,    "itemType"    },
        { CardRole::TitleRole,       "title"       },
        { CardRole::ValueRole,       "value"       },
        { CardRole::SubtextRole,     "subtext"     },
        { CardRole::AccentColorRole, "accentColor" },
        { CardRole::IconRole,        "icon"        },
    };
}

void CardModel::addItem(const CardItem &item)
{
    beginInsertRows(QModelIndex(), m_items.count(), m_items.count());
    m_items.append(item);
    endInsertRows();
}

void CardModel::setItems(const QList<CardItem> &items)
{
    beginResetModel();
    m_items = items;
    endResetModel();
}

void CardModel::clear()
{
    beginResetModel();
    m_items.clear();
    endResetModel();
}

void CardModel::updateItem(int row, const CardItem &item)
{
    if (row < 0 || row >= m_items.count()) return;
    m_items[row] = item;
    emit dataChanged(index(row), index(row));
}

const CardItem& CardModel::itemAt(int row) const
{
    return m_items.at(row);
}
