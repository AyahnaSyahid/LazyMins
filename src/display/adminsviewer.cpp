#include "adminsviewer.h"
#include "ui_dataviewer.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QStyledItemDelegate>

#include "src/dialogs/userdialog.h"
#include "src/controllers/users.h"

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

void AdminsViewer::editUser(int userId) {
    UserController uc;
    auto rc = uc.getUserRecord(userId);
    if(rc.isEmpty()) {
        QMessageBox::warning(this, "Kesalahan", "Data User tidak dapat ditemukan");
        return ;
    }
    UserDialog ud(this);
    ud.prepareModify(rc);
    connect(&ud, &QDialog::accepted, this, &DataViewer::refresh);
    ud.exec();
}

void AdminsViewer::changeUserPassword(int userId) {}

void AdminsViewer::on_dataView_customContextMenuRequested(const QPoint &pt)
{
    QMenu ctx;
    ctx.setToolTipsVisible(true);
    auto ix = Ui()->dataView->indexAt(pt);
    
    if (!ix.isValid()) return;
    
    auto edat = ctx.addAction("Edit Data");
    auto epas = ctx.addAction("Reset Password");

}