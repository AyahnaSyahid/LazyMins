#ifndef ExpenseModuleH
#define ExpenseModuleH

// Mencatat Pengeluaran Divisi
#include <QObject>

class QMainWindow;
class ExpenseModule : public QObject {
    Q_OBJECT
    QMainWindow* mainWindow;

public:
    ExpenseModule(QObject* =nullptr);
    void installMenu(QMainWindow* mw);
    void installDocker(QMainWindow* mw);

private slots:
    void onWriteExpenseNote();

signals:
    void expenseAdded();
    void expenseModified();
    void expenseRemoved();
};

#endif // ExpenseModuleH