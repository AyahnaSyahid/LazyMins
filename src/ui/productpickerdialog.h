#ifndef ProductPickerDialog_H
#define ProductPickerDialog_H

#include <QDialog>

namespace Ui {
    class ProductPickerDialog;
}

class QAbstractItemModel;
class QSortFilterProxyModel;
class QModelIndex;
class ProductPickerDialog : public QDialog {
    Q_OBJECT
public:
    explicit ProductPickerDialog(QAbstractItemModel* src, QWidget* parent);
    ~ProductPickerDialog();

private slots:
    // void on_filterEdit_textChanged(const QString&);
    void on_productView_clicked(const QModelIndex&);

private:
    int toSourceIndex(const QModelIndex&);

signals:
    void activated(int i);

private:
    Ui::ProductPickerDialog* ui;
    QSortFilterProxyModel* proxy;
};
    

#endif