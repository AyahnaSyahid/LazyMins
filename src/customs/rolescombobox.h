#pragma once
#include <QComboBox>

#include "querycombobox.h"

class RolesComboBox : public QueryComboBox  
{
  Q_OBJECT
  public:
    RolesComboBox(QWidget *p=nullptr);
    int currentRoleId() const;
};