#ifndef ManagerOrderDialog_H
#define ManagerOrderDialog_H

namespace Ui {
    class ManagerOrderDialog;
}

#include <QDialog>

class QSqlRelationalTableModel;
class ManagerOrderDialog : public QDialog {
    Q_OBJECT

public:
    ManagerOrderDialog(QWidget* =nullptr);
    ~ManagerOrderDialog();
    void setNotaId(int);

private slots:
    void on_tableView_customContextMenuRequested(const QPoint&);
    void on_tableView_doubleClicked(const QModelIndex& mi);
    void applyFilter();
    
protected:
    QString filterString() const;

private:
    Ui::ManagerOrderDialog* ui;
    QSqlRelationalTableModel* omod;
};

#endif