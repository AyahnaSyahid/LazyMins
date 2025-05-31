#include "menubarplugin.h"
#include <QMainWindow>
#include <QMenuBar>

MenuBarPlugin::MenuBarPlugin(QObject* parent) :
    QObject(parent) {}
    
MenuBarPlugin::~MenuBarPlugin(){};

void MenuBarPlugin::setupPlugin(QMainWindow* mw) {
    QMenuBar *m = mw->menuBar();
    QMenu *viewMenu = m->findChild<QMenu*>("viewMenu");
    if(!viewMenu) {
        viewMenu = m->addMenu("View");
    }
    viewMenu->addAction("MenuBarPlugins");
}