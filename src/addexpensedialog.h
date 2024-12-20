#ifndef AddExpenseDialogH
#define AddExpenseDialogH

#include <QDialog>

namespace Ui {
    class AddExpenseDialog;
}

class AddExpenseDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddExpenseDialog(QWidget* =nullptr);
    ~AddExpenseDialog();

private slots:
    void on_pushButton_clicked();

signals:
    void expenseAdded();
    void divisionAdded();
    void expenseModified();

protected:
    virtual void accept() override;
    Ui::AddExpenseDialog* ui;

};

class EditExpenseDialog : public AddExpenseDialog {
public:
    explicit EditExpenseDialog(qint64 expenseId, QWidget* =nullptr);

private:
    void accept() override;

private:
    qint64 expense_id = -1;
};
#endif // AddExpenseDialogH