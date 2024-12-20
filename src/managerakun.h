#ifndef ManagerAkunH
#define ManagerAkunH

namespace Ui {
    class ManagerAkun;
}

#include <QDialog>
class QModelIndex;
class ManagerAkun : public QDialog
{
    Q_OBJECT
  public:
    ManagerAkun(QWidget* = nullptr);
    ~ManagerAkun();

  private slots:
    void on_pushButton2_clicked();
    void on_pushButton_clicked();
    void on_tableView_customContextMenuRequested(const QPoint&);
    void on_tableView_doubleClicked(const QModelIndex&);
    void resetModel();
    void editUserBio(qint64 uid);
  
  signals:
    void userCreated(qint64 uid);
    
  private:
    Ui::ManagerAkun* ui;
};

#endif // ManagerAkunH