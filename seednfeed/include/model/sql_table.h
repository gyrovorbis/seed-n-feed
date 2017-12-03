#ifndef SQL_TABLE_H
#define SQL_TABLE_H

#include <QString>
#include <QStack>
#include <QSqlTableModel>

#define SQL_TABLE_MODEL_MAX_QUERY_SIZE  1024

class SqlTableModel: public QSqlTableModel {
    Q_OBJECT
protected:

    bool            _sqliteDropSqlColumn(QString colName);
    bool            _normalDropSqlColumn(QString colName);

    bool            _normalRenameSqlColumn(QString oldName, QString newName);
    bool            _sqliteRenameSqlColumn(QString oldName, QString newName=QString());

    QStack<QString> _filterStack;

public:

                    SqlTableModel(QObject* parent, QSqlDatabase database);

    bool            exportCSV(QString filePath) const;
    bool            importCSV(QString filePath);

    void            forceReloadModel(void);
    void            forceDBWrite(void);

   // bool          addSqlTableRow()
    bool            addSqlColumn(QString colName, QString dataType);
    bool            dropSqlColumn(QString colName);
    bool            changeSqlColumnDataType(QString colName, QString dataType);
    bool            renameSqlColumn(QString oldName, QString newName);
    QString         columnNameFromIndex(int index) const;
    int             columnIndexFromName(QString name) const;
    QStringList     getColumnNames(void) const;

    unsigned        updateColumnValues(int col, QVariant oldVal, QVariant newVal, int role = Qt::EditRole);

    static bool     exportCSV(const QAbstractItemModel* model, QString filePath);
    static bool     importCSV(QAbstractItemModel* model, QString filePath);

    virtual bool    setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    bool            pushFilter(QString string=QString());
    bool            popFilter(void);

signals:

    void            cellDataChanged(int row, int col, QVariant oldValue, QVariant newValue, int role);

};


#endif // MODEL_UTILS_H
