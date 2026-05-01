#pragma once

#include <QAbstractButton>

class ButtonGuard {
 private:
  QAbstractButton* m_button;

 public:
  ButtonGuard(QAbstractButton* b);
  ~ButtonGuard();
};