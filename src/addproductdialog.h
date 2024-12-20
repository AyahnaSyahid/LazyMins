#ifndef AddProductDialog_H
#define AddProductDialog_H

#include <QDialog>
namespace Ui{
    class AddProductDialog;
}

class AddProductDialog : public QDialog {
    Q_OBJECT

public:
    AddProductDialog(QWidget* = nullptr);
    ~AddProductDialog();

signals:
    void productAdded(int);

private slots:
    void on_doneButton_clicked();
    void on_saveButton_clicked();
    void on_resetButton_clicked();

private:
    Ui::AddProductDialog* ui;
};

#endif