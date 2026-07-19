#pragma once

#include <QObject>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QPointer>
#include <functional>

class QMainWindow;
class QMenuBar;
class QMenu;
class QDockWidget;
class QWidget;
class QAction;

// Satu-satunya pintu masuk yang dipakai setiap viewer/dialog untuk
// menyentuh MainWindow: dock, menu, dan komunikasi lintas-viewer.
// Viewer tidak pernah menyimpan pointer ke viewer lain secara langsung —
// semua koordinasi lewat onEvent/emitEvent di sini.
class MainWindowContext : public QObject
{
    Q_OBJECT
public:
    explicit MainWindowContext(QMainWindow *mainWindow, QMenuBar *menubar, QObject *parent = nullptr);

    // --- Dock / View ---
    // tabGroup: dock lain dengan tabGroup sama akan di-tabify jadi satu
    // grup tab saat craftAll() dipanggil (mis. semua dock "produk_finishing"
    // akan ditumpuk jadi satu grup). Kosongkan kalau dock ini berdiri sendiri.
    // raiseInGroup: dock ini yang jadi tab aktif pertama di grupnya.
    QDockWidget *addDock(QWidget *widget,
                         const QString &title,
                         Qt::DockWidgetArea area = Qt::TopDockWidgetArea,
                         const QString &tabGroup = QString(),
                         bool raiseInGroup = false);

    // Menambahkan toggleViewAction() milik dock ke menu tertentu.
    void addDockToggleMenu(QDockWidget *dock, const QString &menuPath);

    // --- Menu ---
    // menuPath cocok dengan menu YANG SUDAH ADA dari Designer (mis. "Tambah",
    // "View") berdasarkan teks (mengabaikan mnemonic '&'). Kalau menuPath berupa
    // path bersarang ("Tambah/Laporan") dan submenu belum ada, akan dibuat baru.
    QAction *addMenuAction(const QString &menuPath, const QString &label,
                           std::function<void()> callback);

    QAction *addMenuAction(const QString &menuPath, QAction *act,
                           std::function<void()> callback);

    QMenu *getOrCreateMenu(const QString &menuPath);
    // --- Event bus ---
    // context: object pemilik callback (biasanya `this` milik viewer/dialog).
    // Listener otomatis dilepas kalau context di-destroy — aman dipanggil
    // dari dialog nested/modal tanpa risiko use-after-free.
    void onEvent(const QString &eventName, QObject *context,
                 std::function<void(QVariant)> callback);
    void emitEvent(const QString &eventName, const QVariant &data = QVariant());

    // --- Registrasi refresh awal (opsional) ---
    // Kalau viewer ingin refresh() dijalankan setelah SEMUA viewer selesai
    // inisiasi (bukan langsung saat inisiasi()), daftarkan di sini.
    void registerInitialRefresh(std::function<void()> refreshFn);

    // --- Finalisasi ---
    // Dipanggil SEKALI di akhir MainWindow constructor, setelah semua
    // viewer->inisiasi(api) selesai. Menjalankan tabify/raise per grup dock
    // dan initial refresh yang didaftarkan lewat registerInitialRefresh().
    void craftAll();

signals:
    // Alternatif Qt-native untuk dengar event, kalau ada yang lebih suka
    // connect() biasa daripada onEvent() dengan std::function.
    void eventFired(const QString &name, QVariant data);

private:
    struct Listener
    {
        QPointer<QObject> context;
        std::function<void(QVariant)> callback;
    };

    static QString stripMnemonic(const QString &text);

    QMainWindow *m_mainWindow;
    QMenuBar *m_menubar;

    QMap<QString, QList<QDockWidget *>> m_dockGroups;
    QMap<QString, QDockWidget *> m_raiseTarget;
    QMap<QString, QList<Listener>> m_listeners;
    QList<std::function<void()>> m_initialRefreshFns;

    bool m_crafted = false;
};
