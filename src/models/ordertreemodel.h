#pragma once

#include <QAbstractItemModel>
#include <QSqlDatabase>
#include <QDateTime>
#include <QVariant>
#include <QVector>
#include <QString>
#include <memory>

// ─────────────────────────────────────────────
//  Column indices (shared / homogeneous)
// ─────────────────────────────────────────────
namespace OrderTreeCol {
    enum Column {
        Date        = 0,   // Tanggal (Date level only)
        CustomerName= 1,   // Nama Customer (Customer level only)
        OrderNumber = 2,   // No. Order (Order level only)
        Name        = 3,   // – | – | – | Nama Item | Nama Finishing
        Qty         = 4,   // – | – | – | qty unit | qty
        Size        = 5,   // – | – | – | W×H | –
        UnitPrice   = 6,   // – | – | – | sale_price | finishing_price
        Subtotal    = 7,   // – | – | – | subtotal | subtotal
        Discount    = 8,   // – | – | discount_amount | discount_amount | –
        Total       = 9,   // – | – | total_amount | total | –
        Notes       = 10,  // – | – | notes | notes | –
        COUNT       = 11
    };
}

// ─────────────────────────────────────────────
//  Node level tags
// ─────────────────────────────────────────────
enum class NodeLevel {
    Root,
    Date,
    Customer,
    Order,
    Item,
    Finishing
};

// ─────────────────────────────────────────────
//  TreeNode  — forward-declared, defined in .cpp
// ─────────────────────────────────────────────
struct TreeNode {
    NodeLevel           level       = NodeLevel::Root;
    int                 id          = -1;       // primary key for this level
    int                 parentId    = -1;       // parent's PK (for lazy-fetch queries)
    QVector<QVariant>   columns;                // size == OrderTreeCol::COUNT
    bool                childrenFetched = false;
    bool                hasChildrenHint = true;  // early-fetch hint: does this node
                                                  // actually have children? Determined
                                                  // via EXISTS(...) at fetch time so the
                                                  // expand/collapse arrow is only shown
                                                  // when a node truly has children.

    TreeNode*           parent      = nullptr;
    QVector<TreeNode*>  children;

    explicit TreeNode(NodeLevel lvl, int id = -1, int parentId = -1,
                      TreeNode* parent = nullptr);
    ~TreeNode();                               // recursively deletes children

    // Disable copy
    TreeNode(const TreeNode&)            = delete;
    TreeNode& operator=(const TreeNode&) = delete;
};

// ─────────────────────────────────────────────
//  OrderTreeModel
// ─────────────────────────────────────────────
class OrderTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit OrderTreeModel(QSqlDatabase db, QObject* parent = nullptr);
    ~OrderTreeModel() override;

    // ── QAbstractItemModel interface ──────────
    QModelIndex index(int row, int column,
                      const QModelIndex& parent = {}) const override;

    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount   (const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;

    QVariant data(const QModelIndex& index,
                  int role = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    bool hasChildren(const QModelIndex& parent = {}) const override;

    // canFetchMore / fetchMore  → lazy-load children
    bool canFetchMore(const QModelIndex& parent) const override;
    void fetchMore   (const QModelIndex& parent) override;

    // ── Public helpers ────────────────────────
    /** Full reload — drops all cached data and re-fetches root level. */
    void reload();

    /** Returns the NodeLevel of the node at @p index. */
    NodeLevel nodeLevel(const QModelIndex& index) const;

    /** Returns the database id stored in the node at @p index. */
    int nodeId(const QModelIndex& index) const;

signals:
    void fetchError(const QString& message);

private:
    // ── Fetch helpers (each populates parent->children) ──
    void fetchDateNodes    (TreeNode* parent);
    void fetchCustomerNodes(TreeNode* parent, const QDate& date);
    void fetchOrderNodes   (TreeNode* parent, int customerId, const QDate& date);
    void fetchItemNodes    (TreeNode* parent, int orderId);
    void fetchFinishingNodes(TreeNode* parent, int orderItemId);

    // ── Utility ──────────────────────────────
    static QVariant formatCurrency(int value);
    static QVariant formatSize    (double w, double h, int useArea);

    /** Convert UTC QDateTime stored in DB → local QDateTime. */
    static QDateTime toLocal(const QVariant& dbValue);

    TreeNode*        nodeFromIndex(const QModelIndex& index) const;

    // ── Data members ─────────────────────────
    QSqlDatabase  m_db;
    TreeNode*     m_root;    // invisible root (level == Root)
};
