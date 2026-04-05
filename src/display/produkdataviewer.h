#pragma once

#include "dataviewer.h"

class QAction;
class ProdukDataViewer : public DataViewer
{
    Q_OBJECT

public:
    ProdukDataViewer(QWidget *parent=nullptr);
    ~ProdukDataViewer();
    void prependContextAction(QAction* act);
    QAction* addProductAction();
    QAction* addCategoryProductAction();


public slots:
    // void openStockOpname(int product_id);

private slots:
    void on_dataView_customContextMenuRequested(const QPoint& pt);
    void onContextMenu(const QPoint& pt);
    void onAddProductActionTriggered();
    void onAddCategoryProductActionTriggered();
    void openStockOpname(int product_id);
    void openRefillDialog(int produkId);
    void openPriceEditorDialog(int productId);
    void openProductEditor(int productId);

private:
    Ui::DataViewer *ui;
    QList<QAction*> m_prependedActions;
    QAction* m_addProductAction = nullptr;
    QAction* m_addCategoryProductAction = nullptr;
};
