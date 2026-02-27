#include "querycombobox.h"

class PriceLevelComboBox : public QueryComboBox
{
  public:
    PriceLevelComboBox(QWidget *p=nullptr) : QueryComboBox(p) {
      setQuery("SELECT id, level_name, description FROM price_levels");
      boxView->hideColumn(0);
      setModelColumn(1);
      boxViewAutoResize();
      qDebug() << qmodel->rowCount();
    };
};