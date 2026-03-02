#include "src/customs/productcategoriescombobox.h"

ProductCategoriesComboBox::ProductCategoriesComboBox(QWidget* parent) : QueryComboBox(parent) {
    setQuery("SELECT id, category_name, description FROM product_categories");
    setModelColumn(1);
    boxView->hideColumn(0);
    boxViewAutoResize();
}

void ProductCategoriesComboBox::setCurrentId(int id) {
  QModelIndexList ml = qmodel->match(qmodel->index(0,0), Qt::EditRole, id, 1);
  if(ml.isEmpty()) {
    setCurrentIndex(-1);
    return;
  }
  setCurrentIndex(ml.first().row());
}

int ProductCategoriesComboBox::currentId() const {
  if(!qmodel->index(currentIndex(), 0).isValid()) return 0;
  return qmodel->index(currentIndex(), 0).data(Qt::EditRole).toInt();
}