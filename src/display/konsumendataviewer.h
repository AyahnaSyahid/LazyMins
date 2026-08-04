#pragma once

#include "dataviewer.h"

class QAction;
class KonsumenDataViewer : public DataViewer
{
    Q_OBJECT

public:
    explicit KonsumenDataViewer(QWidget *parent = nullptr);
    ~KonsumenDataViewer();

    QAction *addKonsumenAction();

public slots:
    void openCreateKonsumenDialog();

private slots:
    void on_dataView_customContextMenuRequested(const QPoint &pt);
    void onAddKonsumenActionTriggered();
    void openEditKonsumenDialog(int konsumenId);
    void openOrderHistory(int konsumenId);
    void openKonsumenRankDialog();

private:
    Ui::DataViewer *ui;
    QAction *m_addKonsumenAction = nullptr;
};
