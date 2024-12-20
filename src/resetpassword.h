#ifndef ResetPasswordH
#define ResetPasswordH

#include <QDialog>
#include "useritem.h"

namespace Ui {
    class ResetPassword;
}

class ResetPassword : public QDialog
{
    Q_OBJECT
  public:
    ResetPassword(QWidget* = nullptr);
    ~ResetPassword();
    void setUser(const UserItem& u);

  private slots:
    virtual void accept() override;
    virtual void reject() override;

  signals:
    void passwordChanged(qint64 uid);

  protected:
    Ui::ResetPassword* ui;
    UserItem user;
};

#endif