#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QList>
#include <QMap>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QString>

#include "src/managers/basemanager.h"

// ─────────────────────────────────────────────────────────────────────────────
// OrderHeader – mirrors the orders table (fields managed by the application)
// ─────────────────────────────────────────────────────────────────────────────

struct OrderHeader
{
    // Required on INSERT
    int     admin_id       = -1;
    QString customer_name;            // denormalised, always required

    // Optional / nullable
    int     customer_id    = -1;      // -1 → NULL (walk-in customer)
    QString customer_phone;
    int     price_level_id = 1;

    // Financial (order-level, not item-level)
    int     discount_amount     = 0;
    int     discount_percentage = 0;
    int     tax_amount          = 0;

    // Status
    QString status          = "pending";   // pending|processing|ready|completed|cancelled
    QString priority        = "normal";    // urgent|high|normal|low
    QString payment_status  = "unpaid";    // unpaid|partial|paid

    // Dates
    QDateTime order_date;                  // default: now (left empty → DB default)
    QDateTime deadline_date;               // nullable
    QDateTime completion_date;             // nullable

    // Notes
    QString notes;
    QString internal_notes;

    // Loaded from DB (read-only for callers, written by loadOrder / commit)
    int       id           = -1;
    QString   order_number;

    // Validation helper
    bool isValid() const { return admin_id != -1 && !customer_name.trimmed().isEmpty(); }
};

// ─────────────────────────────────────────────────────────────────────────────

struct FinishingItem
{
    int     id             = -1,
            order_item_id  = -1,
            finishing_id   = -1;
    QString finishing_name;
    int     quantity        = 1,
            finishing_price = 0;

    int subtotal() const;

    FinishingItem() = default;
    static FinishingItem loadFromId(int id);
};

// ─────────────────────────────────────────────────────────────────────────────

struct OrderItem
{
    int     id         = -1,
            order_id   = -1,
            product_id = -1;
    QString product_name,
            sku;
    int     quantity    = 1;
    QString unit        = "pcs";
    qreal   size_width  = 1.0,
            size_height = 1.0;
    bool    use_area    = false;
    int     sale_price  = 0,
            base_price  = 0;
    qreal   discount_percentage = 0.0;
    int     discount_amount     = 0;
    int     finishing_total     = 0;
    
    QString notes;
    
    const QString descriptionText() const;

    int subtotal() const;   // mirrors DB GENERATED column
    int total()    const;   // mirrors DB GENERATED column

    OrderItem() { finishings.reserve(5); };
    OrderItem &loadFromId(int id);

    // Called only by OrderModel::commit(). Not called during normal editing.
    bool save(QSqlDatabase &db);
    
    QList<FinishingItem> finishings;
};

class OrderModel : public QAbstractListModel
{
    Q_OBJECT
public:

    explicit OrderModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}
    ~OrderModel() override;

    // ── QAbstractListModel overrides ──────────────────────────────────────────
    int      rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data    (const QModelIndex &ix, int role = Qt::DisplayRole) const override;
    // Mutates the in-memory cache only. No DB I/O until commit().
    bool     setData (const QModelIndex &ix, const QVariant &va, int role = Qt::EditRole) override;

    // ── Order header ──────────────────────────────────────────────────────────
    // Populate the header fields for a brand-new order (orderId not yet known).
    // Must be called before addItem() on a new order.
    // Marks the header as dirty so commit() will INSERT the orders row.
    void setHeader(const OrderHeader &header);

    // Update a specific field on the header of an already-loaded order.
    // Marks the header dirty so commit() will UPDATE the orders row.
    void setHeaderField(const OrderHeader &header);

    // Read-only access to the current header state.
    const OrderHeader &header() const { return m_header; }

    // ── Bulk load ─────────────────────────────────────────────────────────────
    // Load an existing order (header + all items) from the DB.
    // Discards any uncommitted changes first.
    bool loadOrder(int orderId, QSqlDatabase &db);

    // ── Item mutation  (in-memory only, no DB I/O) ────────────────────────────
    // Append a new item. Returns the index of the new row.
    // id will be -1 until commit() assigns the real DB id.
    QModelIndex addItem(OrderItem item);

    // Mark the item at `row` for deletion. It is removed from the view
    // immediately; the DELETE runs on commit().
    bool removeItem(int row);

    // ── Finishing mutation  (in-memory only, no DB I/O) ───────────────────────
    bool addFinishing   (int row, FinishingItem finishing);
    bool updateFinishing(int row, int finishingIndex, const FinishingItem &finishing);
    bool removeFinishing(int row, int finishingIndex);

    // ── Unit-of-work ──────────────────────────────────────────────────────────
    // Flush every pending change to the DB inside ONE transaction:
    //   • If m_orderId == -1: INSERT the orders row first, then all items.
    //   • If m_orderId != -1: UPDATE the orders row, then items.
    //   • All deletes, inserts, and updates for items and finishings follow.
    // On success, dirty state is cleared and all temporary ids become real ids.
    // On any failure the whole transaction is rolled back.
    bool commit(QSqlDatabase &db = BaseManager::connection);

    // Discard all pending changes:
    //   • If the order was never saved (m_orderId == -1): resets to blank state.
    //   • Otherwise: reloads from the DB.
    bool revert(QSqlDatabase &db);

    // True if header or any item has uncommitted changes.
    bool isDirty() const;

    // ── Accessors ─────────────────────────────────────────────────────────────
    const OrderItem &itemAt(int row) const { return m_items.at(row); }
    int  orderId() const { return m_orderId; }
    OrderItem &itemRef(int ix) { return m_items[ix]; }
    const QList<OrderItem>& items() const { return m_items; }

signals:
    // Emitted after every in-memory mutation so a UI total label can update
    // immediately without waiting for commit().
    void orderTotalChanged(int newTotal);

private:
    void recalcFinishingTotal(int row);  // pure in-memory
    void recalcOrderTotal();             // pure in-memory, emits orderTotalChanged

    // Dirty tracking ──────────────────────────────────────────────────────────
    bool               m_headerDirty = false;
    QSet<int>          m_dirtyRows;
    QList<int>         m_deletedItemIds;
    QMap<int,QList<int>> m_deletedFinishingIds;

    OrderHeader      m_header;
    QList<OrderItem> m_items;
    int              m_orderId = -1;   // -1 = order not yet in DB
    QSqlRecord       empty;
};
