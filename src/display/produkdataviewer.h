#pragma once

#include "dataviewer.h"

class ProdukDataViewer : public DataViewer
{
    Q_OBJECT
public:
    ProdukDataViewer(QWidget *parent=nullptr);
    ~ProdukDataViewer();

private:
    Ui::DataViewer *ui;
};
