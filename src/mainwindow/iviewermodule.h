#pragma once

class PluginAPI;

// Opsional — implementasikan ini di viewer/dialog kalau kamu ingin
// konsistensi bentuk (misal nanti mau iterasi semua modul otomatis).
// Tidak wajib: PluginAPI tidak butuh viewer meng-implementasi apapun,
// cukup viewer punya method apapun bentuknya yang menerima PluginAPI*
// dan memanggil addDock/addMenuAction/onEvent di dalamnya.
class IViewerModule {
public:
    virtual ~IViewerModule() = default;

    // Daftarkan dock, menu, dan event listener milik viewer ini.
    // Dipanggil sekali per viewer, sebelum PluginAPI::craftAll().
    virtual void inisiasi(PluginAPI* api) = 0;
};
