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
  
  private slots:
    void on_simpanButton_clicked();
    void on_tableView_customContextMenuRequested(const QPoint& p);
    void onCreateNewLevel();
  
  protected:
    void reject() override;
    bool saveAll();
    bool isAnythingDirty() const;

  signals:
    void dataCommited();
    
  private:
    Ui::PriceLevelEditorDialog *ui;
    PriceLevelEditorModel *m_model;
    int m_defaultCost;
    int m_productId;
};