#pragma once

namespace Ui {
  class PriceLevelDialog;
}

#include <QDialog>

class PriceLevelDialog : public QDialog
{
  Q_OBJECT
  
  public:
    explicit PriceLevelDialog(QWidget * = nullptr);
    ~PriceLevelDialog();
    
    bool setPriceLevelId(int);
  
  public slots:
    void reject() override;
  
  private slots:
    void on_simpanButton_clicked();
  
  private:
    bool isDirty() const;
    bool commit();
    
    Ui::PriceLevelDialog *ui;
    int m_priceLevelId;
    QString m_name;
    QString m_description;
};