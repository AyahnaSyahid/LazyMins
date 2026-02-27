#include "tipekonsumencombobox.h"
#include <QLineEdit>

TipeKonsumenComboBox::TipeKonsumenComboBox(QWidget *p) : QueryComboBox(p)
{
  setQuery("SELECT DISTINCT customer_type FROM konsumen");
  boxView->setMinimumHeight(50);
  setEditable(true);
  auto le = lineEdit();
  if (le) {
    le->setAlignment(Qt::AlignCenter);
  }
}