#ifndef REALTIMEDATAWIDGET_H
#define REALTIMEDATAWIDGET_H

#include <QWidget>
class RealTimeDataWidget : public QWidget
{
  Q_OBJECT
  public:
    RealTimeDataWidget(QWidget *p=nullptr) : QWidget(p) {}
    ~RealTimeDataWidget() {}
  
  public slots:
    virtual void reloadModelData(const QList<QString> &tables) {}
};

#endif