#include "mainwindowcontext.h"

#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QDockWidget>
#include <QAction>
#include <QDebug>

MainWindowContext::MainWindowContext(QMainWindow* mainWindow, QMenuBar* menubar, QObject* parent)
    : QObject(parent), m_mainWindow(mainWindow), m_menubar(menubar){}

QDockWidget* MainWindowContext::addDock(QWidget* widget, const QString& title,
                                  Qt::DockWidgetArea area,
                                  const QString& tabGroup, bool raiseInGroup) {
    if (m_crafted) {
        qWarning() << "MainWindowContext::addDock dipanggil setelah craftAll() —"
                    << "dock" << title << "tidak akan ikut ter-tabify dengan benar.";
    }

    auto* dock = new QDockWidget(m_mainWindow);
    dock->setWidget(widget);
    dock->setWindowTitle(title);
    m_mainWindow->addDockWidget(area, dock);

    if (!tabGroup.isEmpty()) {
        m_dockGroups[tabGroup].append(dock);
        if (raiseInGroup) {
            m_raiseTarget[tabGroup] = dock;
        }
    }

    return dock;
}

void MainWindowContext::addDockToggleMenu(QDockWidget* dock, const QString& menuPath) {
    QMenu* menu = getOrCreateMenu(menuPath);
    menu->addAction(dock->toggleViewAction());
}

QString MainWindowContext::stripMnemonic(const QString& text) {
    QString out = text;
    out.remove('&');
    return out;
}

QMenu* MainWindowContext::getOrCreateMenu(const QString& menuPath) {
    const QStringList parts = menuPath.split('/', Qt::SkipEmptyParts);
    QMenu* current = nullptr;

    for (const QString& raw : parts) {
        const QString title = raw.trimmed();
        if (title.isEmpty()) continue;

        QMenu* found = nullptr;
        const auto actions = current ? current->actions() : m_menubar->actions();
        for (QAction* action : actions) {
            if (stripMnemonic(action->text()) == title && action->menu()) {
                found = action->menu();
                break;
            }
        }
        if (!found) {
            found = current ? current->addMenu(title) : m_menubar->addMenu(title);
        }
        current = found;
    }

    if (!current) {
        qWarning() << "MainWindowContext::getOrCreateMenu: menuPath kosong, menu tidak valid";
    }
    return current;
}

QAction* MainWindowContext::addMenuAction(const QString& menuPath, const QString& label,
                                    std::function<void()> callback) {
    auto* act = new QAction(label, m_menubar);
    return addMenuAction(menuPath, act, callback);
}

QAction* MainWindowContext::addMenuAction(const QString &menuPath, QAction *act, std::function<void()> callback)
{
    QMenu* menu = getOrCreateMenu(menuPath);
    if (!menu) return nullptr;
    if(callback) {
        QObject::connect(act, &QAction::triggered, this, [callback]{ callback(); });
    }
    menu->addAction(act);
    return act;
}

void MainWindowContext::onEvent(const QString& eventName, QObject* context,
                          std::function<void(QVariant)> callback) {
    Listener listener{QPointer<QObject>(context), std::move(callback)};
    m_listeners[eventName].append(listener);

    // Bersihkan otomatis begitu context dihancurkan — penting untuk dialog
    // nested/modal yang lifetime-nya pendek dan tidak selalu di-disconnect manual.
    if (context) {
        QObject::connect(context, &QObject::destroyed, this,
                          [this, eventName, context]() {
            auto& list = m_listeners[eventName];
            list.erase(std::remove_if(list.begin(), list.end(),
                       [context](const Listener& l) { return l.context == context; }),
                       list.end());
        });
    }
}

void MainWindowContext::emitEvent(const QString& eventName, const QVariant& data) {
    const auto listeners = m_listeners.value(eventName); // salin, callback bisa daftar listener baru
    for (const auto& listener : listeners) {
        if (listener.context.isNull() && listener.context) continue; // sudah destroyed, akan dibersihkan oleh signal destroyed
        listener.callback(data);
    }
    emit eventFired(eventName, data);
}

void MainWindowContext::registerInitialRefresh(std::function<void()> refreshFn) {
    m_initialRefreshFns.append(std::move(refreshFn));
}

void MainWindowContext::craftAll() {
    if (m_crafted) {
        qWarning() << "MainWindowContext::craftAll() dipanggil lebih dari sekali — diabaikan.";
        return;
    }
    m_crafted = true;

    // Tabify semua dock dalam grup yang sama, lalu raise() dock yang ditandai
    for (auto it = m_dockGroups.constBegin(); it != m_dockGroups.constEnd(); ++it) {
        const QString& group = it.key();
        const QList<QDockWidget*>& docks = it.value();
        if (docks.size() < 2) continue;

        for (int i = 1; i < docks.size(); ++i) {
            m_mainWindow->tabifyDockWidget(docks.first(), docks.at(i));
        }

        QDockWidget* toRaise = m_raiseTarget.value(group, docks.first());
        toRaise->raise();
    }

    // Jalankan initial refresh setelah semua viewer & listener terpasang
    for (const auto& fn : m_initialRefreshFns) {
        fn();
    }
}
