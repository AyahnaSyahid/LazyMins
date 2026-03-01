#include "src/customs/productcategoriescombobox.h"


ProductCategoriesComboBox::ProductCategoriesComboBox(QWidget* parent) : QueryComboBox(parent) {
    setQuery("SELECT id, category_name, description FROM product_categories");
    setModelColumn(1);
    boxView->hideColumn(0);
    boxViewAutoResize();
}

