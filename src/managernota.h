#ifndef ManagerNotaH
#define ManagerNotaH

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
    class ManagerNota;
};

class ManagerNota : public QDialog
{
    Q_OBJECT
public:
    ManagerNota(QWidget* parent=nullptr);
    ~ManagerNota();

public slots:
    void on_tableView_customContextMenuRequested(const QPoint& p);
    void actEditDiscount();
    void actEditSale();
    void actEditPayment();
    void actDeleteNota();
    void updateModel();

private:
    Ui::ManagerNota* ui;
    QSqlQueryModel model;
};

#endif