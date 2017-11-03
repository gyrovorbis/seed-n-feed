#ifndef SQL_TABLE_H
#define SQL_TABLE_H

#include <QString>
#include <QSqlTableModel>


class SqlTableModel: public QSqlTableModel {

public:

    bool        exportCSV(QString filePath) const;
    bool        importCSV(QString filePath);

    static bool exportCSV(const QAbstractItemModel* model, QString filePath);
    static bool importCSV(QAbstractItemModel* model, QString filePath);

};


#endif // MODEL_UTILS_H
