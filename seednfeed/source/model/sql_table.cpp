#include <QDebug>
#include <QFile>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDriver>
#include "model/sql_table.h"
#include "core/utilities.h"

SqlTableModel::SqlTableModel(QObject *parent, QSqlDatabase database):
    QSqlTableModel(parent, database) {}

//Stolen from: https://evileg.com/en/post/190/
//thanks, nigga
bool SqlTableModel::exportCSV(const QAbstractItemModel* model, QString filePath) {
    /* Create a CSV file of the object and specify the path to the file
      * Be sure to include a valid path and extension .csv
      * */
     QFile csvFile(filePath);

     // Open or create the file if it does not exist
     if(csvFile.open( QIODevice::WriteOnly ))
     {
         // Create a text stream, which will write the data
         QTextStream textStream( &csvFile );
         QStringList stringList; // The helper object QSqtringList, which will form a line

         for(int row = 0; row < model->rowCount(); row++ )
         {
             stringList.clear();
             for( int column = 0; column < model->columnCount(); column++ )
             {
                 // Write in stringList each table entry
                 stringList << model->data(model->index(row, column)).toString();
             }
             /* Then send the entire file stringList by adding text flow dividers
              * in the form of "" and putting at the end of a line terminator
              * */
             textStream << stringList.join( ',' )+"\n";
         }
         // Close the file - ready
         csvFile.close();
         return true;
     } else {

         qCritical() << "File could not be opened for writing: " << filePath;
         qCritical() << csvFile.errorString();
         return false;
     }
}


bool SqlTableModel::importCSV(QAbstractItemModel* model, QString filePath) {
     QFile csvFile(filePath);

     // Open or create the file if it does not exist
     if(csvFile.open( QIODevice::ReadOnly ))
     {
        model->removeRows(0, model->rowCount());

        //basically blow everything the fuck away first!!!!

        while(!csvFile.atEnd()) {
            QByteArray line = csvFile.readLine();
            QList<QByteArray> stringList = line.split(',');
            model->insertRows(model->rowCount(), 1);
            int row = model->rowCount() - 1;
            for(int i = 0; i < stringList.size(); ++i) {
                model->setData(model->index(row, i), stringList[i]);
            }
        }

         csvFile.close();
         return true;
     } else {

         qCritical() << "File could not be opened for reading: " << filePath;
         qCritical() << csvFile.errorString();
         return false;
     }
}

bool SqlTableModel::exportCSV(QString filePath) const {
    return exportCSV(this, filePath);
}

bool SqlTableModel::importCSV(QString filePath) {
    return importCSV(this, filePath);
}

void SqlTableModel::forceReloadModel(void) {
    //QString name = tableName();
    //setTable("");
    //clear();
    setTable(tableName());
    select();
}

bool SqlTableModel::addSqlColumn(QString colName, QString dataType) {
    QSqlQuery query;
    char sqlCmdBuff[SQL_TABLE_MODEL_MAX_QUERY_SIZE];
    sprintf(sqlCmdBuff,
            "ALTER TABLE %s\n\tADD COLUMN %s %s;", Q_CSTR(tableName()), Q_CSTR(colName), Q_CSTR(dataType));

    if(!query.exec(sqlCmdBuff)) {
        qCritical() << "Unable to add column" << colName << "to the" << tableName() << "database table: " << query.lastError();
        return false;
    } else {
         qDebug() << "Successfully added column" << colName << "to the" << tableName() << "database table!";
        if(submitAll()) {
            database().commit();
        }
        forceReloadModel();
        return true;
    }

}

bool SqlTableModel::_sqliteDropSqlColumn(QString colName) {
   return SqlTableModel::_sqliteRenameSqlColumn(colName);
}

bool SqlTableModel::_normalDropSqlColumn(QString colName) {
    QSqlQuery query;
    char sqlCmdBuff[SQL_TABLE_MODEL_MAX_QUERY_SIZE];

    sprintf(sqlCmdBuff,
            "ALTER TABLE %s\n\tDROP %s;", Q_CSTR(tableName()), Q_CSTR(colName));

    if(!query.exec(sqlCmdBuff)) {
        qCritical() << "Unable to drop column" << colName << "from the" << tableName() << "database table: " << query.lastError();
        return false;
    } else {
         qDebug() << "Successfully dropped column" << colName << "from the" << tableName() << "database table!";
        if(submitAll()) {
            database().commit();
        }
        forceReloadModel();
        return true;
    }
}

bool SqlTableModel::dropSqlColumn(QString colName) {
    if(database().driverName() == "QSQLITE") return _sqliteDropSqlColumn(colName);
    else return _normalDropSqlColumn(colName);
}

bool SqlTableModel::changeSqlColumnDataType(QString colName, QString dataType) {
    QSqlQuery query;
    char sqlCmdBuff[SQL_TABLE_MODEL_MAX_QUERY_SIZE];
    sprintf(sqlCmdBuff,
            "ALTER TABLE %s"
            "ALTER COLUMN %s %s;", Q_CSTR(tableName()), Q_CSTR(colName), Q_CSTR(dataType));

    if(!query.exec(sqlCmdBuff)) {
        qCritical() << "Unable to alter column" << colName << "in the" << tableName() << "database table: " << query.lastError();
        return false;
    } else {
         qDebug() << "Successfully altered column" << colName << "in the" << tableName() << "database table!";
        if(submitAll()) {
            database().commit();
        }
        forceReloadModel();
        return true;
    }
}

