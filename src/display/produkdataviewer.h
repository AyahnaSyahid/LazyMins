#pragma once

#include "dataviewer.h"

class ProdukDataViewer : public DataViewer
{
    Q_OBJECT

public:
    ProdukDataViewer(QWidget *parent=nullptr);
    ~ProdukDataViewer();

public slots:
    // void openStockOpname(int product_id);

private slots:
    void on_dataView_customContextMenuRequested(const QPoint& pt);
    void openStockOpname(int product_id);

private:
    Ui::DataViewer *ui;
};
