#ifndef PermissionEditorH
#define PermissionEditorH

#include <QWidget>
#include <QListWidget>

class PermissionEditor : public QWidget {
    Q_OBJECT

public:
    PermissionEditor(qint64, QWidget *=nullptr);
    ~PermissionEditor();

private:
    qint64 uid;
    QListWidget* listWidget;
};
#endif