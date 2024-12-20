#ifndef AddOrderDialog_H
#define AddOrderDialog_H

namespace Ui{
    class AddOrderDialog;
}

#include <QDialog>

class TmpSaleModel;
class QSqlTableModel;
class QTableView;
class AddOrderDialog : public QDialog {
    Q_OBJECT

public:
    AddOrderDialog(QWidget* =nullptr);
    ~AddOrderDialog();
    void setCustomer(int);
    void setResetButtonEnabled(bool);

public slots:
    void updateModel();

private slots:
    void on_comboBox_currentIndexChanged(int);
    void on_comboBox_currentIndexChanged(QString);
    void revertIndex(const QString&);
    void on_comboBoxProduct_currentIndexChanged(int);
    void on_pushButtonCustomer_clicked();
    void on_pushButtonStore_clicked();
    void on_pushButtonReset_clicked();
    void on_pushButtonBayar_clicked();
    void on_pushButtonSave_clicked();
    void on_doubleSpinBoxP_valueChanged(double);
    void on_doubleSpinBoxT_valueChanged(double);
    void on_spinBoxH_valueChanged(int);
    void on_doubleSpinBoxQ_valueChanged(double);
    void updateTotal();
    void on_actionDeleteSelectedItems_triggered();
    void hideAreaInput();
    void showAreaInput();
    
signals:
    void orderAdded();

private:
    Ui::AddOrderDialog* ui;
    TmpSaleModel* tmpModel;
    QSqlTableModel* custModel;
    QSqlTableModel* prodModel;
    QTableView* custView;
    QTableView* prodView;
};
#endif
