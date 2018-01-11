#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QMessageBox>
#include <QFileInfo>
#include <QStringListModel>
#include <QToolButton>
#include <QtMath>
#include <QFileDialog>
#include <cfloat>
#include <cmath>
#include <QSqlRelationalDelegate>

#include <stdio.h>
#include <stdarg.h>

#include "ui_mainwindow.h"
#include "ui_error_warning_dialog.h"

#include "ui/mainwindow.h"
#include "core/utilities.h"
#include "model/ingredients_table.h"
#include "model/animal_nutrition_req_table.h"
#include "model/ration_table.h"
#include "model/recipe_table.h"
#include "model/nutrient_table.h"
#include "model/animal_table.h"
#include "delegate/rations_table_delegate.h"
#include "ui/error_warning_dialog.h"
#include "delegate/ingredients_table_delegate.h"
#include "delegate/animal_nutrition_table_delegate.h"
#include "delegate/totals_table_delegate.h"
#include "delegate/nutrients_table_delegate.h"
#include "delegate/recipe_table_delegate.h"

#define SPRINTF_TEMP_BUFF_SIZE  2048

//===== STATIC =====

FILE* MainWindow::_dbgLogFile = nullptr;
int MainWindow::_logDepth   = 0;

void MainWindow::_writeLog(const char* str) {
    char buff[SPRINTF_TEMP_BUFF_SIZE] = { 0 };

    for(int i = 0; i < _logDepth; ++i) {
        strcat(buff, "\t");
    }

    strcat(buff, str);
    if(_dbgLogFile) {
        fprintf(_dbgLogFile, "%s\n", buff);
        fflush(_dbgLogFile);
    }

    fflush(stdout);
    fprintf(stdout, "%s\n", buff);
}

void MainWindow::dbgPrintf(const char* str, ...) {
    va_list argptr;
    char buffer[SPRINTF_TEMP_BUFF_SIZE];
    va_start(argptr, str);
    vsprintf(buffer, str, argptr);
    va_end(argptr);
    _writeLog(buffer);
}

void MainWindow::dbgPush(void) {
    ++_logDepth;
}

void MainWindow::dbgPop(int depth) {
    _logDepth -= depth;
    Q_ASSERT(_logDepth >= 0);

}

void MainWindow::logQ(QtMsgType /*type*/, const QMessageLogContext &context, const QString &msg) {
    char buffer[SPRINTF_TEMP_BUFF_SIZE];
    sprintf(buffer, "%s (%s:%u, %s)", Q_CSTR(msg), context.file, context.line, context.function);
    _writeLog(buffer);
}

//===== INSTANCE =====

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow)
{
    _ui->setupUi(this);

    _debugInit();
    _printBuildInfo();
    _dbInit();
    _tableInit();

    _ui->ingredientsTableView->setItemDelegate(new IngredientsTableDelegate);
    _ui->animalNutritionReqTableView->setItemDelegate(new AnimalNutritionTableDelegate);
    //_ui->nutritionalTotalsTableWidget->setItemDelegate(new TotalsTableDelegate);
    _ui->rationTableView->setItemDelegate(new RationsTableDelegate);
    _ui->nutrientsTableView->setItemDelegate(new NutrientsTableDelegate);
    _ui->recipeTableView->setItemDelegate(new RecipeTableDelegate);

    _ui->nutritionalTotalsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    _ui->calculationResultTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    _ui->animalNutritionReqTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    _ui->ingredientsTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    _ui->rationTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    _ui->recipeTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    _ui->nutrientsTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    _ui->startDateEdit->setDate(QDate::currentDate());

    _initStatusBar();

    //connect(ui->act);

    connect(_ui->addIngredientButton, SIGNAL(clicked(bool)), this , SLOT(onAddIngredientClick(bool)));
    connect(_ui->deleteIngredientButton, SIGNAL(clicked(bool)), this, SLOT(onDeleteIngredientClick(bool)));
    connect(_ui->addRationButton, SIGNAL(clicked(bool)), this, SLOT(onAddRationClick(bool)));
    connect(_ui->deleteRationButton, SIGNAL(clicked(bool)), this, SLOT(onDeleteRationClick(bool)));
    connect(_ui->addAnimalNutritionReqButton, SIGNAL(clicked(bool)), this , SLOT(onAddAnimalNutritionReqClick(bool)));
    connect(_ui->deleteAnimalNutritionReqButton, SIGNAL(clicked(bool)), this, SLOT(onDeleteAnimalNutritionReqClick(bool)));
    connect(_ui->recipeAddButton, SIGNAL(clicked(bool)), this, SLOT(onAddRecipeClick(bool)));
    connect(_ui->recipeDeleteButton, SIGNAL(clicked(bool)), this, SLOT(onDeleteRecipeClick(bool)));
    connect(_ui->addNutrientButton, SIGNAL(clicked(bool)), this, SLOT(onAddNutrientClick(bool)));
    connect(_ui->deleteNutrientButton, SIGNAL(clicked(bool)), this, SLOT(onDeleteNutrientClick(bool)));
    connect(_ui->calculatePushButton, SIGNAL(clicked(bool)), this, SLOT(onCalculateButtonClick(bool)));
    connect(_ui->animalComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(onAnimalComboBoxChange(QString)));
    connect(_ui->matureWeightComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(onMatureWeightComboBoxChange(QString)));
    connect(_ui->currentWeightComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(onCurrentWeightComboBoxChange(QString)));
    connect(_ui->adgComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(onAverageDailyGainComboBoxChange(QString)));
    connect(_ui->recipeComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(onRecipeComboBoxChange(QString)));
    connect(_ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onMainTabChange(int)));
}

MainWindow::~MainWindow(void)
{
    //delete _errorDialog;
    //delete _warnDialog;
    fclose(_dbgLogFile);
    delete _ui;
}

bool MainWindow::_debugInit(void) {
    bool success = true;

    dbgPrintf("Detecting user's home path: [%s]", Q_CSTR(QDir::homePath()));
    QDir::setCurrent(QDir::homePath());

    if(QDir::homePath() != QDir::current().absolutePath()) {
        dbgPrintf("Failed to 'cd' into home directory!");
        success = false;
    } else {
        _userDir = QDir(USER_DIR_NAME);
        if(_userDir.exists()) {
            dbgPrintf("Existing application root directory detected: [%s]", Q_CSTR(_userDir.absolutePath()));
        } else {
            if(QDir::current().mkdir(USER_DIR_NAME)) {
                _userDir = QDir(USER_DIR_NAME);
                if(_userDir.exists()) {
                    dbgPrintf("Successfully created application root directory: [%s]", Q_CSTR(_userDir.absolutePath()));
                } else {
                    dbgPrintf("Failed to open application root directory: [%s]", Q_CSTR(_userDir.absolutePath()));
                    success = false;
                }
            } else {
                dbgPrintf("Failed to create application root directory: [%s]", Q_CSTR(_userDir.absolutePath()));
                success = false;
            }
        }
    }

    if(success) QDir::setCurrent(_userDir.absolutePath());

    dbgPrintf("Creating debug log file: [%s]", Q_CSTR(QDir::current().absoluteFilePath(DEBUG_LOG_NAME)));
    _dbgLogFile = fopen(DEBUG_LOG_NAME, "w");
    if(!_dbgLogFile) {
        dbgPrintf("Failed to create debug log file!");
    }

    qInstallMessageHandler(&logQ);

    return success;
}

