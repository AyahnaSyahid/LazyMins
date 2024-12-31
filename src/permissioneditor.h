#ifndef PermissionEditorH
#define PermissionEditorH

#include <QWidget>
#include <QListWidget>

class QListWidgetItem;
class PermissionEditor : public QWidget {
    Q_OBJECT

public:
    PermissionEditor(qint64, QWidget* =nullptr);
    ~PermissionEditor();

public slots:
    void onItemChanged(QListWidgetItem *);
private:
    qint64 uid;
    QListWidget* listWidget;
};
#endif