#ifndef ItemDelegatesH
#define ItemDelegatesH

#include <QStyledItemDelegate>
class DelegateHarga : public QStyledItemDelegate
{
    public:
        DelegateHarga(QObject* parent=nullptr) : QStyledItemDelegate(parent) {}
        QWidget* createEditor(QWidget* parent,const QStyleOptionViewItem& opt, const QModelIndex& mi) const override;
        void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        void setEditorData(QWidget *editor, const QModelIndex &index) const override;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};

class DelegateQty : public DelegateHarga
{
    public:
        DelegateQty(QObject* parent=nullptr) : DelegateHarga(parent) {}
        QWidget* createEditor(QWidget* parent,const QStyleOptionViewItem& opt, const QModelIndex& mi) const override;
};

class LihatHari : public QStyledItemDelegate
{
        bool incTime;
    public:
        LihatHari(QObject* =nullptr);
        QString displayText(const QVariant&, const QLocale&) const;
        void setIncludeTime(bool =true);
};

#endif //ItemDelegatesH