#ifndef ManagerNotaDockerH
#define ManagerNotaDockerH

#include <QDockWidget>

namespace Ui {
    class ManagerNotaDockWidget;
}
class QAbstractTableModel;
class ManagerNotaDockWidget : public QDockWidget {
    Q_OBJECT

public:
    ManagerNotaDockWidget(QWidget* parent=nullptr);
    ~ManagerNotaDockWidget();
    void setLimit(int l);
    void setPage(int pg);
    void applyFiltering();

protected:
    qint64 invoicesCount() const;
    QString baseQuery() const;
    QString whereClause() const;
    QString limitClause() const;

private slots:
    void on_tableView_customContextMenuRequested(const QPoint &);
    void on_lineEdit_textChanged(const QString &text);
    void onTableChanged(const QString& s);
    void navButtonHandler();

private:
    int _limit;
    int _currentPage;
    Ui::ManagerNotaDockWidget *ui;
    QAbstractTableModel *invoiceModel;
};

#endif