bool MainWindow::_tableInit(void) {
    dbgPrintf("Initializing Table Models");
    dbgPush();

    _nutrientTable->setTable("Nutrients");
    _nutrientTable->protectedSelect();
    _nutrientTable->insertHeaderData();
    _nutrientTable->setIngredientsTable(_ingredientsTable);
    _ui->nutrientsTableView->setModel(_nutrientTable);


    _ingredientsTable->setTable("Ingredients");
    _ingredientsTable->protectedSelect();
    _ingredientsTable->setNutrientTable(_nutrientTable);
    _ingredientsTable->insertHeaderData();
    _ui->ingredientsTableView->setModel(_ingredientsTable);


    _recipeTable->setTable("Recipes");
    _recipeTable->protectedSelect();
    _recipeTable->insertHeaderData();
    _ui->recipeTableView->setModel(_recipeTable);


    RationsTableDelegate::setIngredientsTable(_ingredientsTable);
    _rationTable->setIngredientsTable(_ingredientsTable);
    _rationTable->setRecipeTable(_recipeTable);
    _ui->rationTableView->setModel(_rationTable);
    _rationTable->setTable("Rations");
    //_rationTable->select();
    //_rationTable->insertHeaderData();

   // _rationTable->setRelation(RationTable::COL_RECIPE, QSqlRelation("Recipes", "name", "name"));
   // _rationTable->setRelation(RationTable::COL_INGREDIENT, QSqlRelation("Ingredients", "name", "name"));
    //_ui->rationTableView->setModel(_rationTable);
  //  _ui->rationTableView->setColumnHidden(RationTable::COL_RECIPE, true);
    //_ui->rationTableView->setItemDelegate(new QSqlRelationalDelegate(_ui->rationTableView));

    _animalTable->setTable("Animals");
    _animalTable->protectedSelect();
    _animalTable->insertHeaderData();
    _ui->animalTableView->setModel(_animalTable);

    _ui->animalNutritionReqTableView->setModel(_animalNutritionReqTable);
    _animalNutritionReqTable->setTable("AnimalNutritionReq");
    //_animalNutritionReqTable->protectedSelect();
    //_animalNutritionReqTable->insertHeaderData();
    _animalNutritionReqTable->setAnimalTable(_animalTable);

    _animalNutritionReqTable->populateDynamicColumnEntries();
    _ingredientsTable->populateDynamicColumnEntries();

    _ui->animalComboBox->setSrcModelColumn(_animalTable, AnimalTable::COL_NAME);
    _ui->animalComboBox->setEmptyErrorString("No Animal Types found! Please create one.");
    _ui->animalComboBox->populate();
    _ui->recipeComboBox->setSrcModelColumn(_recipeTable, RecipeTable::COL_NAME);
    _ui->animalComboBox->setEmptyErrorString("No Recipes found! Please create one.");
    _ui->recipeComboBox->populate();

    _ui->matureWeightComboBox->setSrcModelColumn(_animalNutritionReqTable, AnimalNutritionReqTable::COL_WEIGHT_MATURE);
    _ui->matureWeightComboBox->setFilterCallback([&](const QModelIndex& index) {
        return (_animalNutritionReqTable->index(index.row(), AnimalNutritionReqTable::COL_ANIMAL).data().toString() == _ui->animalComboBox->currentText());
    });
    _ui->matureWeightComboBox->setEmptyErrorString("No Nutritional Requirements found for the selected Animal Type. Please create at least one entry.");

    _ui->currentWeightComboBox->setSrcModelColumn(_animalNutritionReqTable, AnimalNutritionReqTable::COL_WEIGHT_CURRENT);
    _ui->currentWeightComboBox->setFilterCallback([&](const QModelIndex& idx) {
        auto entry = _animalNutritionReqTable->getTableRowEntry(idx.row());
        return (entry.getColumnValue<QString>(AnimalNutritionReqTable::COL_ANIMAL)              == _ui->animalComboBox->currentText()
             && entry.getColumnVariant(AnimalNutritionReqTable::COL_WEIGHT_MATURE).toString()   == _ui->matureWeightComboBox->currentText());
    });

    _ui->adgComboBox->setSrcModelColumn(_animalNutritionReqTable, AnimalNutritionReqTable::COL_DAILYGAIN);
    _ui->adgComboBox->setFilterCallback([&](const QModelIndex& idx) {
        auto entry = _animalNutritionReqTable->getTableRowEntry(idx.row());
        return (entry.getColumnValue<QString>(AnimalNutritionReqTable::COL_ANIMAL)              == _ui->animalComboBox->currentText()
             && entry.getColumnVariant(AnimalNutritionReqTable::COL_WEIGHT_MATURE).toString()   == _ui->matureWeightComboBox->currentText()
             && entry.getColumnVariant(AnimalNutritionReqTable::COL_WEIGHT_CURRENT).toString()   == _ui->currentWeightComboBox->currentText());
    });

    onAnimalComboBoxChange("FUCKING UPDATE!!!");

    connect(_recipeTable, SIGNAL(cellDataChanged(int,int,QVariant,QVariant,int)), this, SLOT(onRecipeValueChanged(int,int,QVariant,QVariant,int)));
    connect(_ui->recipeTableView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex&,const QModelIndex&)), this, SLOT(onRecipeSelectionChanged(const QModelIndex&,const QModelIndex&)));

    connect(_animalTable, SIGNAL(cellDataChanged(int,int,QVariant,QVariant,int)), this, SLOT(onAnimalValueChanged(int,int,QVariant,QVariant,int)));
    connect(_ui->animalTableView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex&,const QModelIndex&)), this, SLOT(onAnimalSelectionChanged(const QModelIndex&,const QModelIndex&)));

    _updateCalculationTabWidgets();

    dbgPop();
    return true;
}

