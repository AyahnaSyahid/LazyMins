#ifndef EditProductDialog_H
#define EditProductDialog_H

namespace Ui {
    class EditProductDialog;
}

#include <QDialog>
class QTableView;
class QSqlQueryModel;
class EditProductDialog : public QDialog {
    Q_OBJECT

public:
    EditProductDialog(QWidget* = nullptr);
    ~EditProductDialog();
    int currentProduct();
    bool setCurrentProduct(int);
    
private slots:
    void on_comboBox_currentIndexChanged(int);
    void on_saveButton_clicked();
    void on_doneButton_clicked();
    void on_resetButton_clicked();

signals:
    void productEdited();

private:
    Ui::EditProductDialog* ui;
    QTableView* comboView;
    QSqlQueryModel* comboModel;
};

#endif