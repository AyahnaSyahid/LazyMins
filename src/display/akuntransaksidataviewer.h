#pragma once

#include "dataviewer.h"


class QAction;
class AdvancedQueryModel;
class AkunTransaksiDataViewer : public DataViewer
{
    Q_OBJECT

public:
    explicit AkunTransaksiDataViewer(QWidget *parent = nullptr);
    ~AkunTransaksiDataViewer();
    QAction *addAkunAction();

public slots:
    void openCreateAkunDialog();

private slots:
    void on_dataView_customContextMenuRequested(const QPoint &pt);
    void openBalanceAdjustment(int akunId);
    void openDepositDialog(int akunId);
    void openEditAkunDialog(int akunId);

private:
    Ui::DataViewer *ui;
    QAction *m_addAkunAction = nullptr;
    QAction *m_adjustBalanceAction = nullptr;
};