bool MainWindow::_dbInit(QString dbType) {
    bool success        = true;
    bool createNewDb    = false;
    QString dbPath      = QDir::current().absoluteFilePath(DATABASE_NAME);
    QFileInfo dbFileInfo(dbPath);

    dbgPrintf("Initializing Database");
    dbgPush();

    dbgPrintf("Querying for available database drivers:");
    dbgPush();
    QStringList driverList = QSqlDatabase::drivers();
    unsigned number = 0;
    for(auto driver: driverList) {
        dbgPrintf("\t[%d] - %s", ++number, Q_CSTR(driver));
    }
    dbgPop();

    if(dbFileInfo.exists()) {
        dbgPrintf("Using existing database file: [%s]", Q_CSTR(dbFileInfo.absoluteFilePath()));
    } else {
        dbgPrintf("Creating a new database file: [%s]", Q_CSTR(dbFileInfo.absoluteFilePath()));
        createNewDb = true;
    }

    dbgPrintf("Setting DB connection type: %s", Q_CSTR(dbType));
    _db = QSqlDatabase::addDatabase(dbType);
    _db.setDatabaseName(dbPath);

    if(!_db.open()) {
        dbgPrintf("Database connection failed [Path: %s] [LastError: %s]", Q_CSTR(dbPath), Q_CSTR(_db.lastError().text()));
        success = false;
        _dbPath.clear();
    } else {
        dbgPrintf("Successfully connected to database!");
        _dbPath = dbPath;
    }

    delete _animalNutritionReqTable;
    delete _ingredientsTable;
    delete _recipeTable;
    delete _nutrientTable;
    delete _rationTable;
    delete _animalTable;

    _animalNutritionReqTable    = new AnimalNutritionReqTable(this, _db);
    _ingredientsTable           = new IngredientsTable(this, _db);
    _recipeTable                = new RecipeTable(this, _db);
    _nutrientTable              = new NutrientTable(this, _db);
    _rationTable                = new RationTable(this, _db);
    _animalTable                = new AnimalTable(this, _db);

    _nutrientTable->addColumnData("name", "varchar(" STRINGIFY_MACRO(NUTRIENT_NAME_SIZE) ")", QString(), "primary key");

    _animalTable->addColumnData("name", "varchar(" STRINGIFY_MACRO(ANIMAL_NAME_SIZE) ")", QString(), "primary key");

    _animalNutritionReqTable->addColumnData("animal", QString(), QString(), "references Animals(name)");
    _animalNutritionReqTable->addColumnData("currentweight", "real", 1000.0);
    _animalNutritionReqTable->addColumnData("matureweight", "real", 1000.0);
    _animalNutritionReqTable->addColumnData("dailygain", "real", 1.0);
    _animalNutritionReqTable->addColumnData("dmi", "real", 0.0);
    _animalNutritionReqTable->addColumnData("tdn", "real", 0.0);
    _animalNutritionReqTable->addColumnData("nem", "real", 0.0);
    _animalNutritionReqTable->addColumnData("neg", "real", 0.0);
    _animalNutritionReqTable->addColumnData("protein", "real", 0.0);
    _animalNutritionReqTable->addColumnData("calcium", "real", 0.0);
    _animalNutritionReqTable->addColumnData("phosphorus", "real", 0.0);
    _animalNutritionReqTable->addColumnData("vita", "real", 0.0);
    _animalNutritionReqTable->addColumnRelationship(AnimalNutritionReqTable::COL_ANIMAL, AnimalTable::COL_NAME, _animalTable);
    _animalNutritionReqTable->addColumnsFromRowsRelationship(NutrientTable::COL_NAME, _nutrientTable, "real", 0.0);

    _ingredientsTable->addColumnData("name", "varchar(" STRINGIFY_MACRO(INGREDIENT_NAME_SIZE) ")", QString(), "primary key");
    _ingredientsTable->addColumnData("dm", "real", 0.0);
    _ingredientsTable->addColumnData("tdn", "real", 0.0);
    _ingredientsTable->addColumnData("nem", "real", 0.0);
    _ingredientsTable->addColumnData("neg", "real", 0.0);
    _ingredientsTable->addColumnData("protein", "real", 0.0);
    _ingredientsTable->addColumnData("ca", "real", 0.0);
    _ingredientsTable->addColumnData("p", "real", 0.0);
    _ingredientsTable->addColumnData("vita", "real", 0.0);
    _ingredientsTable->addColumnsFromRowsRelationship(NutrientTable::COL_NAME, _nutrientTable, "real", 0.0);

    _recipeTable->addColumnData("name", "varchar(" STRINGIFY_MACRO(RECIPE_NAME_SIZE) ")", QString(), "primary key");

    _rationTable->addColumnData("recipeName", QString(), QString(), "references Recipes(name)");
    _rationTable->addColumnData("ingredientName", QString(), QString(), "references Ingredients(name)");
    _rationTable->addColumnData("asFed", "real", 0.0);
    _rationTable->addColumnData("cost", "real", 0.0);
    _rationTable->addColumnData("weight", "real", 0.0);
    _rationTable->addColumnData("costPerDay", "real", 0.0);
    _rationTable->addColumnData("dm", "real", 0.0);
    _rationTable->addColumnRelationship(RationTable::COL_INGREDIENT, IngredientsTable::COL_NAME, _ingredientsTable);
    _rationTable->addColumnRelationship(RationTable::COL_RECIPE, RecipeTable::COL_NAME, _recipeTable);

    if(/*createNewDb || */!_db.tables().contains("Ingredients")) {
        QSqlQuery query;
        if(!query.exec(_ingredientsTable->makeCreateTableQueryString("Ingredients")))
        {
            dbgPrintf("Create Ingredients table query failed: %s", Q_CSTR(query.lastError().text()));
            success = false;
        } else {
            dbgPrintf("Create Ingredients table query succeeded!");
        }
    } else dbgPrintf("Using existing Ingredients table.");


    if(/*createNewDb || */!_db.tables().contains("Recipes")) {
        QSqlQuery query;
        if(!query.exec(_recipeTable->makeCreateTableQueryString("Recipes")))
        {
            dbgPrintf("Create Recipes table query failed: %s", Q_CSTR(query.lastError().text()));
            success = false;
        } else {
            dbgPrintf("Create Recipes table query succeeded!");
        }
    } else dbgPrintf("Using existing Recipes table.");

    if(/*createNewDb || */!_db.tables().contains("Nutrients")) {
        QSqlQuery query;
        if(!query.exec(_nutrientTable->makeCreateTableQueryString("Nutrients")))
        {
            dbgPrintf("Create Nutrients table query failed: %s", Q_CSTR(query.lastError().text()));
            success = false;
        } else {
            dbgPrintf("Create Nutrients table query succeeded!");
        }
    } else dbgPrintf("Using existing Nutrients table.");

    if(/*createNewDb || */!_db.tables().contains("Rations")) {
        QSqlQuery query;
        if(!query.exec(_rationTable->makeCreateTableQueryString("Rations")))
        {
            dbgPrintf("Create Rations table query failed: %s", Q_CSTR(query.lastError().text()));
            success = false;
        } else {
            dbgPrintf("Create Rations table query succeeded!");
        }
    } else dbgPrintf("Using existing Rations table.");

    if(/*createNewDb || */!_db.tables().contains(ANIMAL_TABLE_NAME)) {
        QSqlQuery query;
        if(!query.exec(_animalTable->makeCreateTableQueryString(ANIMAL_TABLE_NAME)))
        {
            dbgPrintf("Create Animals table query failed: %s", Q_CSTR(query.lastError().text()));
            success = false;
        } else {
            dbgPrintf("Create " ANIMAL_TABLE_NAME "table query succeeded!");
        }
    } else dbgPrintf("Using existing Animals table.");


    if(/*createNewDb || */!_db.tables().contains("AnimalNutritionReq")) {
        QSqlQuery query;
        if(!query.exec(_animalNutritionReqTable->makeCreateTableQueryString("AnimalNutritionReq")))
        {
            dbgPrintf("Create AnimalNutritionReq table query failed: %s", Q_CSTR(query.lastError().text()));
            success = false;
        } else {
            dbgPrintf("Create AnimalNutritionReq table query succeeded!");
        }
    } else dbgPrintf("Using existing AnimalNutritionReq table.");


    dbgPop();
    return success;
}

void MainWindow::closeEvent(QCloseEvent*/*event*/) {
    bool needsCommit = false;
    dbgPrintf("Submitting any pending changes to DB on before exitting.");
    dbgPush();
    needsCommit |= _ingredientsTable->submitAll();
    needsCommit |= _animalNutritionReqTable->submitAll();
    needsCommit |= _nutrientTable->submitAll();
    needsCommit |= _rationTable->submitAll();
    needsCommit |= _recipeTable->submitAll();
    needsCommit |= _animalTable->submitAll();

    if(needsCommit) {
        _db.commit();
    }

    dbgPop();
}

void MainWindow::onAddIngredientClick(bool) {
    dbgPrintf("MainWindow::onAddIngredientClick");
    dbgPush();
    QString newName = _ingredientsTable->makeColumnDataUnique(IngredientsTable::COL_NAME, "New_Ingredient_");
    _ingredientsTable->appendNewRow({
                                   {IngredientsTable::COL_NAME, newName}
                               });
    dbgPop();
}

void MainWindow::onAddAnimalNutritionReqClick(bool) {
    dbgPrintf("MainWindow::onAddNutritionReqClick");
    dbgPush();
    auto selectedIndices = _ui->animalTableView->selectionModel()->selectedRows();
    //Q_ASSERT(selectedIndices.size() == 1);
    if(selectedIndices.size() >= 1) {
        auto selected = selectedIndices[0];

        if(selected.isValid()) {

            QString animalName = selected.data().toString();
            dbgPrintf("Selecting - %s", Q_CSTR(animalName));
            float matureWeight = _animalNutritionReqTable->getGreatestMatureWeight() + 1.0f;
            _animalNutritionReqTable->appendNewRow({
                { AnimalNutritionReqTable::COL_ANIMAL, animalName },
                { AnimalNutritionReqTable::COL_WEIGHT_MATURE, matureWeight },
                { AnimalNutritionReqTable::COL_WEIGHT_CURRENT, matureWeight-1.0f },
                { AnimalNutritionReqTable::COL_DMI, 0.5f }
             });
             _ui->animalNutritionReqTableView->scrollToBottom();
        } else {
            dbgPrintf("onAddAnimalNutritionReqClick() - WTF invalid selection index!?");
            Q_ASSERT(false);
        }
    }
    dbgPop();
}

