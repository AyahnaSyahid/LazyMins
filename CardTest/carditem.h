#pragma once

#include <QString>
#include <QColor>
#include <QVariant>

// ─────────────────────────────────────────────
//  ItemType  —  menentukan layout & ukuran card
// ─────────────────────────────────────────────
enum class ItemType {
    Hero,       // Card besar, value sangat besar — highlight utama
    Stat,       // Card medium — statistik umum
    Compact,    // Card kecil — banyak info ringkas
    Alert,      // Card dengan left-border accent — peringatan
    Divider     // Separator dengan label section
};

// ─────────────────────────────────────────────
//  Custom Roles untuk QAbstractListModel
// ─────────────────────────────────────────────
namespace CardRole {
    enum Roles {
        ItemTypeRole = Qt::UserRole + 1,
        TitleRole,
        ValueRole,
        SubtextRole,
        AccentColorRole,
        IconRole        // opsional: unicode emoji / icon char
    };
}

// ─────────────────────────────────────────────
//  CardItem  —  struktur data satu item
// ─────────────────────────────────────────────
struct CardItem {
    ItemType    type        = ItemType::Stat;
    QString     title;
    QString     value;
    QString     subtext;
    QColor      accentColor = QColor("#3b82f6");
    QString     icon;           // misal: "📦" "💰" "⚠️"

    // ── Builder-style setters ──────────────────
    CardItem& setTitle(const QString &t)       { title = t;        return *this; }
    CardItem& setValue(const QString &v)       { value = v;        return *this; }
    CardItem& setSubtext(const QString &s)     { subtext = s;      return *this; }
    // CardItem& setColor(const QString &c)       { accentColor = QColor(c); return *this; }
    CardItem& setColor(const QColor &c)        { accentColor = c;  return *this; }
    CardItem& setIcon(const QString &i)        { icon = i;         return *this; }

    // ── Factory ───────────────────────────────
    static CardItem make(ItemType t) {
        CardItem item;
        item.type = t;
        return item;
    }
};
