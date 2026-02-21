#pragma once

#include <QColor>

struct DashboardItem {
  enum class ItemTypes {
    Big,
    Medium, 
    Smal
  };
  
  enum Roles {
    TypeRole = Qt::UserRole + 1,
    TitleRole,
    ValueRole,
    SubTextRole
  };
  
  
  static DashboardItem make(ItemTypes type) {
    DashboardItem item;
    item.type = type;
    return item;
  };
  
  // Member Variable
  DashboardItem& setType   (const ItemTypes &t) { type    = t;      return *this; }
  DashboardItem& setTitle  (const QString &t)  { title   = t;      return *this; }
  DashboardItem& setValue  (const QString &t)  { value   = t;      return *this; }
  DashboardItem& setSubText(const QString &t)  { subText = t;      return *this; }
  
  // Member Variable
  ItemTypes     type = ItemTypes::Smal;
  QString      title;
  QString      value;
  QString      subText;
  QColor       accentColor = QColor("#3b82f6");
};