bool SqlTableModel::_normalRenameSqlColumn(QString oldName, QString newName) {
    QSqlQuery query;
    char sqlCmdBuff[SQL_TABLE_MODEL_MAX_QUERY_SIZE];
    sprintf(sqlCmdBuff,
            "ALTER TABLE %s"
            "RENAME COLUMN %s %s;", Q_CSTR(tableName()), Q_CSTR(oldName), Q_CSTR(newName));

    if(!query.exec(sqlCmdBuff)) {
        qCritical() << "Unable to rename column" << oldName << "in the" << tableName() << "database table to" << newName << ": " << query.lastError();
        return false;
    } else {
         qDebug() << "Successfully renamed column" << oldName << "in the" << tableName() << "database table to" << newName << "!";
        if(submitAll()) {
            database().commit();
        }
        forceReloadModel();
        return true;
    }
}

bool SqlTableModel::_sqliteRenameSqlColumn(QString oldName, QString newName) {
    QSqlQuery query;
    char sqlCmdBuff[SQL_TABLE_MODEL_MAX_QUERY_SIZE];

    const bool remove = (newName.isNull() || newName.isEmpty());
    const int column = columnIndexFromName(oldName);
    if(column == -1) {
         if(remove) qCritical() << "Unable to remove column" << oldName << "from the" << tableName() << "database table due to invalid index:" << column;
         else       qCritical() << "Unable to rename column" << oldName << "to" << newName << "from the" << tableName() << "database table due to invalid index:" << column;
         return false;
    }

    QStringList srcColumnNames = getColumnNames();
    for(auto& str: srcColumnNames) str = QString("\"") + str + QString("\"");
    newName = QString("\"") + newName + QString("\"");
    if(remove) srcColumnNames.removeAt(column);
    QStringList dstColumnNames = srcColumnNames;
    if(!remove) dstColumnNames.replace(column, newName);
    QString commaSeparatedSrcColNames = srcColumnNames.join(',');
    QString commaSeparatedDstColNames = dstColumnNames.join(',');

    sprintf(sqlCmdBuff,
            "CREATE TEMPORARY TABLE t1_backup(%s);\n"
            "INSERT INTO t1_backup SELECT %s FROM %s;\n"
            "DROP TABLE %s;\n"
            "CREATE TABLE %s(%s);\n"
            "INSERT INTO %s SELECT %s FROM t1_backup;\n"
            "DROP TABLE t1_backup;",
            Q_CSTR(commaSeparatedSrcColNames), Q_CSTR(commaSeparatedSrcColNames), Q_CSTR(tableName()), Q_CSTR(tableName()),
            Q_CSTR(tableName()), Q_CSTR(commaSeparatedDstColNames), Q_CSTR(tableName()), Q_CSTR(commaSeparatedSrcColNames));

    QStringList queryTextList = QString(sqlCmdBuff).split("\n");

    Q_ASSERT(database().driver()->hasFeature(QSqlDriver::Transactions));

    bool queryFailed = false;
    database().transaction();
    for(int i = 0; i < queryTextList.size(); ++i) {
        if(!query.exec(queryTextList[i])) {
            qCritical() << "Query failed within transaction: " << query.lastError();
            queryFailed = true;
            break;
        }
    }

    if(!queryFailed) {
        //if(submitAll()) {
        submitAll();
            database().commit();
        //}
        forceReloadModel();
        if(remove)  qDebug() << "Successfully dropped column" << oldName << "from the" << tableName() << "database table!";
        else        qDebug() << "Successfully renamed column" << oldName << "from the" << tableName() << "database table to" << newName;
        return true;
    } else {
        database().rollback();
        forceReloadModel();
        if(remove)  qCritical() << "Unable to drop column" << oldName << "from the" << tableName() << "database table.";
        else        qCritical() << "Unable to rename column" << oldName << "to" << newName << "from the" << tableName() << "database table!";
        return false;
    }

}


//same motherfucking problem as deleting a column with SQLite.
//Has to be fundamentally reworked to create a new table and copy shit over all inefficiently as fuck!
bool SqlTableModel::renameSqlColumn(QString oldName, QString newName) {
    if(database().driverName() == "QSQLITE") return _sqliteRenameSqlColumn(oldName, newName);
    else return _normalRenameSqlColumn(oldName, newName);
}

QString SqlTableModel::columnNameFromIndex(int index) const {
    Q_ASSERT(index >= 0 && index < columnCount());
    return headerData(index, Qt::Horizontal, Qt::DisplayRole).toString();

}

int SqlTableModel::columnIndexFromName(QString name) const {
    for(int i = 0; i < columnCount(); ++i) {
        if(headerData(i, Qt::Horizontal, Qt::DisplayRole).toString() == name) return i;
    }
    return -1;
}

QStringList SqlTableModel::getColumnNames(void) const {
    QStringList colNames;
    for(int i = 0; i < columnCount(); ++i) {
        colNames += columnNameFromIndex(i);
    }
    return colNames;
}

bool SqlTableModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    QVariant oldVal = index.data(role);
    if(QSqlTableModel::setData(index, value, role)) {
        emit cellDataChanged(index.row(), index.column(), oldVal, value, role);
        return true;
    } else return false;
}

bool SqlTableModel::pushFilter(QString string) {
    _filterStack.push(filter());
    setFilter(string);
    return select();
}

bool SqlTableModel::popFilter(void) {
    Q_ASSERT(_filterStack.size());
    QString filterStr = _filterStack.pop();
    setFilter(filterStr);
    return select();
}

unsigned SqlTableModel::updateColumnValues(int col, QVariant oldVal, QVariant newVal, int role) {
    int updatedCount = 0;
    for(int r = 0; r < rowCount(); ++r) {
        auto indexRef = index(r, col);
        if(indexRef.data(role) == oldVal) {
            updatedCount += setData(indexRef, newVal, role);
        }
    }
    return updatedCount;
}


//create a list of column names/types
//automatically handle create new table SQL query given parameters
//handle drag n drop
//handle MIME data?
//allow other tables to register as listeners when certain field(s) change?