void MainWindow::onDeleteIngredientClick(bool) {
    dbgPrintf("MainWindow::onDeleteIngredientClick");
    dbgPush();

    // check which row(s) is/are selected (if any)
    QItemSelectionModel *select = _ui->ingredientsTableView->selectionModel();

    QModelIndexList selectedRows = select->selectedRows();
    if(selectedRows.size()) {


        _rationTable->pushFilter();
        for(auto sel : selectedRows) {
            auto idxList = _rationTable->findRowsWithColumnValue(RationTable::COL_INGREDIENT, _ingredientsTable->index(sel.row(), IngredientsTable::COL_NAME).data());
            _rationTable->deleteRows(idxList);
        }
        _rationTable->popFilter();
        _ingredientsTable->deleteRows(selectedRows);

    } else {
        QMessageBox::critical(this, "Delete from Ingredients Table Failed", "Please select at least one entire row for deletion.");

    }

    dbgPop();
}

void MainWindow::onDeleteAnimalNutritionReqClick(bool) {
    dbgPrintf("MainWindow::onDeleteAnimalNutritionReqClick");
    dbgPush();
    // check which row(s) is/are selected (if any)
    QItemSelectionModel *select = _ui->animalNutritionReqTableView->selectionModel();
    if(!select) return;

    QModelIndexList selectedRows = select->selectedRows();
    if(selectedRows.size()) {
        _animalNutritionReqTable->deleteRows(selectedRows);

    } else {
        QMessageBox::critical(this, "Delete from Nutritional Requirements Table Failed", "Please select at least one entire row for deletion.");

    }
    dbgPop();

}

void MainWindow::onAddRationClick(bool) {
    dbgPrintf("MainWindow::onAddRationClick");
    dbgPush();

    auto selectedIndices = _ui->recipeTableView->selectionModel()->selectedRows();
    //Q_ASSERT(selectedIndices.size() == 1);
    if(selectedIndices.size() >= 1) {
        auto selected = selectedIndices[0];

        if(selected.isValid()) {
            auto unusedIngredients = _rationTable->getUnusedIngredientsList();

            if(!unusedIngredients.size()) {
                QMessageBox::critical(nullptr, "Failed to Create Ration", "You've already used all of the available ingredients in this recipe!");
                dbgPop();
                return;
            }

            QString recipeName = selected.data().toString();

            _rationTable->appendNewRow({
                                        { RationTable::COL_RECIPE, recipeName },
                                        { RationTable::COL_INGREDIENT, unusedIngredients.first() }
                                       });
            _ui->rationTableView->scrollToBottom();

        } else {
            Q_ASSERT(false);
        }
    }

    dbgPop();

}

void MainWindow::onDeleteRationClick(bool) {
    dbgPrintf("MainWindow::onDeleteRationClick");
    dbgPush();
    // check which row(s) is/are selected (if any)
    QItemSelectionModel *select = _ui->rationTableView->selectionModel();
    if(!select) {
        dbgPop();
        return;
    }

    QModelIndexList selectedRows = select->selectedRows();
    if(selectedRows.size()) {
        _rationTable->deleteRows(selectedRows);

    } else {
        QMessageBox::critical(this, "Delete from Rations Table Failed", "Please select at least one entire row for deletion.");

    }
    dbgPop();
}

void MainWindow::_validateCalculationInputs(void) {
    dbgPrintf("MainWindow::_validateCalculationInputs()");
    dbgPush();
    QStringList validatedIngredients;

    int     rows            = _rationTable->rowCount();

    _errorList.clear();
    _warningList.clear();


    if(!rows) {
        _errorList += QString("The selected Recipe '%1' does not contain any Ingredients!\n").arg(_ui->recipeComboBox->currentText());
    } else {

        for (int i = 0; i < rows; i++) {
            Ration ration = _rationTable->rationFromRow(i);
            ration.validate(_errorList);

            int ingredientRow = _ingredientsTable->rowFromName(ration.ingredient);
            if(ingredientRow == -1) {
                _errorList += QString("Ingredient '") + QString(ration.ingredient) + QString("' could not be found!\n");
            } else {
                if(!validatedIngredients.contains(ration.ingredient)) {
                    Ingredient ingredient = _ingredientsTable->ingredientFromRow(ingredientRow);
                    ingredient.validate(_errorList);
                    validatedIngredients.push_back(ration.ingredient);
                }
            }
        }
    }
    setStatusBarWarnings(_warningList);
    setStatusBarErrors(_errorList);

    _errorDialog->setEntryList(_errorList);
    _warnDialog->setEntryList(_warningList);

    if(_errorList.size()) {
        QMessageBox msgBox;
        msgBox.setText("Failed to calculate data!");
        msgBox.setInformativeText(QString("Missing one or more pieces of input data required to perform the calculations. See details for more information."));
        msgBox.setStandardButtons(QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);

        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setDetailedText(_errorList.join('\n'));
        QSpacerItem* horizontalSpacer = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        QGridLayout* layout = (QGridLayout*)msgBox.layout();
        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
        msgBox.exec();
    }

    dbgPop();
}

void MainWindow::_clearTotalsTable(void) {
    for(int r = 0; r < _ui->nutritionalTotalsTableWidget->rowCount(); ++r) {
        for(int c = 0; c < _ui->nutritionalTotalsTableWidget->columnCount(); ++c) {
            auto* item = _ui->nutritionalTotalsTableWidget->item(r, c);
            if(item) item->setData(Qt::EditRole, QString(""));
        }
    }
}

void MainWindow::_clearCalculationTable(void) {
    for(int r = 0; r < _ui->calculationResultTableWidget->rowCount(); ++r) {
        for(int c = 0; c < _ui->calculationResultTableWidget->columnCount(); ++c) {
            auto* item = _ui->calculationResultTableWidget->item(r, c);
            if(item) item->setData(Qt::EditRole, QString(""));
        }
    }
}

void MainWindow::_updateTotalsTableRows(void) {
    //Clear out previous dynamic rows from custom nutrients
    int rows = _ui->nutritionalTotalsTableWidget->rowCount();
    for(int i = rows-1; i >= BUILTIN_NUTR_COUNT; --i) {
        _ui->nutritionalTotalsTableWidget->removeRow(i);
    }

    //Insert new dynamic rows from nutrients
    rows = _nutrientTable->rowCount();
    auto& tableWidget = _ui->nutritionalTotalsTableWidget;
    for(int i = 0; i < rows; ++i) {
        tableWidget->insertRow(tableWidget->rowCount());
        tableWidget->setVerticalHeaderItem(tableWidget->rowCount()-1,
                                           new QTableWidgetItem(_nutrientTable->data(_nutrientTable->index(i, NutrientTable::COL_NAME)).toString()));

        QTableWidgetItem* item = new QTableWidgetItem();
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        item->setData(Qt::BackgroundRole, QColor(Qt::gray));
        tableWidget->setItem(tableWidget->rowCount()-1,
                             1,
                             item);


    }
}

QTableWidgetItem* MainWindow::_createItem(const char *fmt, ...) {
    char dest[1024*16];
    va_list args;
    va_start(args, fmt);
    vsprintf(dest, fmt, args);
    va_end(args);

    QTableWidgetItem* item = new QTableWidgetItem(QString(dest));
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    return item;
}

