<<<<<<< HEAD
#ifndef CreateProductDialog_H
#define CreateProductDialog_H
=======
#ifndef CreateProductDilaog_H
#define CreateProductDilaog_H
>>>>>>> refs/remotes/origin/new-start

#include <QDialog>

namespace Ui {
<<<<<<< HEAD
    class CreateProductDialog;
}

class CreateProductDialog : public QDialog {
    Q_OBJECT
public:
    explicit CreateProductDialog(QWidget* = nullptr);
    ~CreateProductDialog();

protected slots:
    virtual void on_pushButton_clicked();

protected:
    Ui::CreateProductDialog* ui;
=======
    class CreateProductDilaog;
}

class CreateProductDilaog : public QDialog {
    Q_OBJECT
public:
    explicit CreateProductDilaog(QWidget* = nullptr);
    ~CreateProductDilaog()

private slots:
    void on_pushButton_clicked();

private:
    Ui::CreateProductDilaog* ui;
>>>>>>> refs/remotes/origin/new-start
};

#endif