#include <QDockWidget>

#include "mainwindow.h"
#include "../ui/ui_mainwindow.h"
#include "loginnotifier.h"
#include "logindialog.h"
#include "managerakun.h"
#include "useritem.h"
#include "userrolesitem.h"
#include "userbioeditor.h"
#include "rolepermissionsitem.h"
#include "userpermissions.h"
#include "customeritem.h"
#include "productitem.h"
#include "passwordverification.h"
#include "customermodule.h"
#include "expensemodule.h"
#include "ordersmodule.h"
#include "productsmodule.h"
#include "config.h"

MainWindow::MainWindow(QWidget* parent)
: ui(new Ui::MainWindow), QMainWindow(parent) {
  ui->setupUi(this);
  auto ln = LoginNotifier::instance();
  connect(ln, SIGNAL(userLoggedIn(qint64)), this, SLOT(userLoggedIn(qint64)));
  connect(ln, SIGNAL(userLoggedOut(qint64)), this, SLOT(userLoggedOut(qint64)));
  menuBar()->setMinimumHeight(24);
  menuBar()->setContentsMargins(0, 5, 0, 5);
  
  auto customerModule =  new CustomerModule(this);
  customerModule->setObjectName("customerModule");
  customerModule->installMenu(this);
  customerModule->installDocker(this);
  
  auto expenseModule = new ExpenseModule(this);
  expenseModule->setObjectName("expenseModule");
  expenseModule->installMenu(this);
  expenseModule->installDocker(this);
  
  auto ordersModule = new OrdersModule(this);
  ordersModule->setObjectName("ordersModule");
  ordersModule->installMenu(this);
  ordersModule->installDocker(this);
  
  auto productsModule = new ProductsModule(this);
  productsModule->setObjectName("productsModule");
  productsModule->installMenu(this);
  productsModule->installDocker(this);
  
  auto user = LoginNotifier::currentUser();
  if(user.user_id > 0)
      userLoggedIn(user.user_id);
}

void MainWindow::userLoggedIn(qint64 uid)
{
#define PRE_CheckPermission(USR, PERM) \
    UserPermissions::hasPermission(UserItem(USR), PermissionItem(PERM))
    UserItem uim(uid);
    ui->actionEditBio->setVisible(true);
    ui->actionMasuk->setText("Keluar");
    if(PRE_CheckPermission(uid, "ManageUser")) {
        ui->actionManager->setVisible(true);
    } else {
        ui->actionManager->setVisible(false);
    }
    setWindowTitle(QString("Masuk Sebagai : %1").arg(uim.full_name.isEmpty() ? uim.username : uim.full_name));
    
    auto acts = findChildren<QObject*>();
    for(auto act=acts.begin(); act!=acts.end(); ++act) {
        // enableWithPerm properties
        auto val = (*act)->property("enableWithPerm");
        if(val.isValid()) {
            auto permitted = PRE_CheckPermission(uid, val.toString());
            // Action Part
            auto isAction = qobject_cast<QAction*>(*act);
            if(isAction) {
                isAction->setEnabled(permitted);
                if(!permitted) {
                    if(isAction->isCheckable() && isAction->isChecked())
                        isAction->setChecked(false);
                }
            }
            // Menu Part
            auto isMenu = qobject_cast<QMenu*>(*act);
            if(isMenu) {
                isMenu->setEnabled(permitted);
            };
            
            // Dock Widget
            auto isDockWidget = qobject_cast<QDockWidget*>(*act);
            if(isDockWidget) {
                if(isDockWidget->isVisible()) {
                    isDockWidget->setVisible(permitted);
                }
            }
        }
    }
#undef CheckPermission
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::userLoggedOut(qint64 luid)
{
    setWindowTitle("");
    ui->actionMasuk->setText("Masuk");
    ui->actionManager->setVisible(false);
    ui->actionEditBio->setVisible(false);

    auto uid = LoginNotifier::currentUser().user_id;

    auto acts = findChildren<QObject*>();
    for(auto act=acts.begin(); act!=acts.end(); ++act) {
        // enableWithPerm properties
        auto val = (*act)->property("enableWithPerm");
        if(val.isValid()) {
            auto permitted = PRE_CheckPermission(uid, val.toString());
            // Action Part
            auto isAction = qobject_cast<QAction*>(*act);
            if(isAction) {
                isAction->setEnabled(permitted);
                if(!permitted) {
                    if(isAction->isCheckable() && isAction->isChecked())
                        isAction->setChecked(false);
                }
            }
            // Menu Part
            auto isMenu = qobject_cast<QMenu*>(*act);
            if(isMenu) {
                isMenu->setEnabled(permitted);
            };
            
            // Dock Widget
            auto isDockWidget = qobject_cast<QDockWidget*>(*act);
            if(isDockWidget) {
                if(isDockWidget->isVisible())
                    isDockWidget->setVisible(permitted);
            }
        }
    }
}

void MainWindow::on_actionMasuk_triggered()
{
    auto acm = qobject_cast<QAction*>(sender());
    if(!acm) return;
    if(GlobalNamespace::logged_user_id > 0) {
        LoginNotifier::instance()->logout();
    } else {
        LoginDialog(this).exec();
    }
}

void MainWindow::on_actionManager_triggered()
{
    PasswordVerification pv(this);
    pv.setUser(LoginNotifier::currentUser());
    if(pv.exec() != QDialog::Accepted) return;
    ManagerAkun* ma = findChild<ManagerAkun*>();
    if(!ma) {
        auto ma = new ManagerAkun(this);
        ma->setAttribute(Qt::WA_DeleteOnClose);
        ma->setWindowModality(Qt::NonModal);
        ma->show();
    } else {
        ma->raise();
    }
}

void MainWindow::on_actionEditBio_triggered()
{
    auto cu = LoginNotifier::currentUser();
    auto ube = new UserBioEditor(this);
    ube->setAttribute(Qt::WA_DeleteOnClose);
    ube->setUser(cu);
    ube->open();
    connect(ube, &QDialog::accepted, [this](){
        auto cu = LoginNotifier::currentUser();
        setWindowTitle(QString("Masuk Sebagai : %1").arg(cu.full_name.isEmpty() ? cu.username : cu.full_name));
    });
}
