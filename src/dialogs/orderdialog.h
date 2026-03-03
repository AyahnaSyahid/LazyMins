#pragma once

namespace Ui {
  class OrderDialog;
}

#include "formdialog.h"
#include "src/managers/managers.h"
#include "src/models/orderitemeditormodel.h"

class OrderDialog : public FormDialog
{
  Q_OBJECT
  
  public:
    explicit OrderDialog(QWidget * =nullptr);
    ~OrderDialog();
    void prepareModify(const QSqlRecord& r);
    
  protected:
    void setupFields() override;
    void setupBoundFields() override;
    bool onSave(const QVariantMap& ) override;
  
  private slots:
    void on_simpanButton_clicked();
    void on_tambahItem_triggered();

    private:
    Ui::OrderDialog *ui;
    OrderItemEditorModel *emodel;
    OrderManager oman;
};