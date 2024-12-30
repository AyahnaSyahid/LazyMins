#ifndef PermissionEditorH
#define PermissionEditorH

#include <QWidget>
#include <QListWidget>

class PermissionEditor : public QWidget {
    Q_OBJECT

public:
    PermissionEditor(qint64, QWidget* =nullptr);
    ~PermissionEditor();

public slots:
    bool commit();
    
private:
    qint64 uid;
    QListWidget* listWidget;
};
#endif