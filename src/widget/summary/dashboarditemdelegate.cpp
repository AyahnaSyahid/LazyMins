#include "dashboarditemdelegate.h"
#include <QtGui>

QSize DashboardItemDelegate::sizeHint(
    const QStyleOptionViewItem &opt, 
    const QModelIndex& mi ) const
{
  auto type = mi.data(DashboardItem::Roles::TypeRole).value<DashboardItem::ItemTypes>();
  qDebug() << mi.model()->roleNames()[(int)type];
  int w = opt.rect.width() > 0 ? opt.rect.width() : 300;
  
  switch (type) {
    case DashboardItem::ItemTypes::Big    : return         {w, 200};
    case DashboardItem::ItemTypes::Medium : return         {w, 160};
    case DashboardItem::ItemTypes::Smal   : return         {w, 100};
    default :                               return         {w, 120};
  }
}

void DashboardItemDelegate::paint(QPainter *painter,
              const QStyleOptionViewItem &option,
              const QModelIndex &index) const
{
  painter->save();
  auto item = itemFromIndex(index);
  bool selected = option.state & QStyle::State_Selected;
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setBrush(Qt::red);
  painter->setPen(Qt::NoPen);
  
  QPainterPath path;
  path.addRoundedRect(option.rect.adjusted(5, 5, -5, -5), 5, 5);
  
  painter->drawPath(path);
  
  // switch (item.type) {
    // case DashboardItem::ItemTypes::Big :    paintBig(painter, option.rect, item, selected);            break;
    // case DashboardItem::ItemTypes::Smal :   paintSmall(painter, option.rect, item, selected);          break;
    // case DashboardItem::ItemTypes::Medium : paintMedium(painter, option.rect, item, selected);         break;
  // }
  painter->restore();
}

DashboardItem DashboardItemDelegate::itemFromIndex(const QModelIndex &i) const {
  DashboardItem item;
  item.type  = i.data(DashboardItem::Roles::TypeRole).value<DashboardItem::ItemTypes>();
  item.title = i.data(DashboardItem::Roles::TitleRole).toString();
  item.value = i.data(DashboardItem::Roles::ValueRole).toString();
  item.subText = i.data(DashboardItem::Roles::SubTextRole).toString();
  return item;
}

void DashboardItemDelegate::paintBig(QPainter *p, const QRect &r, 
                    const DashboardItem& item, bool selected) const
{
  QRect dr = r.adjusted(10, -10, -10, 10);
  // drawBackground
  p->save();
  p->setRenderHint(QPainter::Antialiasing);
  QPainterPath bgPath;
  bgPath.addRoundedRect(dr, 8, 8);
  auto b = QBrush(Qt::SolidPattern);
  b.setColor("#FF5555");
  p->setBrush(b);
  // p->setPen(Qt::SolidLine);
  p->drawPath(bgPath);
}

void DashboardItemDelegate::paintMedium(QPainter *p, const QRect &r, 
                const DashboardItem& item, bool selected) const
{  
  QRect dr = r.adjusted(10, -10, -10, 10);
  // drawBackground
  p->save();
  p->setRenderHint(QPainter::Antialiasing);
  QPainterPath bgPath;
  bgPath.addRoundedRect(r, 8, 8);
  p->setPen(Qt::SolidLine);
  p->drawPath(bgPath);
  // p->drawText(item.title, r);
}

void DashboardItemDelegate::paintSmall(QPainter *p, const QRect &r, 
                const DashboardItem& item, bool selected) const
{
  QRect dr = r.adjusted(10, -10, -10, 10);
  // drawBackground
  p->save();
  p->setRenderHint(QPainter::Antialiasing);
  QPainterPath bgPath;
  bgPath.addRoundedRect(r, 8, 8);
  p->setPen(Qt::SolidLine);
  p->drawPath(bgPath);
}