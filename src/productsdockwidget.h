#ifndef ProductsDockWidgetH
#define ProductsDockWidgetH

namespace Ui {
    class ProductsDockWidget;
}

#include <QDockWidget>

class ProductsDockWidget : public QDockWidget {
    Q_OBJECT

public:
    ProductsDockWidget(QWidget* =nullptr);
    ~ProductsDockWidget();
    QString baseQuery() const;
    QString limitQuery() const;
    QString whereQuery() const;

public slots:
    void refreshModel();

private slots:
    void on_actionCSVUpdate_triggered();

signals:
    void productsUpdated();
    void productAdded();
    void productModified();
    void productRemoved();

private:
    qint64 _totalPage;
    qint64 _limit;
    qint64 _currentPage = 1;
    Ui::ProductsDockWidget *ui;
};

#endif // ProductsDockWidgetH
