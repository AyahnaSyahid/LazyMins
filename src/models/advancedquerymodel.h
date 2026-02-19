#pragma once

#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QVariant>
#include <QMap>
#include <QSet>
#include <QList>
#include <QModelIndexList>
#include <QSortFilterProxyModel>

class AdvancedQueryModel : public QSqlQueryModel
{
    Q_OBJECT

public:
    // Custom roles for visual feedback
    enum CustomRoles {
        PendingInsertRole = Qt::UserRole + 100,
        PendingDeleteRole,
        PendingUpdateRole,
        RowStatusRole
    };
    
    // Row status enumeration
    enum RowStatus {
        NormalRow = 0,
        NewRow = 1,
        ModifiedRow = 2,
        DeletedRow = 3
    };
    
    // Sorting information
    struct SortInfo {
        int column;
        Qt::SortOrder order;
        bool enabled;
        
        SortInfo() : column(-1), order(Qt::AscendingOrder), enabled(false) {}
        SortInfo(int col, Qt::SortOrder ord) : column(col), order(ord), enabled(true) {}
    };

public:
    explicit AdvancedQueryModel(QObject *parent = nullptr);
    ~AdvancedQueryModel();

    // Query management
    void setQueryArgs(const QString &query, const QVariantMap &bindings = QVariantMap(), 
                     const QSqlDatabase &db = QSqlDatabase());
    
    // Paging
    void setPage(int page);
    void setPageSize(int size);
    int currentPage() const;
    int pageSize() const;
    int totalPages() const;
    int totalRecords() const;
    
    // Sorting
    void setSort(int column, Qt::SortOrder order);
    void clearSort();
    SortInfo currentSort() const;
    
    // Filtering
    void setFilter(const QString &filter);
    QString currentFilter() const;
    
    // Dissable column editing
    void setReadOnlyColumns(const QSet<int> &columns);  // set by index
    void setReadOnlyColumn(int column, bool readOnly = true);
    QSet<int> readOnlyColumns() const;
    
    // Transaction management
    bool submitAll();
    void revertAll();
    
    // Row operations
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    
    // Override core methods
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    
    // Status checking
    bool hasPendingChanges() const;
    bool isRowModified(int row) const;
    bool isNewRow(int row) const;
    bool isDeletedRow(int row) const;
    RowStatus rowStatus(int row) const;
    
    // Utility methods
    void refresh();
    QString lastError() const;

private:
    // Internal data structures
    struct PendingInsert {
        QMap<int, QVariant> data;
        int tempId; // Temporary ID for new rows
    };
    
    struct PendingUpdate {
        QMap<int, QVariant> changes;
    };
    
    // Query and database
    QString m_baseQuery;
    QVariantMap m_queryBindings;
    QSqlDatabase m_database;
    
    // Paging
    int m_currentPage;
    int m_pageSize;
    int m_totalRecords;
    
    // Sorting and filtering
    SortInfo m_sortInfo;
    QString m_filter;
    
    // Pending operations
    QList<PendingInsert> m_pendingInserts;
    QMap<int, PendingUpdate> m_pendingUpdates; // key: original row index
    QSet<int> m_pendingDeletes; // values: original row index
    
    // Restricted Columns
    QSet<int> m_readOnlyColumns;
    
    // Row mapping
    mutable QMap<int, int> m_newRowToPendingIndex; // view row -> pending insert index
    mutable bool m_rowMappingDirty;
    
    // Error handling
    QString m_lastError;
    
    // Private methods
    void rebuildQuery();
    QString buildFullQuery() const;
    void updateTotalRecords();
    int mapToOriginalRow(int viewRow) const;
    int mapFromOriginalRow(int originalRow) const;
    void cleanupEmptyNewRows();
    virtual bool executeDatabaseOperation(const QString &sql, const QVariantMap &bindings = QVariantMap());
    QVariant getOriginalValue(int originalRow, int column) const;
    bool isRowFilled(const PendingInsert &insert) const;
    
    // Database operations
    virtual bool insertToDatabase(const PendingInsert &insert);
    virtual bool updateToDatabase(int originalRow, const QMap<int, QVariant> &changes);
    virtual bool deleteFromDatabase(int originalRow);
};