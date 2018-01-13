#ifndef SQL_TABLE_H
#define SQL_TABLE_H

#include <QString>
#include <QStack>
#include <QSqlTableModel>
#include <QSqlTableModel>

#define SQL_TABLE_MODEL_MAX_QUERY_SIZE      1024*8

class SqlTableModel;

class SqlTableRowEntry {
public:
                SqlTableRowEntry(int row, SqlTableModel* model);
                SqlTableRowEntry(SqlTableRowEntry&& entry);

    QVariant    getColumnVariant(int col) const;
    QVariant    getColumnVariant(QString name) const;
    void        setColumnVariant(int col, QVariant variant);
    void        setColumnVariant(QString name, QVariant variant);

    template<typename T>
    T           getColumnValue(int col) const;
    template<typename T>
    T           getColumnValue(QString name) const;
    template<typename T>
    void        setColumnValue(int col, T&& value);
    template<typename T>
    void        setColumnValue(QString name, T&& value);

    int         getColumnCount(void) const;
    bool        isColumnValid(int col) const;
    bool        isColumnValid(QString name) const;
    auto        getColumnType(int col) const -> QVariant::Type;
    auto        getColumnType(QString name) const -> QVariant::Type;
    QString     getColumnTypeName(int col) const;
    QString     getColumnTypeName(QString name) const;
    int         getRow(void) const;

private:
    bool        _validateColumn(int col) const;
    int         _colNameToIndex(QString name) const;

    QVector<QVariant>        _colData;
    int                      _row       = -1;
    SqlTableModel*           _model     = nullptr;
};


class SqlTableModel: public QSqlTableModel {
    Q_OBJECT
protected:

    struct ColumnRelationEntry {
        int localCol;
        int srcCol;
        SqlTableModel* srcModel;
        bool oneToOne;
        int  role;
    };

    struct ColumnEntry {
        QString     name;
        QString     type;
        QVariant    defaultVal;
        QString     other;

        QString     toSqlString(void) const;
    };

    struct DynamicColumnFromRowsEntry {
        int             srcCol;
        SqlTableModel*  srcModel;
        ColumnEntry     colEntry;
        int             role;
    };

    bool            _sqliteDropSqlColumn(QString colName);
    bool            _normalDropSqlColumn(QString colName);

    bool            _normalRenameSqlColumn(QString oldName, QString newName);
    bool            _sqliteRenameSqlColumn(QString oldName, QString newName=QString());

    void            _addColumnEntry(ColumnEntry entry);
    void            _removeColumnEntry(QString name);
    void            _setColumnEntry(int index, ColumnEntry entry);
    ColumnEntry     _getColumnEntry(int index) const;
    ColumnEntry     _findColumnEntry(QString name) const;

    QStack<QString>                 _filterStack;
    QVector<ColumnEntry>            _colEntries;
    QHash<QString, int>             _colEntryNameToIndexHash;
    QVector<ColumnRelationEntry>    _colRelationEntries;
    DynamicColumnFromRowsEntry      _dynColEntry;

public:

                    SqlTableModel(QObject* parent, QSqlDatabase database);

    bool            exportCSV(QString filePath) const;
    bool            importCSV(QString filePath);


    void            forceReloadModel(void);
    void            forceDBWrite(void);
    bool            protectedDbCommit(void);
    bool            protectedSelect(void);

    void            addHeaderData(int col, Qt::Orientation orient, QString name, QString tooltip=QString());
    bool            appendNewRow(QVector<QPair<int, QVariant>> defaultVals=QVector<QPair<int, QVariant>>());
    bool            deleteRows(QModelIndexList& list);
    QVariant        dummyVariantForColumn(int col) const;

   // bool          addSqlTableRow()
    bool            addSqlColumn(QString colName, QString dataType, QVariant defaultVal=QVariant(), QString other=QString());
    bool            dropSqlColumn(QString colName);
    bool            changeSqlColumnDataType(QString colName, QString dataType);
    bool            renameSqlColumn(QString oldName, QString newName);
    QString         columnNameFromIndex(int index) const;
    int             columnIndexFromName(QString name) const;
    QStringList     getColumnNames(void) const;

    QModelIndexList findRowsWithColumnValue(int col, QVariant variant);

    unsigned        updateColumnValues(int col, QVariant oldVal, QVariant newVal, int role = Qt::EditRole);

    static bool     exportCSV(const QAbstractItemModel* model, QString filePath);
    static bool     importCSV(QAbstractItemModel* model, QString filePath);

    virtual bool    setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    virtual int     rowCount(const QModelIndex& parent=QModelIndex()) const override;
    virtual int     columnCount(const QModelIndex& parent=QModelIndex()) const override;

    bool            pushFilter(QString string=QString());
    bool            popFilter(void);

    QString         makeCreateTableQueryString(QString name=QString()) const;
    QString         makeColumnDataUnique(int col, QString prefix) const;
    int             rowIndexWithColumnValue(int col, QVariant value) const;
    void            addColumnData(QString name, QString type, QVariant defaultVal=QVariant(), QString other=QString());
    bool            removeColumnData(QString name);
    void            addColumnRelationship(int localCol, int srcCol, SqlTableModel* model, bool oneToOne=false, int role=Qt::EditRole);
    void            addColumnsFromRowsRelationship(int srcCol, SqlTableModel* model, QString type, QVariant defValue=QVariant(), int role=Qt::EditRole);
    bool            isOpen(void) const;


    QVariant           getVariant(int row, int col, int role=Qt::EditRole) const;
    void               setVariant(int row, int col, QVariant variant, int role=Qt::EditRole);
    SqlTableRowEntry   getTableRowEntry(int row);
    SqlTableRowEntry   createTableRowEntry(void);
    bool               setTableRowEntry(int row, SqlTableRowEntry entry);