void MainWindow::onCalculateButtonClick(bool) {
    dbgPrintf("MainWindow::onCalculateButtonClick");
    dbgPush();

    _clearTotalsTable();
    _clearCalculationTable();

    _rationTable->pushFilter("recipeName = '" + _ui->recipeComboBox->currentText() + "'");
    _animalNutritionReqTable->pushFilter("animal = '" + _ui->animalComboBox->currentText() + "'");

    _validateCalculationInputs();
    _updateTotalsTableRows();

    if(_errorList.size()) {
        _ui->nutritionalTotalsTableWidget->setEnabled(false);
    } else {
        _ui->nutritionalTotalsTableWidget->setEnabled(true);

        int rows = _rationTable->rowCount();
        float totalFed = 0, totalNem = 0, totalTdn = 0, totalNeg = 0, totalProtein = 0, totalCa = 0, totalPhosphorus = 0, totalVita = 0, totalDm = 0;
        float totalCostPerDay = 0.0f;
        const int nutrientCount = _nutrientTable->rowCount();
        float totalHeads = _ui->totalHeadSpinBox->value();
        float totalNutrients[nutrientCount];
        for(int i = 0; i < nutrientCount; ++i) totalNutrients[i] = 0.0f;

            dbgPrintf("Nutrient Count - %d", nutrientCount);
        for (int i = 0; i < rows; i++) {
            dbgPrintf("Ration[%d]", rows);
            dbgPush();

            Ration ration = _rationTable->rationFromRow(i);
            totalCostPerDay += ration.costPerDay;
            dbgPrintf("Ingredient - %s", ration.ingredient);
            int ingredientRow = _ingredientsTable->rowFromName(ration.ingredient);

            Q_ASSERT(ingredientRow != -1);
            if (ingredientRow != -1) {
                using IngTable = IngredientsTable;
                auto ingr = _ingredientsTable->getTableRowEntry(ingredientRow);
                totalFed += ration.asFed;
                float dryMatter = ingr.getColumnValue<float>(IngTable::COL_DM)/100.0f * ration.asFed;

                totalDm         += dryMatter;
                totalTdn        += ingr.getColumnValue<float>(IngTable::COL_TDN)/100.0f        * dryMatter;
                totalNem        += ingr.getColumnValue<float>(IngTable::COL_NEM)/112.0f        * ration.asFed; //convert Mcal/cwt to Mcal/lb
                totalNeg        += ingr.getColumnValue<float>(IngTable::COL_NEG)/112.0f        * ration.asFed;
                totalProtein    += ingr.getColumnValue<float>(IngTable::COL_PROTEIN)/100.0f    * dryMatter;
                totalCa         += ingr.getColumnValue<float>(IngTable::COL_CA)/100.0f         * dryMatter;
                totalPhosphorus += ingr.getColumnValue<float>(IngTable::COL_P)/100.0f          * dryMatter;
                totalVita       += ingr.getColumnValue<float>(IngTable::COL_VITA)              * ration.asFed;

                for(int n = 0; n < nutrientCount; ++n) {
                    totalNutrients[n] += ingr.getColumnValue<float>(IngTable::COL_DYNAMIC + n) * ration.asFed;
                }
            } else dbgPrintf("Wtf, invalid row for ingredient name!?!");

            dbgPop();
        }

        auto setItem = [&](int row, int col, QTableWidgetItem* item) {
            auto* old = _ui->nutritionalTotalsTableWidget->item(row, col);
            if(old) item->setToolTip(old->toolTip());
            _ui->nutritionalTotalsTableWidget->setItem(row, col, item);
        };

        setItem(0, 0, _createItem("%.2f", totalDm));
        setItem(1, 0, _createItem("%.2f", totalTdn));
        setItem(2, 0, _createItem("%.2f", totalNem));
        setItem(3, 0, _createItem("%.2f", totalNeg));
        setItem(4, 0, _createItem("%.2f", totalProtein));
        setItem(5, 0, _createItem("%.2f", totalCa));
        setItem(6, 0, _createItem("%.2f", totalPhosphorus));
        setItem(7, 0, _createItem("%.2f", totalVita));

        for(int j = 0; j < nutrientCount; ++j) {
            setItem(BUILTIN_NUTR_COUNT+j, 0, _createItem("%.2f", totalNutrients[j]));
        }

        int nutReqsRow = _getCalcNutritionReqsRow();
        if(nutReqsRow == -1) {
            dbgPrintf("Could not find the row index for the selected Nutritional Requirements data!");
        }
        auto req = _animalNutritionReqTable->getTableRowEntry(nutReqsRow);

        float sum[BUILTIN_NUTR_COUNT] = {
            totalDm, totalTdn, totalNem, totalNeg, totalProtein, totalCa, totalPhosphorus, totalVita
        };

        using ReqTable = AnimalNutritionReqTable;

        float dmi = req.getColumnValue<float>(ReqTable::COL_DMI);
        float expected[BUILTIN_NUTR_COUNT] = {
            dmi,
            dmi * req.getColumnValue<float>(ReqTable::COL_TDN)/100.0f,
            totalFed * req.getColumnValue<float>(ReqTable::COL_NEM),
            totalFed *req.getColumnValue<float>(ReqTable::COL_NEG),
            dmi * req.getColumnValue<float>(ReqTable::COL_PROTEIN)/100.0f,
            dmi * req.getColumnValue<float>(ReqTable::COL_CALCIUM)/100.0f,
            dmi * req.getColumnValue<float>(ReqTable::COL_PHOSPHORUS)/100.0f,
            req.getColumnValue<float>(ReqTable::COL_VITA)
        };

        dbgPrintf("Calculating Totals");
        dbgPush();

        for(int i = 0; i < _ingredientsTable->columnCount()-1; ++i) {
            dbgPrintf("Totals row[%d]", i);
            dbgPush();

            float totalReq = (i < BUILTIN_NUTR_COUNT)? expected[i]   : req.getColumnValue<float>(ReqTable::COL_DYNAMIC + (i-BUILTIN_NUTR_COUNT));
            float totalSum = (i < BUILTIN_NUTR_COUNT)? sum[i]        : totalNutrients[i-BUILTIN_NUTR_COUNT];
            float diff = fabs(totalReq-totalSum);

            dbgPrintf("Total - %f", totalSum);
            dbgPrintf("Required - %f", totalReq);
            dbgPrintf("Diff - %f", diff);

            QTableWidgetItem* item =
                    (totalReq != 0.0f)?
                        _createItem("%.2f (%.1f%%)", totalReq, totalSum/totalReq*100.0f) :
                        _createItem("%.2f", totalReq);

            if(diff <= totalReq*TOTALS_TABLE_DIFF_GREEN_RANGE) {
                item->setData(Qt::BackgroundRole, QColor(Qt::green));
            } else if(diff <= totalReq*TOTALS_TABLE_DIFF_YELLOW_RANGE) {
                item->setData(Qt::BackgroundRole, QColor(Qt::yellow));
            } else {
                item->setData(Qt::BackgroundRole, QColor(Qt::red));
            }

            setItem(i, 1, item);

            dbgPop();
        }

        dbgPop();

        dbgPrintf("Calculating Projections Table");
        dbgPush();

        auto setResItem = [&](int row, int col, QTableWidgetItem* item) {
            auto* old = _ui->calculationResultTableWidget->item(row, col);
            if(old) item->setToolTip(old->toolTip());
            _ui->calculationResultTableWidget->setItem(row, col, item);
            dbgPrintf("Proj[%d] - %s", row, Q_CSTR(item->data(Qt::EditRole).toString()));
        };

        auto strToFloat = [](float& val, QString input, QString name) {
            bool ok;
            val = input.toFloat(&ok);
            if(!ok) {
                dbgPrintf("%s [%s] could not be converted to a float!", Q_CSTR(name), Q_CSTR(input));
            }
            return ok;
        };

        float currentWeight;
        float averageDailyGain;
        float matureWeight;
        float sellWeight;
        strToFloat(currentWeight, _ui->currentWeightComboBox->currentText(), "Current Weight");
        strToFloat(matureWeight, _ui->matureWeightComboBox->currentText(), "Mature Weight");
        strToFloat(averageDailyGain, _ui->adgComboBox->currentText(), "Average Daily Gain");
        sellWeight = _ui->expectedSellWeightSpinBox->value();

        float lbsToGain             = (sellWeight - currentWeight);
        float daysOnFeed            = (averageDailyGain != 0.0f)? (lbsToGain)/averageDailyGain : INFINITY;
        float weeksOnFeed           = daysOnFeed/7.0f;
        float totalEndCost          = daysOnFeed * totalCostPerDay;
        float costPerLbGained       = (totalEndCost != 0.0f)? lbsToGain / totalEndCost : 0.0f;

        //Total cost
        if(totalHeads > 1) setResItem(0, 0, _createItem("%.2f (%.2f/head)", totalEndCost * totalHeads, totalEndCost));
        else setResItem(0, 0, _createItem("%.2f", totalEndCost));
        //total cost per day
        if(totalHeads > 1) setResItem(1, 0, _createItem("%.2f (%.2f/head)", totalCostPerDay * totalHeads, totalCostPerDay));
        else setResItem(1, 0, _createItem("%.2f", totalCostPerDay));
        //cost per lbs gained
        setResItem(2, 0, _createItem("%.2f", costPerLbGained));
        //average daily gain
        setResItem(3, 0, _createItem("%.2f", averageDailyGain));
        //days on feed
        setResItem(4, 0, _createItem("%.0f", daysOnFeed));
        //weeks on feed
        setResItem(5, 0, _createItem("%.2f", weeksOnFeed));
        //sell date
        QDateTime sellDate = _ui->startDateEdit->dateTime().addDays(daysOnFeed);
        setResItem(6, 0, _createItem("%s", Q_CSTR(sellDate.date().toString())));

        dbgPop();

    }

    _rationTable->popFilter();
    _animalNutritionReqTable->popFilter();
    dbgPop();
}

