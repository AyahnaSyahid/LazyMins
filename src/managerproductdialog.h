#ifndef ManagerProductDialog_H
#define ManagerProductDialog_H

namespace Ui {
    class ManagerProductDialog;
}

#include <QDialog>
#include <QModelIndex>

class QSqlTableModel;
class ManagerProductDialog : public QDialog {
    Q_OBJECT
public:
    ManagerProductDialog(QWidget* = nullptr);
    ~ManagerProductDialog();
    void accept() override;
    
    
private slots:
    void on_lineEdit_textChanged(const QString&);
    void on_pushButton_clicked();
    void on_tableView_doubleClicked(const QModelIndex&);
    void on_aDeleteSelectedIndexes_triggered();
    
private:
    Ui::ManagerProductDialog* ui;
    QSqlTableModel* tableModel;
};

#endif