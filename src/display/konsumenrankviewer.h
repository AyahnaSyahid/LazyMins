#pragma once

#include <QWidget>

class QSqlQueryModel;
class KonsumenRankViewer: public QWidget
{
  public:
    KonsumenRankViewer(QWidget *parent=nullptr);
    ~KonsumenRankViewer();
  
  public slots:
    void fetchData();

  private slots:
    void onDataReady();

  private:
    QSqlQueryModel *_model;  
};
