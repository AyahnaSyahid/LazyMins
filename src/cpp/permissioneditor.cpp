#include "permissioneditor.h"
#include "userpermissions.h"
#include "helper.h"
#include <QListWidget>
#include <QVBoxLayout>
#include <QListWidgetItem>
#include <QSqlQuery>
#include <QtDebug>


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
        cur->setData(ID_ROLE, q.value(0));
        cur->setFlags(cur->flags() | Qt::ItemIsUserCheckable);
        if(q.value(1).toString() == "GRANT_ALL")
            cur->setFlags(cur->flags()^Qt::ItemIsUserCheckable);
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
    connect(listWidget, &QListWidget::itemChanged, this, &PermissionEditor::onItemChanged);
}

PermissionEditor::~PermissionEditor() {
    delete listWidget;
}

void PermissionEditor::onItemChanged(QListWidgetItem* wi) {
    bool ok = UserPermissions::setPermission(UserItem(uid), PermissionItem(wi->data(ID_ROLE).toLongLong()), wi->checkState());
    if(!ok) {
        MessageHelper::information(this, "Gagal", "Perubahan izin tidak dizinkan");
    }
}