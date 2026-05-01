#include "buttonguard.h"

ButtonGuard::ButtonGuard(QAbstractButton* b) : m_button(b) { m_button->setEnabled(false); }
ButtonGuard::~ButtonGuard() { m_button->setEnabled(true); }