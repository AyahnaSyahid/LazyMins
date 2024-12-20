#ifndef LoginDialogH
#define LoginDialogH
#include "config.h"

namespace Ui {
    class LoginDialog;
}

#include <QDialog>

class QTimer;
class LoginDialog : public QDialog
{
    Q_OBJECT
  
  public:
    static qint64 blockingLoginDialog(int max_attemp = 3);
    
    LoginDialog(QWidget* =nullptr);
    ~LoginDialog();
    inline const qint64& getLoggedId() const { return GlobalNamespace::logged_user_id; }

  private slots:
    void accept() override;
  
  private:
    Ui::LoginDialog* ui;
};


#endif // LoginDialogH