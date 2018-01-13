#include <QFile>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDriver>
#include <QSqlRecord>
#include <climits>
#include <QTextStream>
#include <cfloat>
#include <QDebug>
#include "model/animal_nutrition_req_table.h"
#include "model/ingredients_table.h"

#include "model/sql_table.h"
#include "core/utilities.h"
#include "ui/mainwindow.h"

SqlTableModel::SqlTableModel(QObject *parent, QSqlDatabase database):
    QSqlTableModel(parent, database)
{
#if 0
    connect(this, &QSqlTableModel::dataChanged, [&](const QModelIndex&, const QModelIndex&) {
        prefetchTableData();
    });
    connect(this, &QSqlTableModel::rowsInserted, [&](const QModelIndex&, int, int) {
        prefetchTableData();
    });
    connect(this, &QSqlTableModel::rowsRemoved, [&](const QModelIndex&, int, int) {
        prefetchTableData();
    });
    connect(this, &QSqlTableModel::layoutChanged, [&](void) {
        prefetchTableData();
    });
#endif
}


bool SqlTableModel::protectedDbCommit(void) {
    if(submitAll()) {
        if(!database().commit()) {
#if 0
            MainWindow::dbgPrintf("Error commiting database within table [%s]", Q_CSTR(tableName()));
            MainWindow::dbgPush();
            MainWindow::dbgPrintf("Msg: %s", Q_CSTR(database().lastError().text()));
            MainWindow::dbgPop();
            return false;
#endif
        }
    }
    return true;
}

bool SqlTableModel::protectedSelect(void) {
    if(!select()) {
        MainWindow::dbgPrintf("select() failed on DB table [%s]", Q_CSTR(tableName()));
        return false;
    }
    return true;
}

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

         MainWindow::dbgPrintf("File [%s] could not be opened for writing: %s", Q_CSTR(filePath), Q_CSTR(csvFile.errorString()));
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

         MainWindow::dbgPrintf("File [%s] could not be opened for writing: %s", Q_CSTR(filePath), Q_CSTR(csvFile.errorString()));
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
    protectedSelect();
}

bool SqlTableModel::addSqlColumn(QString colName, QString dataType, QVariant defaultVal, QString other) {
    QSqlQuery query;
    char sqlCmdBuff[SQL_TABLE_MODEL_MAX_QUERY_SIZE];
    char defValBuff[200] = { 0 };
    bool success;

    MainWindow::dbgPrintf("Adding column[%s (%s) %s] to table %s", Q_CSTR(colName), Q_CSTR(dataType), Q_CSTR(other), Q_CSTR(tableName()));

    MainWindow::dbgPush();

    if(!defaultVal.isNull() && defaultVal.isValid()) {
        sprintf(defValBuff, "default %s", Q_CSTR(defaultVal.toString()));
    }

    sprintf(sqlCmdBuff,
            "ALTER TABLE %s\n\tADD COLUMN %s %s %s %s;", Q_CSTR(tableName()), Q_CSTR(colName), Q_CSTR(dataType), defValBuff, Q_CSTR(other));

    if(!query.exec(sqlCmdBuff)) {
        MainWindow::dbgPrintf("Query Failed with Error: %s", Q_CSTR(query.lastError().text()));
        MainWindow::dbgPrintf("Query: %s", sqlCmdBuff);
        success = false;
    } else {
        _addColumnEntry({ colName, dataType, defaultVal, other });
        protectedDbCommit();
        forceReloadModel();
        success = true;
    }

    MainWindow::dbgPop();
    return success;

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
        MainWindow::dbgPrintf("Query Failed with Error: %s", Q_CSTR(query.lastError().text()));
        MainWindow::dbgPrintf("Query: %s", sqlCmdBuff);
        return false;
    } else {
        protectedDbCommit();
        forceReloadModel();
        return true;
    }
}

bool SqlTableModel::dropSqlColumn(QString colName) {
    MainWindow::dbgPrintf("Dropping column [%s] from table [%s].", Q_CSTR(colName), Q_CSTR(tableName()));
    MainWindow::dbgPush();

    bool success;
    if(database().driverName() == "QSQLITE") success = _sqliteDropSqlColumn(colName);
    else success = _normalDropSqlColumn(colName);

    if(success) {
        _removeColumnEntry(colName);
    }

    MainWindow::dbgPop();
    return success;
}

