#include "adminsviewer.h"
#include "ui_dataviewer.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QStyledItemDelegate>

namespace
{
    class Delegate : public QStyledItemDelegate
    {
    public:
        explicit Delegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    protected:
        void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override
        {
            QStyledItemDelegate::initStyleOption(option, index);
            switch (index.column())
            {
            case 0:
                option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
                break;
            case 1:
            case 8:
                option->displayAlignment = Qt::AlignCenter;
                break;
            }
        }
    };
}
AdminsViewer::AdminsViewer(QWidget *parent) : DataViewer(parent)
{
    auto ui = Ui();
    setWindowTitle("Data Admins");
    setQueryArgs(R"-(
        SELECT id, 
               rl.role_name, 
               username, 
               nama_lengkap,
               email,
               nomor_telp,
               is_active,
               role_id,
               last_seen 
          FROM admins 
          JOIN roles rl 
            ON admins.role_id = rl.id)-");
    setColumnVisible("role_id", false);
    setEditable(false);
    ui->dataView->setItemDelegate(new Delegate(this));
    ui->dataView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->dataView, &QTableView::customContextMenuRequested, this, &AdminsViewer::on_dataView_customContextMenuRequested);
}

AdminsViewer::~AdminsViewer() {}

void AdminsViewer::on_dataView_customContextMenuRequested(const QPoint &pt)
{
    QMenu ctx;
    ctx.setToolTipsVisible(true);
    auto ix = Ui()->dataView->indexAt(pt);
    if (ix.isValid())
    {
        ctx.addAction("Reset Password")->setData(ix);
        ctx.addAction("Reset Pin")->setData(ix);
}