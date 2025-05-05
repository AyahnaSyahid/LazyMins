#ifndef CreateProductDilaog_H
#define CreateProductDilaog_H

#include <QDialog>

namespace Ui {
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
};

#endif