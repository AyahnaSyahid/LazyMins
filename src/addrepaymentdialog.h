#ifndef AddRepaymentDialogH
#define AddRepaymentDialogH

namespace Ui {
    class AddRepaymentDialog;
}

#include <QDialog>

class AddRepaymentDialog : public QDialog
{
    Q_OBJECT
  
  public:
    AddRepaymentDialog(QWidget* parent);
    ~AddRepaymentDialog();
    int value() const;
    bool isTransfer() const;
  
  public slots:
    void setValue(int);
    void accept() override;
 
  private slots:
    void on_actionFillToMax_triggered();
    
  protected:
    Ui::AddRepaymentDialog* ui;
};

class CashbackDialog : public AddRepaymentDialog
{
  public:
    CashbackDialog(QWidget* =nullptr);
    void setValue(int);
};

#endif // AddRepaymentDialogH