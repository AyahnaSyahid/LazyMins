#ifndef EditOrderDialog_H
#define EditOrderDialog_H

#include <QDialog>
#include <QSqlRecord>

class QSqlError;
class Database;

namespace Ui {
    class EditOrderDialog;
}

class EditOrderDialog : public QDialog {
    Q_OBJECT

public:
    explicit EditOrderDialog(const QSqlRecord&, Database*, QWidget* =nullptr);
    ~EditOrderDialog();

private slots:
    void on_pickDate_clicked();
    void on_cancelButton_clicked();
    void on_saveButton_clicked();
    void on_productBox_currentIndexChanged(int);
    void setCurrentOrderDate(const QDate&);
    void updateSubTotal();

signals:
    void queryUpdate(const QSqlRecord& r);

private:
    QSqlRecord _record;
    Ui::EditOrderDialog* ui;
    Database* db;
};

#endif