#ifndef PembuatNota_H
#define PembuatNota_H

#include <QWidget>

namespace Ui {
    class PembuatNota;
}

class QDialog;
class QAbstractItemModel;
class QPoint;
class PembuatNota: public QWidget {
    Q_OBJECT

private:
    Ui::PembuatNota* ui;
    QAbstractItemModel *ordersModel,
                       *customersModel,
                       *markedModel;
    QList<int> marked;

public:
    explicit PembuatNota(QWidget* parent=nullptr);
    ~PembuatNota();
    inline const QList<int> &markedId() const { return marked; }

public slots:
    bool setCustomer(qint64 cid);
    void markSale(qint64 sid);

private slots:
    void fix_View();
    void on_tableView_doubleClicked(const QModelIndex& ix);
    void on_tableView2_doubleClicked(const QModelIndex& ix);
    void savedSelectionChanged();
    void on_pushButton_clicked();
    void on_pushButton4_clicked();
    void on_comboBox_currentIndexChanged(int);
    void on_pushButton3_clicked();
    void on_pushButton5_clicked();
    void on_pushButton2_clicked();
    void tableViewHeaderContextMenu(const QPoint& p);
    void onTableChanged(const QString& tn);
    
    
protected slots:
    void reloadModels();

signals:
    void penjualanBaru();
    void notaBaru();
};

#endif