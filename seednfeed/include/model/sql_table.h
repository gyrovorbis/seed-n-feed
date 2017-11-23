#ifndef SQL_TABLE_H
#define SQL_TABLE_H

#include <QString>
#include <QSqlTableModel>

#define SQL_TABLE_MODEL_MAX_QUERY_SIZE  1024

class SqlTableModel: public QSqlTableModel {
protected:

    bool        _sqliteDropSqlColumn(QString colName);
    bool        _normalDropSqlColumn(QString colName);

    bool        _normalRenameSqlColumn(QString oldName, QString newName);
    bool        _sqliteRenameSqlColumn(QString oldName, QString newName=QString());

public:

                SqlTableModel(QObject* parent, QSqlDatabase database);

    bool        exportCSV(QString filePath) const;
    bool        importCSV(QString filePath);

    void        forceReloadModel(void);
    void        forceDBWrite(void);

   // bool        addSqlTableRow()
    bool        addSqlColumn(QString colName, QString dataType);
    bool        dropSqlColumn(QString colName);
    bool        changeSqlColumnDataType(QString colName, QString dataType);
    bool        renameSqlColumn(QString oldName, QString newName);
    QString     columnNameFromIndex(int index) const;
    int         columnIndexFromName(QString name) const;
    QStringList getColumnNames(void) const;

    static bool exportCSV(const QAbstractItemModel* model, QString filePath);
    static bool importCSV(QAbstractItemModel* model, QString filePath);

};


#endif // MODEL_UTILS_H
