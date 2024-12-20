#ifndef UserFormH
#define UserFormH

namespace Ui {
    class UserForm;
}

#include <QWidget>

class UserForm : public QWidget
{
  public:
    UserForm(QWidget* =nullptr);
    ~UserForm();
  private:
    Ui::UserForm* ui;
};

#endif // UserFormH