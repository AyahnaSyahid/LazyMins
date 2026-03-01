#include "querycombobox.h"

namespace {
  auto toTitleCase = [](const QString& s) -> QString {
    if (s.isEmpty()) return s;

    QString result = s.toLower();
    QRegularExpression re("\\b\\w");
    QRegularExpressionMatchIterator it = re.globalMatch(result);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        result[match.capturedStart()] = result[match.capturedStart()].toUpper();
    }

    return result;
  };
}

class PriceLevelComboBox : public QueryComboBox
{
  public:
    PriceLevelComboBox(QWidget *p=nullptr) : QueryComboBox(p) {
      setQuery("SELECT id, level_name, description FROM price_levels");
      boxView->hideColumn(0);
      setModelColumn(1);
      boxViewAutoResize();
    };
    
    void setLevelID (int id) {
      for (int r=0; r<qmodel->rowCount(); ++r) {
        auto var = qmodel->index(r, 0).data(Qt::EditRole);
        auto var2 = qmodel->index(r, 1).data(Qt::EditRole);
        if (var.isValid() && var.toInt() == id) {
          setCurrentIndex(r);
          return;
        }
      }
    setCurrentIndex(-1);
    }
    
    int currentId () const {
      auto cid = qmodel->index(currentIndex(), 0).data(Qt::EditRole).toInt();
      return cid;
    }
};