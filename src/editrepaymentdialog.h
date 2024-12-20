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
    EditRepaymentDialog(const QSqlRecord& rec, QWidget* =nullptr);
    ~EditRepaymentDialog();
    inline const QSqlRecord& record() const { return r; }
    
  public slots:
    void on_buttonBox_accepted();

  private:
    QSqlRecord r;
    Ui::EditRepaymentDialog* ui;
};

#endif //EditRepaymentDialogH