void MainWindow::_printBuildInfo(void) {
    char strBuff[40];
    const char* platString =
  #if defined(__APPLE__)
    "MacOS";
  #elif defined(_WIN32)
    "Windows";
  #else
    "Linux";
  #endif

    dbgPrintf("Initializing Seed~N~Feed [%s] v%s", platString, VERSION_STRING);
    dbgPush();
#ifdef GYRO_LOCAL_BUILD
    dbgPrintf("THIS IS AN UNOFFICIAL OR LOCAL BUILD!");
#else
    dbgPrintf("THIS IS AN OFFICIAL BUILD!");
#endif
    dbgPrintf("%-20s: %40s", "Git branch name",  STRINGIFY_MACRO(GYRO_BUILD_BRANCH));
    dbgPrintf("%-20s: %40s", "Git commit ID",    STRINGIFY_MACRO(GYRO_BUILD_COMMIT));
    dbgPrintf("%-20s: %40s", "Build number",     STRINGIFY_MACRO(GYRO_BUILD_NUMBER));
    dbgPrintf("%-20s: %40s", "Build node",       STRINGIFY_MACRO(GYRO_BUILD_NODE));
    sprintf(strBuff, "%s %s", __DATE__, __TIME__);
    dbgPrintf("%-20s: %40s", "Build timestamp", strBuff);
    dbgPop();
}


void MainWindow::setStatusBarWarnings(QStringList list) {
    const auto size = list.size();
    _statusWarnButton->setText(QString::number(size) + QString(" Warnings"));
    _statusWarnButton->setEnabled(size? true : false);
}

void MainWindow::setStatusBarErrors(QStringList list) {
    const auto size = list.size();
    _statusErrorButton->setText(QString::number(size) + QString(" Errors"));
    _statusErrorButton->setEnabled(size? true : false);
}

void MainWindow::_initStatusBar(void) {
    dbgPrintf("MainWindow::_initStatusBar");
    dbgPush();
    _statusWarnButton       = new QToolButton();
    _statusErrorButton      = new QToolButton();
    _statusWarnButton->setContentsMargins(0, 0, 0, 0);
    _statusErrorButton->setContentsMargins(0, 0, 0, 0);
    _statusErrorButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    _statusWarnButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    _statusErrorButton->setText("0 Errors");
    _statusWarnButton->setText("0 Warnings");

    _statusWarnButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning));
    _statusErrorButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical));

    auto* bar = statusBar();
    bar->addWidget(_statusErrorButton);
    bar->addWidget(_statusWarnButton);
    bar->layout()->setSpacing(-1);

    _errorDialog = new ErrorWarningDialog("Nutritional Totals");
    _errorDialog->setDescriptionText("The nutritional totals could not be calculated for the given animal and rations due to the following errors.");
    _warnDialog = new ErrorWarningDialog("Projections");
    _warnDialog->setDescriptionText("The calculated/advanced statistics could not be calculated for the given animal and rations due to the following issues.");

    connect(_statusWarnButton, &QToolButton::clicked, [&](bool) {
        if(_warningList.size()) {
            _warnDialog->show();
        }
    });

    connect(_statusErrorButton, &QToolButton::clicked, [&](bool) {
        if(_errorList.size()) {
            _errorDialog->setEntryList(_errorList);
            _errorDialog->show();
        }

    });

    dbgPop();
}

void MainWindow::on_actionExport_triggered(void) {
    dbgPrintf("MainWindow::onActionExportTriggered");
    dbgPush();
    if(_dbPath.isNull()) {
        QMessageBox::critical(this, "Export Failed", "No existing database found to export.");
        dbgPop();
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Save File", QDir::current().absolutePath(), "Database Files (*.db)");

    if(fileName.isNull() || fileName.isEmpty()) {
        dbgPop();
        return;
    }


    dbgPrintf("Exporting database file to %s", Q_CSTR(fileName));
    _ingredientsTable->protectedDbCommit();

    if(!QFile::copy(_dbPath, fileName)) {
        dbgPrintf("Copy failed!");
        QMessageBox::critical(this, "Export Failed", "Could not create new file for export!");
    }
    dbgPop();
}

bool MainWindow::_importDb(QString filepath) {
    dbgPrintf("MainWindow::onActionExportTriggered");
    dbgPush();
    if(!_dbPath.isNull() && !_dbPath.isEmpty()) {
        int ret = QMessageBox::warning(this, "Warning",
                                        "Importing the selected database will overwrite the current database. Continue?",
                                       QMessageBox::Yes | QMessageBox::Cancel);

        if(ret == QMessageBox::Cancel) {
            dbgPop();
            return false;
        }
    }

    dbgPrintf("Importing database file: %s", Q_CSTR(filepath));
    _db.close();

    if(!QDir::current().remove(_dbPath)) {
        dbgPrintf("Unable to remove current database file!");
        dbgPop();
        return false;
    }

    if(!QFile::copy(filepath, _dbPath)) {
        dbgPrintf("Copy failed!");
        dbgPop();
        return false;
    }

    QFile file(_dbPath);

    bool success = true;

    if(!file.setPermissions(QFile::WriteUser|QFile::ReadUser|QFile::ReadOwner|QFile::WriteOwner)) {
        dbgPrintf("Failed to set permissions on new DB file!");
        success = false;
    }
\
    success &= _dbInit();
    success &= _tableInit();
    dbgPop();
    return success;
}

void MainWindow::on_actionImport_triggered(void) {
    dbgPrintf("MainWindow::onActionImportTriggered");
    dbgPush();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    QDir::current().absolutePath(),
                                                    "Database Files (*.db)");

    if(fileName.isNull() || fileName.isEmpty()) {
        dbgPop();
        return;
    }

    _importDb(fileName);
    dbgPop();
}

