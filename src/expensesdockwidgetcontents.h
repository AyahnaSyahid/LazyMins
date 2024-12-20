#ifndef ExpensesDockWidgetContentsH
#define ExpensesDockWidgetContentsH

namespace Ui {
    class ExpensesDockWidgetContents;
}

#include <QWidget>

class QPoint;
class QAbstractItemModel;
class ExpensesDockWidgetContents : public QWidget {
    Q_OBJECT
public:
    ExpensesDockWidgetContents(QWidget* = nullptr);
    ~ExpensesDockWidgetContents();
    QString limitQuery() const;
    QString whereQuery() const;

public slots:
    void refreshModel();

private slots:
    void on_spinBox_valueChanged(int cv);
    void on_comboBulan_currentIndexChanged(int ix);
    void on_comboTahun_currentIndexChanged(const QString& tx);
    void horizontalHeaderContextMenu(const QPoint& pos);
    
private:
    Ui::ExpensesDockWidgetContents *ui;
    qint64 maxRowCount = 100;
    int currentPage = 1;
    int totalPageCount;
    QAbstractItemModel* modelExpenses;
    QAbstractItemModel* modelBulanan;
    QAbstractItemModel* modelTahunan;
};

#endif // ExpensesDockWidgetContentsH