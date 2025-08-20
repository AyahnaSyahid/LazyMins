#ifndef PermissionDialog_H
#define PermissionDialog_H

#include <QDialog>

namespace Ui {
  class PermissionDialog;
}

class QAbstractItemModel;
class QStandardItem;



class PermissionDialog : public QDialog
{
  Q_OBJECT

public:
  explicit PermissionDialog(int UserId, QWidget * =nullptr);
  ~PermissionDialog();

private slots:
  void on_comboBox_currentIndexChanged(int);
  void on_itemChanged(QStandardItem *item);

private:
  QAbstractItemModel *permissionModel;
  Ui::PermissionDialog *ui;
  int _user_id;
  bool _safe_to_open;
};

#endif