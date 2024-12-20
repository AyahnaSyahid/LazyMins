#ifndef AddUserDialogH
#define AddUserDialogH

namespace Ui {
    class AddUserDialog;
}

#include <QDialog>

class QAbstractButton;
class UserItem;
class AddUserDialog : public QDialog {
    Q_OBJECT
    
  public:
    AddUserDialog(QWidget* =nullptr);
    ~AddUserDialog();

  public slots:
    void on_buttonBox_clicked(QAbstractButton* bt);
  
  signals:
    void userCreated(qint64 uid);
  
  private:
    UserItem* userItem;
    Ui::AddUserDialog* ui;
};

#endif // AddUserDialogH