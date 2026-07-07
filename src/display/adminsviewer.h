#pragma once

#include "dataviewer.h"

namespace Ui {
    class DataViewer;
}

class AdminsViewer : public DataViewer
{
    Q_OBJECT
public:
    explicit AdminsViewer(QWidget *parent = nullptr);
    ~AdminsViewer();

private slots:
    void on_dataView_customContextMenuRequested(const QPoint &pt);
    void editUser(int userId);
    void changeUserPassword(int userId);
protected:
    Ui::DataViewer *ui;
};