void MainWindow::onAddRecipeClick(bool) {
    dbgPrintf("MainWindow::onAddRecipeClick");
    dbgPush();
    QString newName = _recipeTable->makeColumnDataUnique(RecipeTable::COL_NAME, "New_Recipe_");
    if(_recipeTable->appendNewRow({
                                   {RecipeTable::COL_NAME, newName}
    })) {
        _ui->recipeTableView->selectRow(_recipeTable->rowCount()-1);
    }
    dbgPop();
}


void MainWindow::onDeleteRecipeClick(bool) {
    dbgPrintf("MainWindow::onDeleteRecipeClick");
    dbgPush();
    // check which row(s) is/are selected (if any)
    QItemSelectionModel *select = _ui->recipeTableView->selectionModel();
    QModelIndexList selectedRows = select->selectedRows();
    int lowestRow = _recipeTable->rowCount()-1;

    if(selectedRows.size()) {
        for(auto it: selectedRows) {
            if(it.row() < lowestRow) lowestRow = it.row();
        }

        for(auto sel : selectedRows) {
            auto idxList = _rationTable->findRowsWithColumnValue(RationTable::COL_RECIPE, _recipeTable->data(_recipeTable->index(sel.row(), RecipeTable::COL_NAME)));
            _rationTable->deleteRows(idxList);
        }
        if(_recipeTable->deleteRows(selectedRows) && _recipeTable->rowCount()) {
            if(lowestRow >= 0 && lowestRow < _recipeTable->rowCount()) {
                _ui->recipeTableView->selectRow(lowestRow);
            } else _ui->recipeTableView->selectRow(0);

        }

    } else {
        QMessageBox::critical(this, "Delete from RecipeTable Failed", "Please select at least one entire row for deletion.");
    }
    dbgPop();
}

void MainWindow::onRecipeSelectionChanged(const QModelIndex& selected, const QModelIndex&) {
    dbgPrintf("MainWindow::onRecipeSelectionChanged");
    dbgPush();
    //_ui->rationTableView->setEnabled(selected.isValid());
    _ui->rationTableView->setModel(_rationTable);
    _rationTable->insertHeaderData();
    _ui->rationTableView->setColumnHidden(RationTable::COL_RECIPE, true);

    if(selected.isValid()) {

        QString recipeName = selected.data().toString();
        _rationTable->setFilter("recipeName = '" + recipeName + "'");
        dbgPrintf("Selecting Recipe [%s]", Q_CSTR(recipeName));
        _rationTable->protectedSelect();

    }
    dbgPop();
}


//CHECK FOR RENAME
void MainWindow::onRecipeValueChanged(int row, int, QVariant, QVariant, int) {
    dbgPrintf("MainWindow::onRecipeValueChanged");
    dbgPush();
    if(_ui->recipeTableView->selectionModel()->hasSelection()) {
        auto selectedList = _ui->recipeTableView->selectionModel()->selectedRows();

        for(auto& index : selectedList) {
            if(index.row() == row) {
                _ui->recipeTableView->selectRow(row);
                onRecipeSelectionChanged(index, QModelIndex());
                break;
            }
        }
    }
    dbgPop();
}


void MainWindow::onAddNutrientClick(bool) {
    dbgPrintf("MainWindow::onAddNutrientClick");
    dbgPush();
    QString newName = _nutrientTable->makeColumnDataUnique(NutrientTable::COL_NAME, "New_Nutrient_");
    _nutrientTable->appendNewRow({
                                     {NutrientTable::COL_NAME, newName}
                                 });
    dbgPop();

}

void MainWindow::onDeleteNutrientClick(bool) {
    dbgPrintf("MainWindow::onDeleteNutrientClick");
    dbgPush();
    // check which row(s) is/are selected (if any)
    QItemSelectionModel *select = _ui->nutrientsTableView->selectionModel();

    QModelIndexList selectedRows = select->selectedRows();
    if(selectedRows.size()) {
        QStringList names;
        for(auto it: selectedRows) {
            names << it.data().toString();
        }
        for(auto it: names) {
            _ingredientsTable->dropSqlColumn(it);
            _animalNutritionReqTable->dropSqlColumn(it);
        }
        _nutrientTable->deleteRows(selectedRows);
    } else {
        QMessageBox::critical(this, "Delete from NutrientTable Failed", "Please select at least one entire row for deletion.");
    }
    dbgPop();
}


void MainWindow::onAnimalComboBoxChange(QString text) {
    dbgPrintf("MainWindow::onAnimalComboBoxChange");
    dbgPush();
    if(!text.isNull() && !text.isEmpty()) {
        int row = _ui->animalComboBox->currentIndex();
        if(row < _animalTable->rowCount()) {
            auto req = AnimalNutritionReq();//_animalNutritionReqTable->nutritionReqFromRow(row);
      //      _ui->currentBodyWeightSpinBox->setValue(req.weight);
            _ui->expectedSellWeightSpinBox->setMinimum(req.currentWeight+1.0f);
            _ui->matureWeightComboBox->setEnabled(true);
        }
    }
    _ui->matureWeightComboBox->populate();
    _ui->currentWeightComboBox->populate();
    _ui->adgComboBox->populate();
   // _ui->expectedSellWeightSpinBox->clear();
    _updateCalculationTabWidgets();
    dbgPop();
}

void MainWindow::on_actionUse_Default_triggered() {
    dbgPrintf("MainWindow::onActionUseDefaultTriggered");
    dbgPush();
    _importDb(DEFAULT_DB_PATH);
    dbgPop();
}

void MainWindow::on_actionAbout_triggered() {
    QMessageBox::about(this, "About ElysianVMU",
        QString("<table><tr><td width='30%'><img src=':/feedIco.png' width='64' height='64'></td>") +
        QString("<td>Seed~N~Feed<br>") +
        QString("Version ")+QString(VERSION_STRING)+QString("<br>") +
        QString("<br>A Girgis Family Production") +
        QString("</td></tr></table>") +
        QString("<br><br>Seed~N~Feed is a cross-platform ration and nutrition calculator for beef cattle.<br>")+
        QString("<table><tr><td>Project Manager: </td><td>Ayman Girgis</td></tr>") +
        QString("<tr><td>Software: </td><td> Falco Girgis, Lauren Girgis</td></tr>") +
        QString("<tr><td>Logo: </td><td>Patrick Kowalik</td></tr></table><br>") +
        QString("</table><br><br>") +
        QString("<i>Copyright 2017, 2018 A&G Enterprises</i><br>"));
}

void MainWindow::on_addAnimalTypeButton_clicked()
{
    dbgPrintf("MainWindow::onAddAnimalTypeButtonClicked");
    dbgPush();
    QString newName = _animalTable->makeColumnDataUnique(AnimalTable::COL_NAME, "New_Animal_");
    if(_animalTable->appendNewRow({
                                   {AnimalTable::COL_NAME, newName}
    })) {
        _ui->animalTableView->selectRow(_animalTable->rowCount()-1);
    }
    dbgPop();
}

