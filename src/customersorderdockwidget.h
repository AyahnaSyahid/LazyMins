#ifndef CustomersOrderDockWidgetH
#define CustomersOrderDockWidgetH
    
namespace Ui {
    class CustomersOrderDockWidget;
}

#include <QDockWidget>
class QPoint;
class CustomersOrderProxy;
class CustomersOrderDockWidget : public QDockWidget {
    Q_OBJECT

public:
    CustomersOrderDockWidget(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~CustomersOrderDockWidget();

    QString limitQuery() const;
    QString whereQuery() const;
    QString baseQuery() const;

public slots:
    void refreshModel();
    void on_actionView_triggered();
    void on_actionEditOrder_triggered();
    void on_actionHapusOrder_triggered();

private slots:
    void headerCustomContextMenu(const QPoint& p);
    void on_tableView_doubleClicked(const QModelIndex&);
    void onTableChanged(const QString&);
    
signals:
    void orderAdded();
    void orderRemoved();
    void orderModified();

private:
    Ui::CustomersOrderDockWidget* ui;
    int _totalPage;
    int _currentPage = 1;
    int _limit = 100;
};

#endif // CustomersOrderDockWidgetH