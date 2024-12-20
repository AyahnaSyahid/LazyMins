#ifndef PaginatedTableViewH
#define PaginatedTableViewH

#include <QTableView>

class QAction;
class QContextMenuEvent;
class QAbstractItemModel;
class PaginatedProxyModel;
class PaginatedTableView : public QTableView
{
  Q_OBJECT
  public:
    PaginatedTableView(QWidget* =nullptr);
    ~PaginatedTableView();
    void setModel(QAbstractItemModel* tm) override;
    void setLimit(int limit);

    int maxPage() const;
    int limit() const;
    int currentPage() const;
    inline void setContextMenuShown(bool s=true) { setProperty("draw_menu_event", true); }
    
  signals:
    void pageChanged(int p);
    void rowPerPageChanged(int p);
    void maxPageChanged(int p);
  
  private:
    void contextMenuEvent(QContextMenuEvent*) override;
    PaginatedProxyModel* proxy;
    QAction* displayFirst;
    QAction* displayNext;
    QAction* displayPrev;
    QAction* displayLast;
    QAction* changeLimit;
};

#endif // PaginatedTableViewH