#ifndef EditOrderDialog_H
#define EditOrderDialog_H

namespace Ui {
    class EditOrderDialog;
}

#include <QDialog>
class ManagerOrderDialog;

class EditOrderDialog : public QDialog {
    Q_OBJECT

public:
    EditOrderDialog(QWidget* = nullptr);
    ~EditOrderDialog();
    void setOrder(qint64 oid);

private slots:
    void on_pushButton_clicked();
    void on_comboBox2_currentIndexChanged(int);
    void setProdukSelectorEnabled(bool);
    void updateHarga();

private :
    Ui::EditOrderDialog* ui;
    qint64 order_id = -1;
    friend class ManagerOrderDialog;
};

#endif