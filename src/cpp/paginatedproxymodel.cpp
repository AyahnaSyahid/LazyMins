#include "paginatedproxymodel.h"
#include <QVariant>
#include <QtMath>
#include <QtDebug>

PaginatedProxyModel::PaginatedProxyModel(QObject* parent, int limit) :
    _page(1), _limit(limit), QSortFilterProxyModel(parent)
{
  setDynamicSortFilter(false);
}

PaginatedProxyModel::~PaginatedProxyModel() {}

int PaginatedProxyModel::maxPage() const {
    if(!sourceModel()) return 1;
    double sc = sourceModel()->rowCount();
    int rnd = qCeil(sc / _limit);
    return rnd;
}

bool PaginatedProxyModel::loadNext() {
    // qDebug() << "NEXT Current Page :" << _page << "Max Page : " << maxPage() ;
    if(_page >= maxPage()) return false;
    ++_page;
    if((_page + 1) == maxPage()) {
        if(sourceModel()->canFetchMore(QModelIndex()))
            sourceModel()->fetchMore(QModelIndex());
    }
    invalidateFilter();
    emit pageChanged(_page);
    return true;
}

bool PaginatedProxyModel::loadPrev() {
    // qDebug() << "PREV Current Page :" << _page << "Max Page : " << maxPage() ;
    if(_page < 2) return false;
    --_page;
    invalidateFilter();
    emit pageChanged(_page);
    return true;
}

bool PaginatedProxyModel::loadFirst() {
    // qDebug() << "FIRST Current Page :" << _page << "Max Page : " << maxPage() ;
    _page = 1;
    invalidateFilter();
    emit pageChanged(_page);
    return true;
}

bool PaginatedProxyModel::loadLast() {
    // qDebug() << "LAST Current Page :" << _page << "Max Page : " << maxPage() ;
    _page = maxPage();
    invalidateFilter();
    emit pageChanged(_page);
    if(sourceModel()->canFetchMore(QModelIndex()))
        sourceModel()->fetchMore(QModelIndex());
    return true;
}

void PaginatedProxyModel::changeLimit(int L)
{
    int cp = _page;
    _limit = L;
    int m = maxPage();
    emit maxChanged(m);
    if(cp > m)
    {
      _page = m;
      emit pageChanged(_page);
    }
    emit limitChanged(L);
    invalidateFilter();
}

bool PaginatedProxyModel::filterAcceptsRow(int sr, const QModelIndex& src) const
{
    auto first = (_page - 1) * _limit;
    auto last = _page * _limit;
    auto ok = (first <= sr) && (sr < last);
    return ok;
}