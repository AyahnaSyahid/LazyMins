#ifndef RemoveProductDialog_H
#define RemoveProductDialog_H

namespace Ui {
    class RemoveProductDialog;
}

#include <QDialog>

class QSqlTableModel;
class RemoveProductDialog : public QDialog {
    Q_OBJECT
public:
    RemoveProductDialog(QWidget* = nullptr);
    ~RemoveProductDialog();
private slots:
    void on_lineEdit_textChanged(const QString&);
    void on_pushButton_clicked();

private:
    Ui::RemoveProductDialog* ui;
    QSqlTableModel* tableModel;
};

#endif