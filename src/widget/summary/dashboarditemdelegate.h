#pragma once

#include <QStyledItemDelegate>
#include "dashboarditem.h"

class DashboardItemDelegate : public QStyledItemDelegate
{
  public:
    using QStyledItemDelegate::QStyledItemDelegate;
    
    void    paint(QPainter *painter,
                  const QStyleOptionViewItem &option,
                  const QModelIndex &index) const override;
    
    QSize   sizeHint(const QStyleOptionViewItem &option,
                     const QModelIndex &index) const override;
    
  private:
    DashboardItem itemFromIndex(const QModelIndex& mi) const;
    
    void paintBig(QPainter *p, const QRect &r, 
                    const DashboardItem& item, bool selected) const;
    void paintMedium(QPainter *p, const QRect &r, 
                    const DashboardItem& item, bool selected) const;
    void paintSmall(QPainter *p, const QRect &r, 
                    const DashboardItem& item, bool selected) const;
};