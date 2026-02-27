#include "querycombobox.h"

namespace {
  auto toTitleCase = [](const QString& s) -> QString {
    if (s.isEmpty()) return s;
    
    QString result = s.toLower();
    QRegularExpression re("\\b\\w");
    
    return result.replace(re, [](const QRegularExpressionMatch& m) {
        return m.captured().toUpper();
    });
}

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
    
    void setLevelID (int id) {
      for (int r=0; r<qmodel->rowCount(); ++r) {
        auto var = qmodel->index(r, 0).data(Qt::EditRole);
        if (var.isValid() && var.toInt() == id) {
          setCurrentIndex(r);
          return
        }
      }
    }
    setCurrentIndex(-1);
};