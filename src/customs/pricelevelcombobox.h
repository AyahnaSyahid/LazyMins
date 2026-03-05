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
      auto matches = model()->match(model()->index(0, 0), id, 1, Qt::MatchExactly);
      if (!matches.isEmpty()) {
        setCurrentIndex(matches.first().row());
      } else {
        setCurrentIndex(-1);
      }
    }
    
    int currentId () const {
      if (currentIndex() < 0) return -1;
      auto cid = qmodel->index(currentIndex(), 0).data(Qt::EditRole).toInt();
      return cid;
    }
};