void MainWindow::on_deleteAnimalTypeButton_clicked()
{
    dbgPrintf("MainWindow::onDeleteAnimalTypeButtonClicked");
    dbgPush();
    // check which row(s) is/are selected (if any)
    QItemSelectionModel *select = _ui->animalTableView->selectionModel();
    QModelIndexList selectedRows = select->selectedRows();

    int lowestRow = _animalTable->rowCount()-1;

    if(selectedRows.size()) {
        for(auto it: selectedRows) {
            if(it.row() < lowestRow) lowestRow = it.row();
        }

        for(auto sel : selectedRows) {
            auto idxList = _animalNutritionReqTable->findRowsWithColumnValue(AnimalNutritionReqTable::COL_ANIMAL, _animalTable->data(_animalTable->index(sel.row(), AnimalTable::COL_NAME)));
            _animalNutritionReqTable->deleteRows(idxList);
        }

        if(_animalTable->deleteRows(selectedRows) && _animalTable->rowCount()) {
            if(lowestRow >= 0 && lowestRow < _animalTable->rowCount()) {
                _ui->animalTableView->selectRow(lowestRow);
            } else _ui->animalTableView->selectRow(0);
        }

    } else {
        QMessageBox::critical(this, "Delete from AnimalTable Failed", "Please select at least one entire row for deletion.");
    }
    dbgPop();
}


void MainWindow::onAnimalSelectionChanged(const QModelIndex& selected, const QModelIndex&) {
    dbgPrintf("MainWindow::onAnimalSelectionChanged");
    dbgPush();
    _ui->animalNutritionReqTableView->setModel(_animalNutritionReqTable);
    _animalNutritionReqTable->insertHeaderData();
    _ui->animalNutritionReqTableView->setColumnHidden(AnimalNutritionReqTable::COL_ANIMAL, true);

    if(selected.isValid()) {

        QString animalName = selected.data().toString();
        _animalNutritionReqTable->setFilter("animal = '" + animalName + "'");
        dbgPrintf("Selecting Animal [%s]", Q_CSTR(animalName));
        _animalNutritionReqTable->protectedSelect();

    }
    dbgPop();
}

//CHECK FOR RENAME
void MainWindow::onAnimalValueChanged(int row, int, QVariant, QVariant, int) {
    dbgPrintf("MainWindow::onAnimalValueChanged");
    dbgPush();
    if(_ui->animalTableView->selectionModel()->hasSelection()) {
        auto selectedList = _ui->animalTableView->selectionModel()->selectedRows();
        for(auto& index : selectedList) {
            if(index.row() == row) {
                _ui->animalTableView->selectRow(row);
                onAnimalSelectionChanged(index, QModelIndex());
                break;
            }
        }
    }
    dbgPop();
}

void MainWindow::onMatureWeightComboBoxChange(QString text) {
    dbgPrintf("MainWindow::onMatureWeightComboBoxChange");
    dbgPush();
    if(!text.isNull() && !text.isEmpty()) {
            _ui->currentWeightComboBox->setEnabled(true);

    } else {
        _ui->currentWeightComboBox->setEnabled(false);
    }
    _ui->currentWeightComboBox->populate();
    _ui->adgComboBox->populate();
   // _ui->expectedSellWeightSpinBox->clear();
    _updateCalculationTabWidgets();
    dbgPop();
}

void MainWindow::onCurrentWeightComboBoxChange(QString) {
    dbgPrintf("MainWindow::onCurrentWeightComboBoxChange");
    dbgPush();
    _ui->adgComboBox->clear();
   // _ui->expectedSellWeightSpinBox->clear();
    _updateCalculationTabWidgets();
    _ui->adgComboBox->populate();
    dbgPop();
}

void MainWindow::onAverageDailyGainComboBoxChange(QString) {
    _updateCalculationTabWidgets();
}

void MainWindow::onRecipeComboBoxChange(QString) {
    _updateCalculationTabWidgets();

}

int MainWindow::_getCalcAnimalRow(void) const {
    return _animalNutritionReqTable->findRowsWithColumnValue(AnimalNutritionReqTable::COL_ANIMAL, _ui->animalComboBox->currentText()).first().row();
}

int MainWindow::_getCalcRecipeRow(void) const {
    return _recipeTable->findRowsWithColumnValue(RecipeTable::COL_NAME, _ui->recipeComboBox->currentText()).first().row();
}

int MainWindow::_getCalcNutritionReqsRow(void) const {
    int row = _getCalcAnimalRow();
    if(row == -1) return -1;
    auto  rows = _animalNutritionReqTable->findRowsWithColumnValue(AnimalNutritionReqTable::COL_ANIMAL, _ui->animalComboBox->currentText());
    for(auto it: rows) {
        auto entry = _animalNutritionReqTable->getTableRowEntry(it.row());
       if(entry.getColumnValue<QString>(AnimalNutritionReqTable::COL_ANIMAL)              == _ui->animalComboBox->currentText()
             && entry.getColumnVariant(AnimalNutritionReqTable::COL_WEIGHT_MATURE).toString()   == _ui->matureWeightComboBox->currentText()
               && entry.getColumnVariant(AnimalNutritionReqTable::COL_WEIGHT_CURRENT).toString()   == _ui->currentWeightComboBox->currentText()){
           return it.row();

       }
    }
    return -1;
}

void MainWindow::_updateCalculationTabWidgets(void) {
    _ui->matureWeightComboBox->setEnabled(false);
    _ui->currentWeightComboBox->setEnabled(false);
    _ui->adgComboBox->setEnabled(false);
    _ui->calculatePushButton->setEnabled(false);
    _ui->expectedSellWeightSpinBox->setEnabled(false);

    if(!_ui->animalComboBox->currentText().isNull() && !_ui->animalComboBox->currentText().isEmpty()) {
        _ui->matureWeightComboBox->setEnabled(true);

        if(!_ui->matureWeightComboBox->currentText().isNull() && !_ui->matureWeightComboBox->currentText().isEmpty()) {
            _ui->currentWeightComboBox->setEnabled(true);

            if(!_ui->currentWeightComboBox->currentText().isNull() && !_ui->currentWeightComboBox->currentText().isEmpty()) {
                _ui->adgComboBox->setEnabled(true);

                bool minOk, maxOk;
                const double min = _ui->currentWeightComboBox->currentText().toDouble(&minOk);
                const double max = _ui->matureWeightComboBox->currentText().toDouble(&maxOk);
                if(minOk && maxOk && max < min) {
                    dbgPrintf("Attempting to set currentWeightComboBox Limits, impossible values! [Min: %lf, Max: %lf]", min, max);
                    minOk = maxOk = false;
                }

                if(minOk) _ui->expectedSellWeightSpinBox->setMinimum(min + 1.0f);
                if(maxOk) _ui->expectedSellWeightSpinBox->setMaximum(max);

                if(minOk && maxOk) {
                    _ui->expectedSellWeightSpinBox->setEnabled(true);

                    if(!_ui->adgComboBox->currentText().isNull() && !_ui->adgComboBox->currentText().isEmpty()) {

                        if(!_ui->recipeComboBox->currentText().isNull() && !_ui->recipeComboBox->currentText().isEmpty()) {

                            _ui->calculatePushButton->setEnabled(true);

                        }
                    }
                }
            }
        }
    }
}

void MainWindow::onMainTabChange(int index) {
    dbgPrintf("MainWindow::onMainTabChange - %d", index);
    dbgPush();
    switch(index) {
    case MAIN_TAB_ANIMALS:
        if(!_ui->animalTableView->selectionModel()->hasSelection() && _animalTable->rowCount()) {
            _ui->animalTableView->selectRow(0);
        }
        break;
    case MAIN_TAB_RECIPES:
        if(!_ui->recipeTableView->selectionModel()->hasSelection() && _recipeTable->rowCount()) {
            _ui->recipeTableView->selectRow(0);
        }
        break;
    case MAIN_TAB_CALC:
        /*Protect data from getting out of sync when tables are changed in other tabs
          because I'm too lazy to dynamically update them.
          */
        _ui->animalComboBox->populate();
        _ui->recipeComboBox->populate();
        _ui->matureWeightComboBox->populate();
        _ui->currentWeightComboBox->populate();
        _ui->adgComboBox->populate();
        break;
    default: break;

    }
    dbgPop();

}


