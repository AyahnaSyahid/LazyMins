#include "src/customs/querycombobox.h"

class ProductCategoriesComboBox : public QueryComboBox
{
    Q_OBJECT
public:
    ProductCategoriesComboBox(QWidget *parent = nullptr);
    void setCurrentId(int id);
    int currentId() const;
};
