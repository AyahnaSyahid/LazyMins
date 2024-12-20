#ifndef ExpensesDockWidgetH
#define ExpensesDockWidgetH

namespace Ui {
    class ExpensesDockWidget;
}

#include <QDockWidget>

class ExpensesDockWidget : public QDockWidget {
    Q_OBJECT
public:
    ExpensesDockWidget(QWidget* = nullptr);
    ~ExpensesDockWidget();
private:
    Ui::ExpensesDockWidget* ui;
};

#endif // ExpensesDockWidgetH