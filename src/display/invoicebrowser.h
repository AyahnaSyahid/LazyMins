#pragma once

#include "src/display/dataviewer.h"

class InvoiceBrowser : public DataViewer
{
    Q_OBJECT
public:
    InvoiceBrowser(QWidget *parent = nullptr);

private slots:
    void on_dataView_customContextMenuRequested(const QPoint& pt);

signals:
    void serialPrintRequested(int id);
};