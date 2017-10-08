#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QMessageBox>
#include <QFileInfo>
#include <QDebug>
#include <stdio.h>
#include <stdarg.h>
#include "mainwindow.h"
#include "utilities.h"
#include "ui_mainwindow.h"
#include "ingredients_table.h"
#include "animal_nutrition_req_table.h"
#include "ration_table.h"
#include "rations_table_delegate.h"
//#include "qtdir/src/sql/drivers/sqlite/qsql_sqlite.cpp"
#include "ingredients_table_delegate.h"

#define SPRINTF_TEMP_BUFF_SIZE  2048

//===== STATIC =====

FILE* MainWindow::dbgLogFile = nullptr;

void MainWindow::_writeLog(const char* str) {
    if(dbgLogFile) {
        fprintf(dbgLogFile, "%s\n", str);
        fflush(dbgLogFile);
    }

    fflush(stdout);
    fprintf(stdout, "%s\n", str);
}

void MainWindow::dbgPrintf(const char* str, ...) {
    va_list argptr;
    char buffer[SPRINTF_TEMP_BUFF_SIZE];
    va_start(argptr, str);
    vsprintf(buffer, str, argptr);
    va_end(argptr);
    _writeLog(buffer);
}


void MainWindow::logQ(QtMsgType /*type*/, const QMessageLogContext &context, const QString &msg) {
    char buffer[SPRINTF_TEMP_BUFF_SIZE];
    sprintf(buffer, "%s (%s:%u, %s)", Q_CSTR(msg), context.file, context.line, context.function);
    _writeLog(buffer);
}

//===== INSTANCE =====

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _debugInit();
    _dbInit();

    ingredientsTable = new IngredientsTable(this, db);
    ingredientsTable->setTable("Ingredients");
    ingredientsTable->select();
    //ingredientsTable->insertColumns(0, 8);

    //ingredientsTable->insertRows(ingredientsTable->rowCount(), 4);
    ui->ingredientsTableView->setModel(ingredientsTable);
  //  ui->ingredientsTableView->setItemDelegate(new IngredientsTableDelegate);
    //ui->ingredientsTableView->setItemDelegateForColumn(1, new IngredientsTableDelegate);

    rationTable = new RationTable(this);
    RationsTableDelegate::setIngredientsTable(ingredientsTable);

    animalNutritionReqTable = new AnimalNutritionReqTable(this, db);
    animalNutritionReqTable->setTable("AnimalNutritionReq");
    animalNutritionReqTable->select();
    animalNutritionReqTable->setHeaderData(0, Qt::Horizontal, "Animal Type + Status");
    animalNutritionReqTable->setHeaderData(1, Qt::Horizontal, "DM, lbs");
    animalNutritionReqTable->setHeaderData(2, Qt::Horizontal, "NEm");
    animalNutritionReqTable->setHeaderData(3, Qt::Horizontal, "NEg");
    animalNutritionReqTable->setHeaderData(4, Qt::Horizontal, "Protein, lbs");
    animalNutritionReqTable->setHeaderData(5, Qt::Horizontal, "Ca, lbs");
    animalNutritionReqTable->setHeaderData(6, Qt::Horizontal, "P, lbs");
    animalNutritionReqTable->setHeaderData(7, Qt::Horizontal, "Vit A, IU");
    animalNutritionReqTable->setEditStrategy(QSqlTableModel::OnFieldChange);
    ui->animalNutritionReqTableView->setModel(animalNutritionReqTable);

    ui->rationCalculatorTableView->setModel(rationTable);
    ui->rationCalculatorTableView->setItemDelegate(new RationsTableDelegate);

    connect(ui->addIngredientButton, SIGNAL(clicked(bool)), this , SLOT(onAddIngredientClick(bool)));
    connect(ui->deleteIngredientButton, SIGNAL(clicked(bool)), this, SLOT(onDeleteIngredientClick(bool)));

    connect(ui->addRationButton, SIGNAL(clicked(bool)), this, SLOT(onAddRationClick(bool)));
    connect(ui->deleteRationButton, SIGNAL(clicked(bool)), this, SLOT(onDeleteRationClick(bool)));
    connect(ui->addAnimalNutritionReqButton, SIGNAL(clicked(bool)), this , SLOT(onAddAnimalNutritionReqClick(bool)));
    connect(ui->deleteAnimalNutritionReqButton, SIGNAL(clicked(bool)), this, SLOT(onDeleteAnimalNutritionReqClick(bool)));
}

MainWindow::~MainWindow(void)
{
    fclose(dbgLogFile);
    delete ui;
}

bool MainWindow::_debugInit(void) {
    bool success = true;

    dbgPrintf("Detecting user's home path: [%s]", Q_CSTR(QDir::homePath()));
    QDir::setCurrent(QDir::homePath());

    if(QDir::homePath() != QDir::current().absolutePath()) {
        dbgPrintf("Failed to 'cd' into home directory!");
        success = false;
    } else {
        userDir = QDir(USER_DIR_NAME);
        if(userDir.exists()) {
            dbgPrintf("Existing application root directory detected: [%s]", Q_CSTR(userDir.absolutePath()));
        } else {
            if(QDir::current().mkdir(USER_DIR_NAME)) {
                userDir = QDir(USER_DIR_NAME);
                if(userDir.exists()) {
                    dbgPrintf("Successfully created application root directory: [%s]", Q_CSTR(userDir.absolutePath()));
                } else {
                    dbgPrintf("Failed to open application root directory: [%s]", Q_CSTR(userDir.absolutePath()));
                    success = false;
                }
            } else {
                dbgPrintf("Failed to create application root directory: [%s]", Q_CSTR(userDir.absolutePath()));
                success = false;
            }
        }
    }

    if(success) QDir::setCurrent(userDir.absolutePath());

    dbgPrintf("Creating debug log file: [%s]", Q_CSTR(QDir::current().absoluteFilePath(DEBUG_LOG_NAME)));
    dbgLogFile = fopen(DEBUG_LOG_NAME, "w");
    if(!dbgLogFile) {
        dbgPrintf("Failed to create debug log file!");
    }

    qInstallMessageHandler(&logQ);

    return success;
}