bool SqlTableModel::changeSqlColumnDataType(QString colName, QString dataType) {

    QSqlQuery query;
    char sqlCmdBuff[SQL_TABLE_MODEL_MAX_QUERY_SIZE];
    sprintf(sqlCmdBuff,
            "ALTER TABLE %s"
            "ALTER COLUMN %s %s;", Q_CSTR(tableName()), Q_CSTR(colName), Q_CSTR(dataType));

    if(!query.exec(sqlCmdBuff)) {
        MainWindow::dbgPrintf("Query Failed with Error: %s", Q_CSTR(query.lastError().text()));
        MainWindow::dbgPrintf("Query: %s", sqlCmdBuff);
        return false;
    } else {
       //  qDebug() << "Successfully altered column" << colName << "in the" << tableName() << "database table!";
        protectedDbCommit();
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
        MainWindow::dbgPrintf("Query Failed with Error: %s", Q_CSTR(query.lastError().text()));
        MainWindow::dbgPrintf("Query: %s", sqlCmdBuff);
        return false;
    } else {
        // qDebug() << "Successfully renamed column" << oldName << "in the" << tableName() << "database table to" << newName << "!";

        protectedDbCommit();
        forceReloadModel();
        return true;
    }
}

bool SqlTableModel::_sqliteRenameSqlColumn(QString oldName, QString newName) {
  //  pushFilter();
    QSqlQuery query(MainWindow::getDb());
    QString sqlCmdBuff;

    MainWindow::dbgPrintf("SqlTableModel::_sqliteRenameSqlColumn(%s, %s)", oldName.isNull()? "NULL" : Q_CSTR(oldName), newName.isNull()? "NULL" : Q_CSTR(newName));
    MainWindow::dbgPush();

    const bool remove = (newName.isNull() || newName.isEmpty());

    const int column = columnIndexFromName(oldName);
    if(column == -1) {
         MainWindow::dbgPrintf("Invalid column index retrived from oldName!");
         MainWindow::dbgPop();
         return false;
    }

    auto srcColEntries = _colEntries;
    auto dstColEntries = _colEntries;

    if(remove) {
        srcColEntries.remove(column);           //skip (remove) entry source and dest
        dstColEntries.remove(column);
    } else {
        dstColEntries[column].name = newName;   //rename entry in dest
    }

    QString srcColNameStrings;
    QString dstColNameStrings;
    QString dstColInitStrings;

    for(int i = 0; i < srcColEntries.size(); ++i) {
        srcColNameStrings += srcColEntries[i].name;
        dstColNameStrings += dstColEntries[i].name;
        dstColInitStrings += dstColEntries[i].toSqlString();
        if(i + 1 < srcColEntries.size()) {
            srcColNameStrings += ", ";
            dstColNameStrings += ", ";
            dstColInitStrings += ", ";
        }
    }

    sqlCmdBuff = QString(
            "CREATE TEMPORARY TABLE %1_backup(%2);\n"
            "INSERT INTO %3_backup SELECT %4 FROM %5;\n"
            "DROP TABLE %6;\n"
            "CREATE TABLE %7(%8);\n"
            "INSERT INTO %9 SELECT %10 FROM %11_backup;\n"
            "DROP TABLE %12_backup;").arg(Q_CSTR(tableName()),
                                          Q_CSTR(dstColInitStrings), Q_CSTR(tableName()), Q_CSTR(srcColNameStrings)).
            arg(Q_CSTR(tableName()), Q_CSTR(tableName()),
                Q_CSTR(tableName()), Q_CSTR(dstColInitStrings), Q_CSTR(tableName()), Q_CSTR(dstColNameStrings), Q_CSTR(tableName()), Q_CSTR(tableName())
            );    

    QStringList queryTextList = QString(sqlCmdBuff).split("\n");

    MainWindow::dbgPrintf("Final Query:");
    MainWindow::dbgPush();
    for(int i = 0; i < queryTextList.size(); ++i) {
        MainWindow::dbgPrintf("%s", Q_CSTR(queryTextList[i]));
    }
    MainWindow::dbgPop();

    Q_ASSERT(database().driver()->hasFeature(QSqlDriver::Transactions));

    MainWindow::getInstance()->_animalNutritionReqTable->prefetchTableData();
    MainWindow::getInstance()->_ingredientsTable->prefetchTableData();
    while(canFetchMore()) fetchMore();

    bool queryFailed = false;
    database().transaction();
    for(int i = 0; i < queryTextList.size(); ++i) {
        if(!query.exec(queryTextList[i])) {
            qDebug() << query.lastError();
            //MainWindow::dbgPrintf("Query error: ", Q_CSTR(query.lastError().text()));
            //MainWindow::dbgPrintf("Database Error: %s", Q_CSTR(database().lastError().text()));
            queryFailed = true;
            break;
        }
        query.next();
    }
    query.finish();

    bool success;
    if(!queryFailed) {
        //if(submitAll()) {
        submitAll();
           database().commit();
        //}
        forceReloadModel();
        success = true;
    } else {
        MainWindow::dbgPrintf("Rolling back db.");
        database().rollback();
        forceReloadModel();

        success = false;
    }

    MainWindow::dbgPop();

  //  popFilter();

    return success;
}


