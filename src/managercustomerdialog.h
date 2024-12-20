#ifndef ManagerCustomerDialog_H
#define ManagerCustomerDialog_H

namespace Ui {
    class ManagerCustomerDialog;
}

#include <QDialog>
#include <QPoint>

class QSqlTableModel;
class ManagerCustomerDialog : public QDialog {
    Q_OBJECT

public:
    ManagerCustomerDialog(QWidget* = nullptr);
    ~ManagerCustomerDialog();

public slots:
    void refreshModel();

private slots:
    void on_lineEdit_textChanged(const QString&);
    void on_pushButton_clicked();
    void on_tableView_doubleClicked(const QModelIndex&);
    void tableHeaderContextMenu(const QPoint& pos);
    void contextMenuHandler(bool t);
    
signals:
    void customerRemoved();
    
private:
    Ui::ManagerCustomerDialog* ui;
    QSqlTableModel* custModel;
};

#endif