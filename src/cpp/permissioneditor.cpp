#include "permissioneditor.h"
#include <QListWidgetItem>
#include <QSqlQuery>

PermissionEditor::PermissionEditor(qint64 _uid, QWidget *parent)
: uid(_uid), listWidget(new QListWidgetItem(this)), QWidget(parent){
    QSqlQuery q;
    q.prepare("SELECT permission_id, ")
}