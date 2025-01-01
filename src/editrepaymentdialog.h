#ifndef EditRepaymentDialogH
#define EditRepaymentDialogH

#include <QDialog>
#include <QSqlRecord>

namespace Ui {
    class EditRepaymentDialog;
}

class EditRepaymentDialog : public QDialog
{
    Q_OBJECT    
  public:
    EditRepaymentDialog(qint64, QWidget* =nullptr);
    ~EditRepaymentDialog();

  public slots:
    void on_buttonBox_accepted();

  private:
    Ui::EditRepaymentDialog* ui;
};

#endif //EditRepaymentDialogH