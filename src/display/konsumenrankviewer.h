#pragma once

#include <QWidget>

class QSqlQueryModel;
class QDateEdit;
class KonsumenRankViewer : public QWidget {
  Q_OBJECT
 public:
  KonsumenRankViewer(QWidget* parent = nullptr);
  ~KonsumenRankViewer();

 public slots:
  void fetchData();

 private slots:
  void onDataReady();
  void handleFilterEdit();

 private:
  void initDateFilter();
  QTimer* filterTimer;
  QDateEdit* startDateEdit;
  QDateEdit* endDateEdit;
  QSqlQueryModel* _model;
};
