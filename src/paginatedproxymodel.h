#ifndef PaginatedProxyModelH
#define PaginatedProxyModelH
#include <QSortFilterProxyModel>

class PaginatedProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    public:
        PaginatedProxyModel(QObject* parent=nullptr, int limit=100);
        ~PaginatedProxyModel();

    public slots:
        bool loadNext();
        bool loadPrev();
        bool loadFirst();
        bool loadLast();
        void changeLimit(int L);
        int maxPage() const;
        const int& limit() const { return _limit; }
        const int& currentPage() const { return _page; }
        
    protected:
        bool filterAcceptsRow(int sr, const QModelIndex& src) const override;
    
    signals:
        void pageChanged(int cp);
        void limitChanged(int cp);
        void maxChanged(int m);

    private:
        int _page, _limit;
};

#endif // PaginatedProxyModelH
