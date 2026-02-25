#pragma once
#include <QComboBox>

class QSqlQueryModel;
class RolesComboBox : public QComboBox
{
  Q_OBJECT
  public:
    RolesComboBox(QWidget *p=nullptr);
    int currentRoleId() const;
  
  public slots:
    void refetchData();

  private:
    QSqlQueryModel *qmodel;
};