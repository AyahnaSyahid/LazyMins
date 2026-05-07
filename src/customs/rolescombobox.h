#pragma once
#include <QComboBox>

#include "querycombobox.h"

class RolesComboBox : public QueryComboBox  
{
  public:
    RolesComboBox(QWidget *p=nullptr);
    int currentRoleId() const;
    void setCurrentRoleId(int roleId);
};