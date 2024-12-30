#include "permissioneditor.h"
#include "userpermissions.h"
#include <QListWidget>
#include <QVBoxLayout>
#include <QListWidgetItem>
#include <QSqlQuery>

int ID_ROLE = 2210;
int DB_STAT = 2211;

PermissionEditor::PermissionEditor(qint64 _uid, QWidget *parent)
: uid(_uid), listWidget(new QListWidget(this)), QWidget(parent){
    QSqlQuery q;
    q.prepare("SELECT permission_id, permission_name FROM permissions ORDER BY permission_id ASC");
    q.exec();
    while(q.next()) {
        listWidget->addItem(q.value(1).toString());
        QListWidgetItem* cur = listWidget->item(listWidget->count() -1);
        cur->setData(ID_ROLE, q.value(0).toLongLong());
        cur->setFlags(cur->flags() | Qt::ItemIsUserCheckable);
        auto checked = UserPermissions::hasPermission(UserItem(uid), PermissionItem(q.value(0).toLongLong()));
        cur->setData(DB_STAT, checked);
        cur->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
        auto sz = cur->sizeHint();
        sz.setHeight(20);
        cur->setSizeHint(sz);
    }
    auto l = new QVBoxLayout(this);
    setLayout(l);
    l->addWidget(listWidget);
}

PermissionEditor::~PermissionEditor() {
    delete listWidget;
}

bool PermissionEditor::commit() {
    // get active perms
    QList<qint64> activePerms;
    for(int ix =0 ; ix < listWidget->count(); ++ix) {
        auto item = listWidget->item(ix);
        if(item->checkState() == Qt::Checked) {
            activePerms << item->data(ID_ROLE).toLongLong();
        }
    }
    // check if there is a role with identical active perms
    QStringList stl;
    for(auto ap = activePerms.begin(); ap != activePerms.end(); ++ap) {
        stl << QString::number(*ap);
    }
    QSqlQuery q;
    q.prepere(QString("SELECT role_id WHERE 
    ")
}