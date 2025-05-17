#ifndef CustomerPickerDialog_H
#define CustomerPickerDialog_H

#include <QDialog>

namespace Ui {
    class CustomerPickerDialog;
}

class QAbstractItemModel;
class QSortFilterProxyModel;
class QModelIndex;
class CustomerPickerDialog : public QDialog {
    Q_OBJECT
public:
    explicit CustomerPickerDialog(QAbstractItemModel* src, QWidget* parent);
    ~CustomerPickerDialog();

private slots:
    // void on_filterEdit_textChanged(const QString&);
    void on_customerView_clicked(const QModelIndex&);

private:
    int toSourceIndex(const QModelIndex&);

signals:
    void activated(int i);

private:
    Ui::CustomerPickerDialog* ui;
    QSortFilterProxyModel* proxy;
};
    

#endif