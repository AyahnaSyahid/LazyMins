#ifndef PermissionDialog_H
#define PermissionDialog_H

#include <QDialog>
#include <QList>

namespace Ui {
  class PermissionDialog;
}

class QAbstractItemModel;
class QModelIndex;

class PermissionDialog : public QDialog
{
  Q_OBJECT

public:
  explicit PermissionDialog(int UserId, QWidget * =nullptr);
  ~PermissionDialog();

private slots:
  void on_comboBox_currentIndexChanged(int);
  void on_itemDataChanged(const QModelIndex& tl, const QModelIndex& bl, const QVector<int>& roles);
  void loadPermissionsForUser(int uid);

private:
  QAbstractItemModel *permissionModel;
  Ui::PermissionDialog *ui;
  int _user_id;
  QList<int> checkedIndex;
};

#endif