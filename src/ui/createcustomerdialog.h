#ifndef CreateCustomerDialog_H
#define CreateCustomerDialog_H

#include <QDialog>

namespace Ui {
    class CreateCustomerDialog;
}

class Database;
class QSqlRecord;
class CreateCustomerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateCustomerDialog(Database*, QWidget* =nullptr);
    ~CreateCustomerDialog();

private slots:
    void on_pushButton_clicked();
    void reset_input();

signals:
    void queryInsert(const QSqlRecord&);

protected:
    Ui::CreateCustomerDialog* ui;

private:
    Database* db;
};

#endif // AddCustomerDialog_H