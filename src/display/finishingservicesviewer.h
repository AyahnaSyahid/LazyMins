#pragma once

#include "dataviewer.h"

class FinishingServicesViewer : public DataViewer
{
  Q_OBJECT

public:
  explicit FinishingServicesViewer(QWidget * = nullptr);
  ~FinishingServicesViewer();

public slots:
  void openCreateFinishingDialog();

private slots:
  void on_dataView_customContextMenuRequested(const QPoint& p);
};