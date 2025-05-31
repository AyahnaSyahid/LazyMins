#ifndef LazyMinsPluginInterface_H
#define LazyMinsPluginInterface_H

#include <QtPlugin>

class QMainWindow;
class Database;
class LazyMinsPluginInterface {
    public:
        virtual ~LazyMinsPluginInterface() = default;
        virtual void configureUi(QMainWindow*) = 0;
        virtual void configureDatabase(Database*) = 0;
};

#define LazyMinsPluginInterface_iid "LazyMins.ExtenderPlugins"
Q_DECLARE_INTERFACE(LazyMinsPluginInterface, LazyMinsPluginInterface_iid)

#endif