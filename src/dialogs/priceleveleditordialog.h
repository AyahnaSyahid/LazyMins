#pragma once

namespace Ui {
  class PriceLevelEditorDialog;
}

#include <QDialog>

class PriceLevelEditorModel;
class PriceLevelEditorDialog : public QDialog
{
  Q_OBJECT
  
  public:
    explicit PriceLevelEditorDialog(QWidget * = nullptr);
    ~PriceLevelEditorDialog();
    bool setProductId(int);
  
  private:
    Ui::PriceLevelEditorDialog *ui;
    PriceLevelEditorModel *model;
};