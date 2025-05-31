#ifndef MenuBarPlugin_H
#define MenuBarPlugin_H

#include "menubarmodifierinterface.h"

#define MenuBarModifierInterface_iid "LazyAdmin.Plugins.MenuBarModifier"

class QMainWindow;
class MenuBarPlugin : public QObject, public MenuBarModifierInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "LazyAdmin.Plugins.MenuBarModifier" FILE "menubarplugin.json");
    Q_INTERFACES(MenuBarModifierInterface);

public:
    MenuBarPlugin(QObject* =nullptr);
    ~MenuBarPlugin();
    void setupPlugin(QMainWindow*);
};

#endif