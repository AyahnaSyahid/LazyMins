#ifndef REPAYMENTINPUTDIALOG_H
#define REPAYMENTINPUTDIALOG_H

namespace Ui {
  class RepaymentInputDialog;
}

#include <QDialog>

class RepaymentInputDialog : public QDialog {
  
  Q_OBJECT
  
  public:
    RepaymentInputDialog(int invoice_id, QWidget *p=nullptr);
    ~RepaymentInputDialog();
    
    
  private slots:
    void on_bayarBox_valueChanged(int);
    void on_dealButton_clicked();
  
  private:
    Ui::RepaymentInputDialog *ui;
    int m_invoice;
};

#endif


