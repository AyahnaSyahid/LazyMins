#include "managerakun.h"
#include "ui_managerakun.h"
#include "adduserdialog.h"
#include "userform.h"
#include "userbioeditor.h"
#include "resetpassword.h"
#include "passwordverification.h"
#include "loginnotifier.h"
#include "helper.h"
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QHeaderView>
#include <QPoint>
#include <QMenu>
#include <QAction>

#include <QDialog>
#include <QInputDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>

#define query_model findChild<QSqlQueryModel*>(QString(), Qt::FindDirectChildrenOnly)

class UserFormDialog;
ManagerAkun::ManagerAkun(QWidget* parent)
  : ui(new Ui::ManagerAkun), QDialog(parent)
{
    ui->setupUi(this);
    QSqlQueryModel* qm = new QSqlQueryModel(this);
    QString baseQuery(R"-(
            SELECT user_id, username, full_name, email, phone_number, home_address FROM users
        )-");
    qm->setProperty("baseQuery", baseQuery);
    qm->setQuery(baseQuery);
    qm->setHeaderData(1, Qt::Horizontal, "ID Login");
    qm->setHeaderData(2, Qt::Horizontal, "Nama User");
    qm->setHeaderData(3, Qt::Horizontal, "Email");
    qm->setHeaderData(4, Qt::Horizontal, "No. Tel");
    qm->setHeaderData(5, Qt::Horizontal, "Alamat");

    auto pxm = new QSortFilterProxyModel(this);
    pxm->setSourceModel(qm);
    ui->tableView->setModel(pxm);
    ui->tableView->hideColumn(0);
    ui->tableView->verticalHeader()->setMinimumSectionSize(18);
    ui->tableView->verticalHeader()->setDefaultSectionSize(18);
    ui->tableView->resizeColumnsToContents();
    auto smod = ui->tableView->selectionModel();
    ui->pushButton->setEnabled(false);
    connect(smod, &QItemSelectionModel::selectionChanged, this, [this, smod]
    {
        if(smod->hasSelection()) {
            ui->pushButton->setEnabled(true);
        } else {
            ui->pushButton->setEnabled(false);
        }
    });
    adjustSize();
}

ManagerAkun::~ManagerAkun()
{
    delete ui;
}

void ManagerAkun::editUserBio(qint64 uid)
{
    UserBioEditor* ube = new UserBioEditor(this);
    ube->setUser(uid);
    ube->setAttribute(Qt::WA_DeleteOnClose);
    ube->setWindowModality(Qt::NonModal);
    ube->adjustSize();
    connect(ube, SIGNAL(accepted()), SLOT(resetModel()));
    ube->open();
}

void ManagerAkun::resetModel()
{
    query_model->setQuery(query_model->property("baseQuery").toString());
}

void ManagerAkun::on_pushButton_clicked()
{
    auto current_index = ui->tableView->currentIndex();
    if(!current_index.isValid()) return;
    auto selected = UserItem(current_index.siblingAtColumn(0).data(Qt::EditRole).toLongLong());
    if(selected.user_id == 1) {
        MessageHelper::warning(this, "Akses Diblokir", "Root user tidak dapat dihapus!!");
        return;
    }
    auto logged = LoginNotifier::currentUser();
    if(logged == selected) {
        MessageHelper::warning(this, "Akses Diblokir", "Anda tidak dapat menghapus akun anda sendiri!!");
        return;
    }
    // enter password sebelum menghapus
    PasswordVerification pv(this);
    pv.setUser(logged);
    pv.setMaxRetries(3);
    if(pv.exec() == QDialog::Accepted) {
        selected.erase();
        resetModel();
        return;
    }
}

void ManagerAkun::on_pushButton2_clicked()
{
    auto aud = new AddUserDialog(this);
    aud->adjustSize();
    aud->setWindowModality(Qt::NonModal);
    aud->setAttribute(Qt::WA_DeleteOnClose);
    connect(aud, SIGNAL(userCreated(qint64)), SIGNAL(userCreated(qint64)));
    connect(aud, &AddUserDialog::accepted, this, [this]{
        query_model->setQuery(query_model->property("baseQuery").toString());
        
    });
    aud->open();
}

void ManagerAkun::on_tableView_doubleClicked(const QModelIndex& mi)
{
    if(!mi.isValid()) return;
    auto uid = mi.siblingAtColumn(0).data(Qt::EditRole).toLongLong();
    if(uid > 0) editUserBio(uid);
}

void ManagerAkun::on_tableView_customContextMenuRequested(const QPoint& pos)
{
    auto gpos = ui->tableView->viewport()->mapToGlobal(pos);
    if(ui->tableView->selectionModel()->hasSelection()) {
        auto sIndex = ui->tableView->selectionModel()->selectedRows(1)[0];
        auto iUser = sIndex.siblingAtColumn(0).data(Qt::EditRole).toLongLong();
        QMenu menu;
        auto act1 = menu.addAction("Edit Bio");
        auto act2 = menu.addAction("Reset Password");
        auto sel = menu.exec(gpos);
        if(sel == act1) {
            editUserBio(iUser);
            return;
        } else if (sel == act2) {
            auto *rp = new ResetPassword(this);
            rp->setUser(iUser);
            rp->setAttribute(Qt::WA_DeleteOnClose);
            rp->adjustSize();
            connect(rp, &ResetPassword::rejected, this,
                []{ MessageHelper::information(nullptr, "Dibatalkan", "Password tidak berubah"); } );
            connect(rp, &ResetPassword::accepted, this, 
                []{ MessageHelper::information(nullptr, "Berhasil", "Password berhasil diubah"); } );
            rp->open();
        }
    }
}