//same motherfucking problem as deleting a column with SQLite.
//Has to be fundamentally reworked to create a new table and copy shit over all inefficiently as fuck!
bool SqlTableModel::renameSqlColumn(QString oldName, QString newName) {
    MainWindow::dbgPrintf("Renaming column [%s] to [%s] in table [%s].", Q_CSTR(oldName), Q_CSTR(newName), Q_CSTR(tableName()));
    MainWindow::dbgPush();

    bool success;
    if(database().driverName() == "QSQLITE") success = _sqliteRenameSqlColumn(oldName, newName);
    else success = _normalRenameSqlColumn(oldName, newName);

    if(success) {
        int idx = columnIndexFromName(oldName);
        auto entry = _getColumnEntry(idx);
        entry.name = newName;
        _setColumnEntry(idx, entry);
    }

    MainWindow::dbgPop();
    return success;
}

QString SqlTableModel::columnNameFromIndex(int index) const {
    Q_ASSERT(index >= 0 && index < columnCount());
    return _colEntries[index].name;
}

int SqlTableModel::columnIndexFromName(QString name) const {
    auto it = _colEntryNameToIndexHash.constFind(name);
    return (it == _colEntryNameToIndexHash.constEnd())? -1 : it.value();
}

QStringList SqlTableModel::getColumnNames(void) const {
    QStringList colNames;
    for(int i = 0; i < columnCount(); ++i) {
        colNames += _getColumnEntry(i).name;
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
    return protectedSelect();
}

bool SqlTableModel::popFilter(void) {
    Q_ASSERT(_filterStack.size());
    QString filterStr = _filterStack.pop();
    setFilter(filterStr);
    return protectedSelect();
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

QString SqlTableModel::makeCreateTableQueryString(QString name) const {
    QString str = "create table ";
    str += (name.isNull() || name.isEmpty())? tableName() : name;
    str += " (\n";

    for(int i = 0; i < _colEntries.size(); ++i) {
        str += "\t";
        str += _colEntries[i].toSqlString();
        if(i+1 < _colEntries.size()) str += ",\n";
    }

   str += "\n)";

    return str;
}

void SqlTableModel::addColumnData(QString name, QString type, QVariant defVal, QString other) {
    _addColumnEntry({ name, type, defVal, other });
}

bool SqlTableModel::removeColumnData(QString name) {
    _removeColumnEntry(name);
    return true;
}

bool SqlTableModel::appendNewRow(QVector<QPair<int, QVariant>> defaultVals) {

    bool success = true;
    pushFilter();
    int count = rowCount();

    MainWindow::dbgPrintf("Appending new row to table [%s]", Q_CSTR(tableName()));
    MainWindow::dbgPush();

    insertRows(count, 1);

    submitAll();
    database().commit();

    auto defaultVal = [&](int col, QVariant old) -> QVariant {
        for(int cx = 0; cx < defaultVals.size(); ++cx) {
            if(col == defaultVals[cx].first)
                return defaultVals[cx].second;
        }
        QVariant colDefaultVal = _getColumnEntry(col).defaultVal;
        return (!colDefaultVal.isNull() && colDefaultVal.isValid())? colDefaultVal : old;
    };

    for(int c = 0; c < columnCount(); ++c) {
        auto idx = index(count, c);
        QVariant old = idx.data(Qt::EditRole);
        setData(idx, dummyVariantForColumn(c), Qt::EditRole);
        setData(idx, defaultVal(c, old), Qt::EditRole);
    }

    submitAll();
    database().commit();

    if(count+1 == rowCount()) {
    } else {
        MainWindow::dbgPrintf("Row wasn't added properly. Expected rowCount: %d, Actual rowCount: %d", count+1, rowCount());
        success = false;
    }


    submitAll();
    database().commit();
    success &= protectedSelect();
    popFilter();
    protectedSelect();

    MainWindow::dbgPop();
    return success;
}

bool SqlTableModel::deleteRows(QModelIndexList& list) {
    bool success = true;
    int expectedRows = rowCount() - list.size();

    MainWindow::dbgPrintf("Deleting %d rows from table [%s].", list.size(), Q_CSTR(tableName()));
    MainWindow::dbgPush();

    QList<QPersistentModelIndex> persistList;


    for(auto idx: list) persistList.push_back(idx);
    for(auto pIdx: persistList) {
        Q_ASSERT(pIdx.isValid());
        removeRows(pIdx.row(), 1);
    }

    if(protectedDbCommit()) {

        if(protectedSelect()) {
            if(expectedRows != rowCount()) {
                MainWindow::dbgPrintf("Something went wrong. Expected rowCount: %d, Actual rowCount: %d", expectedRows, rowCount());
                success = false;
            }
        } else success = false;

    } else success = false;

    success &= protectedSelect();

    MainWindow::dbgPop();
    return success;
}

int SqlTableModel::rowIndexWithColumnValue(int col, QVariant value) const {
    for(int i = 0; i < rowCount(); ++i) {
        QModelIndex idx = index(i, col);
        Q_ASSERT(idx.isValid());
        if(data(idx) == value) {
            return i;
        }
    }
    return -1;
}


QString SqlTableModel::makeColumnDataUnique(int col, QString prefix) const {
    Q_ASSERT(col >= 0 && col < columnCount());

    int num = 0;
    QString str;
    do {
        str = prefix;
        str += QString::number(++num);
    } while(rowIndexWithColumnValue(col, str) != -1);

    return str;
}


SqlTableRowEntry::SqlTableRowEntry(int row, SqlTableModel *model):
    _model(model)
{
    Q_ASSERT(model);

    if(row >= 0 && row < model->rowCount()) { //an actual row existing within the DB
        _colData.reserve(model->columnCount());
        for(int i = 0; i < model->columnCount(); ++i) {
            _colData.push_back(model->data(_model->index(row, i)));
        }
    } else { //this is just a new entry that doesn't really exist within the DB
        _colData.resize(model->columnCount());
        //model->database().record(model->tableName()).

    }
}


SqlTableRowEntry SqlTableModel::getTableRowEntry(int row) {
    Q_ASSERT(row >= 0 && row < rowCount());
    return SqlTableRowEntry(row, this);
}

SqlTableRowEntry SqlTableModel::createTableRowEntry(void) {
    return SqlTableRowEntry(-1, this);
}

QModelIndexList SqlTableModel::findRowsWithColumnValue(int col, QVariant variant) {
    Q_ASSERT(col >= 0 && col < columnCount());

    QModelIndexList list;

    for(int i = 0; i < rowCount(); ++i) {
        QModelIndex idx = index(i, col);
        if(idx.data() == variant) list.push_back(idx);
    }

    return list;
}

QVariant SqlTableModel::getVariant(int row, int col, int role) const {
    auto idx = index(row, col);
    Q_ASSERT(idx.isValid());
    return idx.isValid()? idx.data(role) : QVariant();
}


void SqlTableModel::setVariant(int row, int col, QVariant variant, int role) {
    auto idx = index(row, col);
    Q_ASSERT(idx.isValid());
    if(idx.isValid()) setData(idx, variant, role);
}


void SqlTableModel::addColumnRelationship(int localCol, int srcCol, SqlTableModel *model, bool oneToOne, int role) {
    _colRelationEntries.push_back(ColumnRelationEntry{ localCol, srcCol, model, oneToOne, role });
    connect(model, &SqlTableModel::cellDataChanged, [=](int r, int c, QVariant oldVariant, QVariant newVariant, int rol) {
        if(rol == role && c == srcCol) {
            pushFilter();
            iterateForColumnVariants(localCol, [&](int row, QVariant v) {
                if(v == oldVariant) {
                    setData(index(row, localCol), newVariant, role);
                }
            });
            popFilter();
        }
    });
}

void SqlTableModel::addColumnsFromRowsRelationship(int srcCol, SqlTableModel* model, QString type, QVariant defValue, int role) {
    _dynColEntry = {
        srcCol,
        model,
        {
            QString(),
            type,
            defValue,
            QString(),
        },
        role
    };

    auto varIsGoodString = [](QVariant var) {
        return (!var.isNull() && var.isValid() && !var.toString().isEmpty() && !var.toString().isNull());
    };
    //remove column
    connect(model, &SqlTableModel::rowsRemoved, [=](const QModelIndex& parent, int first, int last) {
        for(int r =  first; r <= last; ++r) {
            auto idx = model->index(r, srcCol, parent);
            Q_ASSERT(idx.isValid());
            auto name = idx.data(role);
            pushFilter();
            dropSqlColumn(name.toString());
            popFilter();
        }
    });


    //add/rename column
    connect(model, &SqlTableModel::cellDataChanged, [=](int r, int c, QVariant oldVariant, QVariant newVariant, int rol) {
        if(rol == role && c == srcCol) {
            //pushFilter();
            if(varIsGoodString(newVariant)) {
                if(varIsGoodString(oldVariant)) renameSqlColumn(oldVariant.toString(), newVariant.toString());
                else addSqlColumn(newVariant.toString(), type, defValue);

            } else {
                MainWindow::dbgPrintf("Table[%s] rename/add column from row of [%s]: new column/row string is invalid!", Q_CSTR(tableName()), Q_CSTR(model->tableName()));
            }
           // popFilter();
        }
    });

}


//create a list of column names/types
//automatically handle create new table SQL query given parameters
//handle drag n drop
//handle MIME data?
//allow other tables to register as listeners when certain field(s) change?

QVariant SqlTableModel::dummyVariantForColumn(int col) const {
    Q_ASSERT(col >= 0 && col < columnCount());
    auto entry = _getColumnEntry(col);
    if(entry.type == "real") {
        return QVariant(FLT_MAX);
    } else if(entry.type == "int") {
        return QVariant(INT_MAX);
    } else {
        return QVariant("string");
    }
}

//Required for when the application starts up to sync columns, as opposed to when they were added interactively
void SqlTableModel::populateDynamicColumnEntries(void) {
    for(int r = 0; r < _dynColEntry.srcModel->rowCount(); ++r) {
        //(_dynColEntry.srcModel->index(r, _dynColEntry.srcCol).data(_dynColEntry.role).toString(), _dynColEntry.colEntry.type, _dynColEntry.colEntry.defaultVal);
        _addColumnEntry({_dynColEntry.srcModel->index(r, _dynColEntry.srcCol).data(_dynColEntry.role).toString(),
                        _dynColEntry.colEntry.type,
                        _dynColEntry.colEntry.defaultVal,
                        QString()});
    }
}

void SqlTableModel::addHeaderData(int col, Qt::Orientation orient, QString name, QString tooltip) {
    setHeaderData(col, orient, name, Qt::EditRole);
    setHeaderData(col, orient, tooltip, Qt::ToolTipRole);
}

int SqlTableModel::rowCount(const QModelIndex& parent) const {
   // Q_ASSERT(!canFetchMore(parent));
    return QSqlTableModel::rowCount(parent);
}

int SqlTableModel::columnCount(const QModelIndex& parent) const {
    //Q_ASSERT(!canFetchMore(parent));
    return QSqlTableModel::columnCount(parent);
}

void SqlTableModel::prefetchTableData(void) {
  //  pushFilter();
    while(canFetchMore()) fetchMore();
  //  popFilter();
}