bool MainWindow::_dbInit(void) {
    bool success        = true;
    bool createNewDb    = false;
    QString dbPath      = QDir::current().absoluteFilePath(DATABASE_NAME);
    QFileInfo dbFileInfo(dbPath);

    dbgPrintf("Querying for available database drivers:");
    QStringList driverList = QSqlDatabase::drivers();
    unsigned number = 0;
    for(auto driver: driverList) {
        dbgPrintf("\t[%d] - %s", ++number, Q_CSTR(driver));

    }

    if(dbFileInfo.exists()) {
        dbgPrintf("Using existing database file: [%s]", Q_CSTR(dbFileInfo.absoluteFilePath()));
    } else {
        dbgPrintf("Creating a new database file: [%s]", Q_CSTR(dbFileInfo.absoluteFilePath()));
        createNewDb = true;
    }

    dbgPrintf("Setting DB connection type: %s", DATABASE_TYPE);
    db = QSqlDatabase::addDatabase(DATABASE_TYPE);
    db.setDatabaseName(dbPath);

    if(!db.open()) {
        qCritical() << "Database connection failed: " << dbPath << db.lastError();
        success = false;
    } else {
        dbgPrintf("Successfully connected to database!");
    }

    if(createNewDb) {
        QSqlQuery query;
        if(!query.exec("create table Ingredients"
                  "(name varchar(50) primary key, "
                  "dm real, "
                  "nem real,"
                  "neg real,"
                   "protein real,"
                   "ca real,"
                   "p real,"
                       "vita real)"))
        {
            qCritical() << "Create ingredients table query failed: " << query.lastError();
            success = false;
        } else {
            dbgPrintf("Create ingredients table query succeeded!");
        }

        if(!query.exec("create table AnimalNutritionReq"
                  "(type varchar(50) primary key, "
                  "dm real, "
                  "nem real,"
                  "neg real,"
                   "protein real,"
                   "ca real,"
                   "p real,"
                       "vita real)"))
        {
            qCritical() << "Create animalNutritionReq table query failed: " << query.lastError();
            success = false;
        } else {
            dbgPrintf("Create animalNutritionReq table query succeeded!");
        }
    }

    return success;
}

void MainWindow::closeEvent(QCloseEvent*/*event*/) {
    dbgPrintf("Submitting any pending changes to DB on before exitting.");
    if(ingredientsTable->submitAll()) {
        db.commit();
    }
}

void MainWindow::onAddIngredientClick(bool) {
    ingredientsTable->insertRows(ingredientsTable->rowCount(), 1);
    if(ingredientsTable->submitAll()) {
        ingredientsTable->database().commit();
    }
    //ingredientsTable->select();
    //ingredientsTable->setData(0, "hi", Qt::EditRole)
}

void MainWindow::onAddAnimalNutritionReqClick(bool) {
    animalNutritionReqTable->insertRows(animalNutritionReqTable->rowCount(), 1);
    if(animalNutritionReqTable->submitAll()) {
        animalNutritionReqTable->database().commit();
    }
}

void MainWindow::onDeleteIngredientClick(bool) {
    // check which row(s) is/are selected (if any)
    QItemSelectionModel *select = ui->ingredientsTableView->selectionModel();

    QModelIndexList selectedRows = select->selectedRows();
    if(selectedRows.size()) {

        for(auto it: selectedRows) ingredientsTable->removeRows(it.row(), 1);
        if(ingredientsTable->submitAll()) {
            ingredientsTable->database().commit();
        }
        ingredientsTable->select();

    } else {
        QMessageBox::critical(this, "Delete from Ingredients Table Failed", "Please select at least one entire row for deletion.");

    }
}

void MainWindow::onDeleteAnimalNutritionReqClick(bool) {
    // check which row(s) is/are selected (if any)
    QItemSelectionModel *select = ui->animalNutritionReqTableView->selectionModel();

    QModelIndexList selectedRows = select->selectedRows();
    if(selectedRows.size()) {

        for(auto it: selectedRows) animalNutritionReqTable->removeRows(it.row(), 1);
        if(animalNutritionReqTable->submitAll()) {
            animalNutritionReqTable->database().commit();
        }
        animalNutritionReqTable->select();

    } else {
        QMessageBox::critical(this, "Delete from AnimalNutritionReqTable Failed", "Please select at least one entire row for deletion.");

    }
}

void MainWindow::onAddRationClick(bool) {
    rationTable->insertRow(rationTable->rowCount());
}

void MainWindow::onDeleteRationClick(bool) {
    // check which row(s) is/are selected (if any)
    QItemSelectionModel* select = ui->rationCalculatorTableView->selectionModel();

    QModelIndexList selectedRows = select->selectedRows();
    if(selectedRows.size()) {
        for(auto it: selectedRows) rationTable->removeRows(it.row(), 1);
    } else {
        QMessageBox::critical(this, "Delete Failed", "Please select at least one entire row for deletion.");

    }

}