    template<typename F>
    void               iterateForColumnVariants(int col, F&& func, int role=Qt::EditRole) const;

    void               populateDynamicColumnEntries(void);


signals:

    void            cellDataChanged(int row, int col, QVariant oldValue, QVariant newValue, int role);

public slots:
    void            prefetchTableData(void);
};





//======== INLINEZ ========
inline QString SqlTableModel::ColumnEntry::toSqlString(void) const {
    return name + QString(" ") + type + QString(" ") + other;
}

inline bool SqlTableModel::isOpen(void) const {
    return (!tableName().isNull() && !tableName().isEmpty());

}

template<typename F//, typename = typename std::enable_if< std::is_same<, decltype(void(int, QVariant)) >::value>::type>
         >
inline void SqlTableModel::iterateForColumnVariants(int col, F&& func, int role) const {
    for(int r = 0; r < rowCount(); ++r) {
        auto idx = index(r, col);
        Q_ASSERT(idx.isValid());
        if(idx.isValid()) {
            func(r, idx.data(role));
        }
    }
}


inline void SqlTableModel::_addColumnEntry(ColumnEntry entry) {
    _colEntryNameToIndexHash[entry.name] = _colEntries.size();
    _colEntries.push_back(std::move(entry));
}

inline void SqlTableModel::_removeColumnEntry(QString name) {
    int index = columnIndexFromName(name);
    Q_ASSERT(index >= 0 && index < _colEntries.size());
    _colEntries.remove(index);
    _colEntryNameToIndexHash.clear();
    for(int c = 0; c < _colEntries.size(); ++c) {
        _colEntryNameToIndexHash[_colEntries[c].name] = c;
    }
}

inline void SqlTableModel::_setColumnEntry(int index, ColumnEntry entry) {
    Q_ASSERT(index >= 0 && index < columnCount());
    auto& entryRef = _colEntries[index];
    _colEntryNameToIndexHash.remove(entryRef.name);
    _colEntryNameToIndexHash[entry.name] = index;
    entryRef = std::move(entry);
}
inline auto SqlTableModel::_getColumnEntry(int index) const -> ColumnEntry {
    Q_ASSERT(index >= 0 && index < columnCount());
    return _colEntries[index];
}

inline auto SqlTableModel::_findColumnEntry(QString name) const -> ColumnEntry {
    const int idx = columnIndexFromName(name);
    Q_ASSERT(idx >= 0 && idx < columnCount());
    return _colEntries[idx];
}

inline bool SqlTableRowEntry::_validateColumn(int col) const {
    const bool valid = (col >= 0 && col < _model->columnCount());
    Q_ASSERT(valid);
    return valid;
}

inline int SqlTableRowEntry::_colNameToIndex(QString name) const {
    Q_ASSERT(!name.isNull());
    return _model->columnIndexFromName(std::move(name));
}


inline QVariant SqlTableRowEntry::getColumnVariant(int col) const {
    Q_ASSERT(_model);
    return (_validateColumn(col))? _colData[col] : QVariant();
}

inline void SqlTableRowEntry::setColumnVariant(int col, QVariant variant) {
    Q_ASSERT(_model);
    if(_validateColumn(col)) _colData[col] = std::move(variant);
}

template<typename T>
inline T SqlTableRowEntry::getColumnValue(int col) const {
    Q_ASSERT(_model);
    _validateColumn(col);
    return _colData[col].value<T>();
}

template<typename T>
inline void SqlTableRowEntry::setColumnValue(int col, T&& value) {
    Q_ASSERT(_model);
    if(_validateColumn(col)) _colData[col] = std::forward<T>(value);
}

inline int SqlTableRowEntry::getColumnCount(void) const {
    return _colData.size();
}

inline bool SqlTableRowEntry::isColumnValid(int col) const {
    Q_ASSERT(_model);
    return (_validateColumn(col))? _colData[col].isValid() : false;
}

inline auto SqlTableRowEntry::getColumnType(int col) const -> QVariant::Type {
    Q_ASSERT(_model);
    return (_validateColumn(col))? _colData[col].type() : QVariant::Invalid;
}

inline QString SqlTableRowEntry::getColumnTypeName(int col) const {
    Q_ASSERT(_model);
    return (_validateColumn(col))? _colData[col].typeName() : QString("Invalid Column Index: ") + QString::number(col);
}

inline int SqlTableRowEntry::getRow(void) const {
    return _row;
}

inline QVariant             SqlTableRowEntry::getColumnVariant(QString name) const { return getColumnVariant(_colNameToIndex(name)); }
inline void                 SqlTableRowEntry::setColumnVariant(QString name, QVariant variant) { setColumnVariant(_colNameToIndex(name), std::move(variant)); }
template <typename T> T     SqlTableRowEntry::getColumnValue(QString name) const { return getColumnValue<T>(_colNameToIndex(name)); }
template <typename T> void  SqlTableRowEntry::setColumnValue(QString name, T&& value) { setColumnValue<T>(_colNameToIndex(name), std::forward<T>(value)); }
inline bool                 SqlTableRowEntry::isColumnValid(QString name) const { return isColumnValid(_colNameToIndex(name)); }
inline auto                 SqlTableRowEntry::getColumnType(QString name) const -> QVariant::Type { return getColumnType(_colNameToIndex(name)); }
inline QString              SqlTableRowEntry::getColumnTypeName(QString name) const { return getColumnTypeName(_colNameToIndex(name)); }


#endif // MODEL_UTILS_H
