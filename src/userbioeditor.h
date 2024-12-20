#ifndef UserBioEditorH
#define UserBioEditorH

namespace Ui {
    class UserBioEditor;
}

#include "useritem.h"
#include <QDialog>

class UserBioEditor : public QDialog
{
  public:
    UserBioEditor(QWidget* =nullptr);
    ~UserBioEditor();
    void setUser(const UserItem&);
  
  private slots:
    void reject() override;
    void accept() override;
    
  private:
    Ui::UserBioEditor* ui;
    UserItem user;
};

#endif // UserFormH
