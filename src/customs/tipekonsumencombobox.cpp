#include "tipekonsumencombobox.h"
#include <QLineEdit>
#include <QHeaderView>

TipeKonsumenComboBox::TipeKonsumenComboBox(QWidget *p) : QueryComboBox(p)
{
  setQuery("SELECT DISTINCT customer_type FROM konsumen");
  setModel(qmodel);
  setModelColumn(0);
  boxView->horizontalHeader()->setStretchLastSection(true);
  setEditable(true);
  qDebug() << "is Column 0 hidden" << boxView->isColumnHidden(0);
}