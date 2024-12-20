#ifndef EditCustomerDialog_H
#define EditCustomerDialog_H

namespace Ui {
    class EditCustomerDialog;
}

#include <QDialog>

class QSqlTableModel;
class QTableView;

class EditCustomerDialog : public QDialog {
    Q_OBJECT

public:
    EditCustomerDialog(QWidget* = nullptr);
    ~EditCustomerDialog();
    int currentCustomer();
    bool setCurrentCustomer(int);

private slots:
    void on_pushButton_clicked();
    void on_comboBox_currentIndexChanged(int);

signals:
    void customerEdited();
    
private:
    Ui::EditCustomerDialog* ui;
    QSqlTableModel* comboModel;
    QTableView* comboView;
};
#endif