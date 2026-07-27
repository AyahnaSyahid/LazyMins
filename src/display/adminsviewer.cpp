#include "adminsviewer.h"
#include "ui_dataviewer.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QStyledItemDelegate>

#include "src/controllers/users.h"
#include "src/dialogs/edituserdialog.h"
#include "src/dialogs/edituserlogininfodialog.h"

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
    ui = Ui();
    setObjectName("adminsViewer");
    setWindowTitle("Data Admins");
    setQueryArgs(R"-(
        SELECT admins.id, 
               rl.role_name, 
               username, 
               nama_lengkap,
               email,
               nomor_telp,
               is_active,
               role_id,
               last_login 
          FROM admins 
          JOIN roles rl 
            ON admins.role_id = rl.id)-");
    setColumnVisible("role_id", false);
    ui->dataView->model()->setHeaderData(0, Qt::Horizontal, "AdminID", Qt::DisplayRole);
    ui->dataView->model()->setHeaderData(1, Qt::Horizontal, "Role", Qt::DisplayRole);
    ui->dataView->model()->setHeaderData(2, Qt::Horizontal, "Username", Qt::DisplayRole);
    ui->dataView->model()->setHeaderData(3, Qt::Horizontal, "Nama Lengkap", Qt::DisplayRole);
    ui->dataView->model()->setHeaderData(4, Qt::Horizontal, "Email", Qt::DisplayRole);
    ui->dataView->model()->setHeaderData(5, Qt::Horizontal, "Nomor Telp", Qt::DisplayRole);
    ui->dataView->model()->setHeaderData(6, Qt::Horizontal, "Aktif", Qt::DisplayRole);
    ui->dataView->model()->setHeaderData(8, Qt::Horizontal, "Terakhir Login", Qt::DisplayRole);
    adjustColumns();
    setEditable(false);
    ui->dataView->setItemDelegate(new Delegate(this));
    ui->dataView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->dataView, &QTableView::customContextMenuRequested, this, &AdminsViewer::on_dataView_customContextMenuRequested);
}

AdminsViewer::~AdminsViewer() {}

void AdminsViewer::editUser(int userId)
{
    UserController uc;
    auto rc = uc.getUserRecord(userId);
    if (rc.isEmpty())
    {
        QMessageBox::warning(this, "Kesalahan", "Data User tidak dapat ditemukan");
        return;
    }
    EditUserDialog ud(rc.value("username").toString(), this);
    connect(&ud, &QDialog::accepted, this, &DataViewer::refresh);
    ud.exec();
}

void AdminsViewer::changeUserPassword(int userId)
{
    UserController uc;
    auto rc = uc.getUserRecord(userId);
    if (rc.isEmpty())
    {
        QMessageBox::warning(this, "Kesalahan", "Data User tidak dapat ditemukan");
        return;
    }
    EditUserLoginInfoDialog ud(rc.value("username").toString(), this);
    connect(&ud, &QDialog::accepted, this, &DataViewer::refresh);
    ud.exec();
}

void AdminsViewer::on_dataView_customContextMenuRequested(const QPoint &pt)
{
    QMenu ctx;
    ctx.setToolTipsVisible(true);
    auto ix = ui->dataView->indexAt(pt);

    if (!ix.isValid())
        return;

    auto edat = ctx.addAction("Edit Data");
    auto epas = ctx.addAction("Reset Password");

    connect(edat, &QAction::triggered, [this, ix]()
            { editUser(ix.siblingAtColumn(0).data().toInt()); });
    connect(epas, &QAction::triggered, [this, ix]()
            { changeUserPassword(ix.siblingAtColumn(0).data().toInt()); });
    ctx.exec(ui->dataView->viewport()->mapToGlobal(pt));
}