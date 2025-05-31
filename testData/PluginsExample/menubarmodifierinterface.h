#ifndef MenuBarModifierInterface_H
#define MenuBarModifierInterface_H

#include <QtPlugin>

class QMainWindow;

class MenuBarModifierInterface {
public:
    virtual ~MenuBarModifierInterface() = default;
    virtual void setupPlugin(QMainWindow*) = 0;
};

#define MenuBarModifierInterface_iid "LazyAdmin.Plugins.MenuBarModifier"
Q_DECLARE_INTERFACE(MenuBarModifierInterface, MenuBarModifierInterface_iid);

#endif