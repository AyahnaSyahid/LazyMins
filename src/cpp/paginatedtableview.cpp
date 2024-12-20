#include "paginatedtableview.h"
#include "paginatedproxymodel.h"
#include <QContextMenuEvent>
#include <QAction>
#include <QMenu>

PaginatedTableView::PaginatedTableView(QWidget* parent) :
  proxy(new PaginatedProxyModel(this)),
  QTableView(parent)
{
    setHorizontalScrollMode(QTableView::ScrollPerPixel);
    setVerticalScrollMode(QTableView::ScrollPerPixel);
    displayFirst = new QAction("First", this);
    // displayFirst->setShortcut("ctrl+1");
    displayPrev = new QAction("Prev", this);
    // displayPrev->setShortcut("ctrl+b");
    displayNext = new QAction("Next", this);
    // displayPrev->setShortcut("ctrl+b");
    displayLast = new QAction("Last", this);
    
    QTableView::setModel(proxy);
    
    connect(proxy, SIGNAL(pageChanged(int)), SIGNAL(pageChanged(int)));
    connect(proxy, &PaginatedProxyModel::pageChanged, [this](int p){
        setWindowTitle(QString("Showing page %1 of %2 with max_row : %3")
            .arg(p)
            .arg(proxy->maxPage())
            .arg(proxy->limit()));
    });
    connect(proxy, SIGNAL(limitChanged(int)), SIGNAL(rowPerPageChanged(int)));
    connect(proxy, SIGNAL(maxChanged(int)), SIGNAL(maxPageChanged(int)));
    connect(displayFirst, SIGNAL(triggered()), proxy, SLOT(loadFirst()));
    connect(displayPrev, SIGNAL(triggered()), proxy, SLOT(loadPrev()));
    connect(displayNext, SIGNAL(triggered()), proxy, SLOT(loadNext()));
    connect(displayLast, SIGNAL(triggered()), proxy, SLOT(loadLast()));
    setContextMenuShown(true);
}

PaginatedTableView::~PaginatedTableView() {}

void PaginatedTableView::setLimit(int l)
{
    proxy->changeLimit(l);
}

void PaginatedTableView::setModel(QAbstractItemModel* m)
{
    proxy->setSourceModel(m);
}

int PaginatedTableView::maxPage() const {
    return proxy->maxPage();
}

int PaginatedTableView::limit() const {
    return proxy->limit();
}

int PaginatedTableView::currentPage() const {
    return proxy->currentPage();
}

void PaginatedTableView::contextMenuEvent(QContextMenuEvent* evt)
{
    auto dispMenu = property("draw_menu_event");
    if(dispMenu.isValid() && dispMenu.toBool()) {
        auto pos = evt->globalPos();
        QMenu autoMenu;
        auto cp = proxy->currentPage();
        auto mp = proxy->maxPage();
        displayFirst->setEnabled(cp > 1);
        displayPrev->setEnabled(cp > 1);
        displayNext->setEnabled(cp < mp);
        displayLast->setEnabled(cp < mp);
        autoMenu.addSection("Navigasi");
        autoMenu.addAction(displayFirst);
        autoMenu.addAction(displayPrev);
        autoMenu.addAction(displayNext);
        autoMenu.addAction(displayLast);
        autoMenu.addSeparator();
        autoMenu.exec(pos);
    }
}