#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QMessageBox>
#include <QFileInfo>
#include <QStringListModel>
#include <QToolButton>
#include <QDebug>
#include <QtMath>
#include <QFileDialog>
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
#include "delegate/rations_table_delegate.h"
#include "ui/error_warning_dialog.h"
#include "delegate/ingredients_table_delegate.h"
#include "delegate/animal_nutrition_table_delegate.h"
#include "delegate/totals_table_delegate.h"
#include "model/sql_table.h"
#include "delegate/nutrients_table_delegate.h"
#include "delegate/recipe_table_delegate.h"

#define SPRINTF_TEMP_BUFF_SIZE  2048

//===== STATIC =====

FILE* MainWindow::_dbgLogFile = nullptr;

void MainWindow::_writeLog(const char* str) {
    if(_dbgLogFile) {
        fprintf(_dbgLogFile, "%s\n", str);
        fflush(_dbgLogFile);
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

    _ui->nutritionalTotalsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _ui->calculationResultTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _ui->animalNutritionReqTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _ui->ingredientsTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _ui->rationTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _ui->recipeTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _ui->nutrientsTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

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
    delete _nutrientTable;
    delete _ingredientsTable;
    delete _recipeTable;
    delete _rationTable;
    delete _animalNutritionReqTable;

    _nutrientTable              = new NutrientTable(this, _db);
    _ingredientsTable           = new IngredientsTable(this, _db);
    _recipeTable                = new RecipeTable(this, _db);
    _rationTable                = new RationTable(this, _db);
    _animalNutritionReqTable    = new AnimalNutritionReqTable(this, _db);

    _nutrientTable->setTable("Nutrients");
    _nutrientTable->select();
    _nutrientTable->insertHeaderData();
    _nutrientTable->setIngredientsTable(_ingredientsTable);
    _ui->nutrientsTableView->setModel(_nutrientTable);


    _ingredientsTable->setTable("Ingredients");
    _ingredientsTable->select();
    _ingredientsTable->setNutrientTable(_nutrientTable);
    _ingredientsTable->insertHeaderData();
    _ui->ingredientsTableView->setModel(_ingredientsTable);


    _recipeTable->setTable("Recipes");
    _recipeTable->select();
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


    _animalNutritionReqTable->setTable("AnimalNutritionReq");
    _animalNutritionReqTable->select();
    _animalNutritionReqTable->insertHeaderData();
    _ui->animalNutritionReqTableView->setModel(_animalNutritionReqTable);

    _ui->animalComboBox->setSrcModelColumn(_animalNutritionReqTable, AnimalNutritionReqTable::COL_DESC);
    _ui->animalComboBox->populate();
    onAnimalComboBoxChange("FUCKING UPDATE!!!");
    _ui->recipeComboBox->setSrcModelColumn(_recipeTable, RecipeTable::COL_NAME);
    _ui->recipeComboBox->populate();

    connect(_recipeTable, SIGNAL(cellDataChanged(int,int,QVariant,QVariant,int)), this, SLOT(onRecipeValueChanged(int,int,QVariant,QVariant,int)));
    connect(_ui->recipeTableView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex&,const QModelIndex&)), this, SLOT(onRecipeSelectionChanged(const QModelIndex&,const QModelIndex&)));

    return true;
}

bool MainWindow::_dbInit(QString dbType) {
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

    dbgPrintf("Setting DB connection type: %s", Q_CSTR(dbType));
    _db = QSqlDatabase::addDatabase(dbType);
    _db.setDatabaseName(dbPath);

    if(!_db.open()) {
        qCritical() << "Database connection failed: " << dbPath << _db.lastError();
        success = false;
        _dbPath.clear();
    } else {
        dbgPrintf("Successfully connected to database!");
        _dbPath = dbPath;
    }

    if(/*createNewDb || */!_db.tables().contains("Ingredients")) {
        QSqlQuery query;
        if(!query.exec("create table Ingredients"
                  "(name varchar(" STRINGIFY_MACRO(INGREDIENT_NAME_SIZE) ") primary key, "
                  "dm real, "
                  "nem real,"
                  "neg real,"
                   "protein real,"
                   "ca real,"
                   "p real,"
                   "vita real)"))
        {
            qCritical() << "Create Ingredients table query failed: " << query.lastError();
            success = false;
        } else {
            dbgPrintf("Create Ingredients table query succeeded!");
        }
    }


    if(/*createNewDb || */!_db.tables().contains("Recipes")) {
        QSqlQuery query;
        if(!query.exec("create table Recipes"
                  "(name varchar(" STRINGIFY_MACRO(RECIPE_NAME_SIZE) ") primary key "
                       ")"))
        {
            qCritical() << "Create Recipes table query failed: " << query.lastError();
            success = false;
        } else {
            dbgPrintf("Create Recipes table query succeeded!");
        }
    }

    if(/*createNewDb || */!_db.tables().contains("Nutrients")) {
        QSqlQuery query;
        if(!query.exec("create table Nutrients"
                  "(name varchar(" STRINGIFY_MACRO(NUTRIENT_NAME_SIZE) ") primary key "
                       ")"))
        {
            qCritical() << "Create Nutrients table query failed: " << query.lastError();
            success = false;
        } else {
            dbgPrintf("Create Nutrients table query succeeded!");
        }
    }

    if(/*createNewDb || */!_db.tables().contains("Rations")) {
        QSqlQuery query;
        if(!query.exec("create table Rations"
                  "(recipeName references Recipes(name),"
                       "ingredientName references Ingredients(name), "
                       "asfed real,"
                       "cost real,"
                       "weight real,"
                       "costPerDay real,"
                       "dm real"
        ")"))
        {
            qCritical() << "Create Rations table query failed: " << query.lastError();
            success = false;
        } else {
            dbgPrintf("Create Rations table query succeeded!");
        }
    }


    if(/*createNewDb || */!_db.tables().contains("AnimalNutritionReq")) {
        QSqlQuery query;
        if(!query.exec("create table AnimalNutritionReq"
                  "(animaldescription varchar(" STRINGIFY_MACRO(ANIMAL_NUTRITION_REQ_DESC_SIZE) ") primary key, "
                  "type int,"
                  "weight real,"
                  "dailygain real,"
                  "dmi real,"
                  "nem real,"
                  "neg real,"
                  "protein real,"
                  "calcium real,"
                  "phosphorus real,"
                  "vita real)"))
        {
            qCritical() << "Create AnimalNutritionReq table query failed: " << query.lastError();
            success = false;
        } else {
            dbgPrintf("Create AnimalNutritionReq table query succeeded!");
        }
    }

    return success;
}

void MainWindow::closeEvent(QCloseEvent*/*event*/) {
    bool needsCommit = false;
    dbgPrintf("Submitting any pending changes to DB on before exitting.");
    needsCommit |= _ingredientsTable->submitAll();
    needsCommit |= _animalNutritionReqTable->submitAll();
    needsCommit |= _nutrientTable->submitAll();
    needsCommit |= _rationTable->submitAll();
    needsCommit |= _recipeTable->submitAll();

    if(needsCommit) _db.commit();
}

void MainWindow::onAddIngredientClick(bool) {
    _ingredientsTable->insertRows(_ingredientsTable->rowCount(), 1);
    if(_ingredientsTable->submitAll()) {
        _ingredientsTable->database().commit();
    }
}

void MainWindow::onAddAnimalNutritionReqClick(bool) {
    _animalNutritionReqTable->insertRows(_animalNutritionReqTable->rowCount(), 1);
    if(_animalNutritionReqTable->submitAll()) {
        _animalNutritionReqTable->database().commit();
    }
}

void MainWindow::onDeleteIngredientClick(bool) {
    // check which row(s) is/are selected (if any)
    QItemSelectionModel *select = _ui->ingredientsTableView->selectionModel();

    QModelIndexList selectedRows = select->selectedRows();
    if(selectedRows.size()) {

        for(auto it: selectedRows) _ingredientsTable->removeRows(it.row(), 1);
        if(_ingredientsTable->submitAll()) {
            _ingredientsTable->database().commit();
        }
        _ingredientsTable->select();

    } else {
        QMessageBox::critical(this, "Delete from Ingredients Table Failed", "Please select at least one entire row for deletion.");

    }
}

void MainWindow::onDeleteAnimalNutritionReqClick(bool) {
    // check which row(s) is/are selected (if any)
    QItemSelectionModel *select = _ui->animalNutritionReqTableView->selectionModel();

    QModelIndexList selectedRows = select->selectedRows();
    if(selectedRows.size()) {

        for(auto it: selectedRows) _animalNutritionReqTable->removeRows(it.row(), 1);
        if(_animalNutritionReqTable->submitAll()) {
            _animalNutritionReqTable->database().commit();
        }
        _animalNutritionReqTable->select();

    } else {
        QMessageBox::critical(this, "Delete from AnimalNutritionReqTable Failed", "Please select at least one entire row for deletion.");

    }
}

void MainWindow::onAddRationClick(bool) {
#if 0
    _rationTable->insertRows(_rationTable->rowCount(), 1);
        if(_rationTable->submitAll()) {
            _rationTable->database().commit();
    }
       // _rationTable->setFilter("recipeName = '" + recipeName + "'");
        //_rationTable->select();
#else
    auto selectedIndices = _ui->recipeTableView->selectionModel()->selectedRows();
    //Q_ASSERT(selectedIndices.size() == 1);
    if(selectedIndices.size() >= 1) {
        auto selected = selectedIndices[0];

        if(selected.isValid()) {
            QString recipeName = selected.data().toString();
            qDebug() << "SELECTING " << recipeName;
            //_rationTable->setFilter("recipeName = '" + recipeName + "'");
            if(_rationTable->tryAppendRow()) {
                if(_rationTable->submitAll()) {
                    _rationTable->database().commit();
                }
                const auto& newIndex = _rationTable->index(_rationTable->rowCount()-1, RationTable::COL_RECIPE);
                Q_ASSERT(newIndex.isValid());
                _rationTable->setData(newIndex, recipeName);
                _ui->rationTableView->scrollToBottom();

                //_rationTable->setFilter("recipeName = '" + recipeName + "'");
               // _rationTable->select();
            } //else Q_ASSERT(false);
        } else Q_ASSERT(false);
    }
#endif
}

void MainWindow::onDeleteRationClick(bool) {
#if 0
    // check which row(s) is/are selected (if any)
    QItemSelectionModel* select = _ui->rationTableView->selectionModel();

    QModelIndexList selectedRows = select->selectedRows();
    if(selectedRows.size()) {
        for(auto it: selectedRows) _rationTable->removeRows(it.row(), 1);
    } else {
        QMessageBox::critical(this, "Delete Fail8ed", "Please select at least one entire row for deletion.");

    }
#else
    // check which row(s) is/are selected (if any)
    QItemSelectionModel *select = _ui->rationTableView->selectionModel();
    if(!select) return;

    QModelIndexList selectedRows = select->selectedRows();
    if(selectedRows.size()) {

        for(auto it: selectedRows) _rationTable->removeRows(it.row(), 1);
        if(_rationTable->submitAll()) {
            _rationTable->database().commit();
        }
        _rationTable->select();

    } else {
        QMessageBox::critical(this, "Delete from Rations Table Failed", "Please select at least one entire row for deletion.");

    }
#endif
}

void MainWindow::_validateCalculationInputs(void) {
    QStringList validatedIngredients;

    int     rows            = _rationTable->rowCount();

    _errorList.clear();
    _warningList.clear();


    if(!rows) {
        _errorList += QString("No rations provided!\n");
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

    const int reqRow = _animalNutritionReqTable->rowFromDesc(_ui->animalComboBox->currentText());
    if(reqRow == -1) {
        _warningList += QString("No animal requirements selected!\n");
    } else {
        AnimalNutritionReq animalReq = _animalNutritionReqTable->nutritionReqFromRow(reqRow);
        animalReq.validate(_warningList);

        float finishedWeight  = _ui->expectedSellWeightSpinBox->value();

        if(animalReq.weightValid && finishedWeight <= animalReq.weight) {
            _warningList += QString("'Expected Sell Weight' (lbs) should be greater than 'Current Weight' (lbs)!\n");
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
    for(int i = rows-1; i >= 6; --i) {
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

void MainWindow::onCalculateButtonClick(bool) {
    //validate selected RECIPE
    //validate selected ANIMAL type

    _clearTotalsTable();
    _clearCalculationTable();


    _rationTable->pushFilter("recipeName = '" + _ui->recipeComboBox->currentText() + "'");

    _validateCalculationInputs();
    _updateTotalsTableRows();

    if(_errorList.size()) {
        _ui->nutritionalTotalsTableWidget->setEnabled(false);
    } else {
        _ui->nutritionalTotalsTableWidget->setEnabled(true);

        int rows = _rationTable->rowCount();
        float totalNem = 0, totalNeg = 0, totalProtein = 0, totalCa = 0, totalPhosphorus = 0, totalDm = 0;
        const unsigned nutrientCount = _nutrientTable->rowCount();
        float totalNutrients[nutrientCount];
        for(unsigned i = 0; i < nutrientCount; ++i) totalNutrients[i] = 0.0f;


        qDebug() << "===== PERFORMING CALCULATIONS ======";
        qDebug() << "ROWCOUNT - " << rows;
        for (int i = 0; i < rows; i++) {
            qDebug() << "ROW INDEX - " << i;
            Ration ration = _rationTable->rationFromRow(i);
            qDebug() << "INGREDIENT - " << ration.ingredient;
            int ingredientRow = _ingredientsTable->rowFromName(ration.ingredient);

            Q_ASSERT(ingredientRow != -1);
            if (ingredientRow != -1) {
                Ingredient ingredient = _ingredientsTable->ingredientFromRow(ingredientRow);
                float dryMatter = ingredient.dm/100.0f * ration.asFed;
                totalDm         += dryMatter;
                totalNem        += ingredient.nem               * dryMatter;
                totalNeg        += ingredient.neg               * dryMatter;
                totalProtein    += ingredient.protein/100.0f    * dryMatter;
                totalCa         += ingredient.ca/100.0f         * dryMatter;
                totalPhosphorus += ingredient.p/100.0f          * dryMatter;
                for(unsigned j = 0; j < ingredient.getNutrientCount(); ++j) {
                    if(ingredient.isNutrientValid(j))
                        totalNutrients[j] += ingredient.getNutrientValue(j) * ration.asFed;
                }
            }
        }

        char totalNemString[200], totalNegString[200], totalProteinString[200], totalCaString[200], totalPhosphorusString[200], totalDmString[200];

        sprintf(totalDmString,          "%.2f", totalDm);
        sprintf(totalNemString,         "%.2f", totalNem);
        sprintf(totalNegString,         "%.2f", totalNeg);
        sprintf(totalProteinString,     "%.2f", totalProtein);
        sprintf(totalCaString,          "%.2f", totalCa);
        sprintf(totalPhosphorusString,  "%.2f", totalPhosphorus);

        auto createItem = [](QString label) {
            QTableWidgetItem* item = new QTableWidgetItem(label);
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            return item;
        };

        _ui->nutritionalTotalsTableWidget->setItem(0, 0, createItem(totalDmString));
        _ui->nutritionalTotalsTableWidget->setItem(1, 0, createItem(totalNemString));
        _ui->nutritionalTotalsTableWidget->setItem(2, 0, createItem(totalNegString));
        _ui->nutritionalTotalsTableWidget->setItem(3, 0, createItem(totalProteinString));
        _ui->nutritionalTotalsTableWidget->setItem(4, 0, createItem(totalCaString));
        _ui->nutritionalTotalsTableWidget->setItem(5, 0, createItem(totalPhosphorusString));

        for(int j = 0; j < nutrientCount; ++j) {
            char buff[200];
            sprintf(buff, "%.2f", totalNutrients[j]);
            _ui->nutritionalTotalsTableWidget->setItem(6+j, 0, createItem(buff));

        }

        AnimalNutritionReq req = _animalNutritionReqTable->nutritionReqFromRow(_animalNutritionReqTable->rowFromDesc(_ui->animalComboBox->currentText()));

        float sum[6] = {
            totalDm, totalNem, totalNeg, totalProtein, totalCa, totalPhosphorus
        };

        float expected[6] = {
            req.dmi, req.dmi*req.nem, req.dmi*req.neg, req.dmi*req.protein/100.0f, req.dmi*req.calcium/100.0f, req.dmi*req.phosphorus/100.0f
        };


        for(unsigned i = 0; i < 6; ++i) {
            char strBuff[50];
            float diff = fabs(expected[i]-sum[i]);
            QTableWidgetItem* item = new QTableWidgetItem();//QString::number(expected[i]) + QString(" (")+QString::number(sum[i]/expected[i]*100.0f)+QString(")"));
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            sprintf(strBuff, "%.2f (%.1f%%)", expected[i], sum[i]/expected[i]*100.0f);
            item->setData(Qt::EditRole, QString(strBuff));

            if(diff <= expected[i]*TOTALS_TABLE_DIFF_GREEN_RANGE) {
                item->setData(Qt::BackgroundRole, QColor(Qt::green));
            } else if(diff <= expected[i]*TOTALS_TABLE_DIFF_YELLOW_RANGE) {
                item->setData(Qt::BackgroundRole, QColor(Qt::yellow));
            } else {
                item->setData(Qt::BackgroundRole, QColor(Qt::red));
            }
            _ui->nutritionalTotalsTableWidget->setItem(i, 1, item);
        }


        if(req.type < ANIMAL_TYPE_CUSTOM && !_warningList.size()) {
            auto& energyData = AnimalNutritionReqTable::builtinEnergyTable[req.type];
            _ui->calculationResultTableWidget->setEnabled(true);

            //Compute average weight gained per day (lbs)
            float totalFeedPerDay = 0.0;
            float totalCostPerDay = 0.0;
            for (int i = 0; i < rows; i++) {
                Ration ration = _rationTable->rationFromRow(i);
                totalFeedPerDay += ration.asFed;
                totalCostPerDay += ration.costPerDay;
            }

            float maintenanceEnergyFromRation = totalNem;// / totalFeedPerDay;
            float energyAvailForGainingFromRation = totalNeg;// / totalFeedPerDay;

            //standard metric conversion
            //float currentWeightKgs = _ui->currentBodyWeightSpinBox->value() * KG_PER_LB;
            float currentWeightKgs = req.weight*KG_PER_LB;

            float totalHeads = _ui->totalHeadSpinBox->value();

            //same for all cattle (steers, heifers, bulls, cows
            float dailyEnergyReqForCattle = (0.077 * qPow(currentWeightKgs, 0.75));

            float feedRequiredToMaintainWeight = dailyEnergyReqForCattle / maintenanceEnergyFromRation;
            float amountOfFeedLeftForGaining = totalFeedPerDay - feedRequiredToMaintainWeight;
            float netEnergyForGainingFromRemainingFeed = amountOfFeedLeftForGaining * energyAvailForGainingFromRation;

            //different depending on frames of shit
            float liveWeightGainForMedFrameSteerKg = energyData.negCoeff * qPow(netEnergyForGainingFromRemainingFeed, energyData.negExp) * qPow(currentWeightKgs, energyData.weightExp);
            float liveWeightGainForMedFrameSteerlb = liveWeightGainForMedFrameSteerKg / 0.453592;

            char liveWeightGainForMedFrameSteerlbString[200];
            sprintf(liveWeightGainForMedFrameSteerlbString, "%.2f", req.dailyGain);
            _ui->calculationResultTableWidget->setItem(3, 0, createItem(liveWeightGainForMedFrameSteerlbString));
            //END compute and display average weight gained per day (lbs)

            //display total cost per day
            char totalCostPerDayString[200];
            sprintf(totalCostPerDayString, "%.2f", totalCostPerDay*totalHeads);
            _ui->calculationResultTableWidget->setItem(0, 0, createItem(totalCostPerDayString));

            //compute and display cost per pound gained
            //float costPerPoundGained = totalCostPerDay / liveWeightGainForMedFrameSteerlb;

            float costPerPoundGained = (1.0f/req.dailyGain)*totalCostPerDay;
            char costPerPoundGainedString[200];
            sprintf(costPerPoundGainedString, "%.2f", costPerPoundGained*totalHeads);
            _ui->calculationResultTableWidget->setItem(1, 0, createItem(costPerPoundGainedString));

            //compute and display days on feed
            float desiredWeight = _ui->expectedSellWeightSpinBox->value();
            //float numDaysOnFeed = (_ui->expectedSellWeightSpinBox->value() - _ui->currentBodyWeightSpinBox->value())/liveWeightGainForMedFrameSteerlb;
            char numDaysOnFeedString[200];
            float numDaysOnFeed = (desiredWeight - req.weight)/req.dailyGain;
            sprintf(numDaysOnFeedString, "%.2f", numDaysOnFeed);
            _ui->calculationResultTableWidget->setItem(4, 0, createItem(numDaysOnFeedString));

            //compute and display cost per head
            float costPerHead = totalCostPerDay * numDaysOnFeed;
            char costPerHeadString[200];
            sprintf(costPerHeadString, "%.2f", costPerHead);
            _ui->calculationResultTableWidget->setItem(2, 0, createItem(costPerHeadString));

            //compute and display weeks on feed
            float numWeeksOnFeed = numDaysOnFeed / 7.0;
            char numWeeksOnFeedString[200];
            sprintf(numWeeksOnFeedString, "%.2f", numWeeksOnFeed);
            _ui->calculationResultTableWidget->setItem(5, 0, createItem(numWeeksOnFeedString));

            //compute and display sell date
            QDateTime sellDate = _ui->startDateEdit->dateTime().addDays(numDaysOnFeed);
            _ui->calculationResultTableWidget->setItem(6, 0, createItem(sellDate.date().toString()));        } else {
            _ui->calculationResultTableWidget->setEnabled(false);
        }
    }

    _rationTable->popFilter();
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

    dbgPrintf("Initializing Seed N Feed [%s] v%s", platString, VERSION_STRING);
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


}

void MainWindow::on_actionExport_triggered(void) {
    if(_dbPath.isNull()) {
        QMessageBox::critical(this, "Export Failed", "No existing database found to export.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Save File", QDir::current().absolutePath(), "Database Files (*.db)");

    if(fileName.isNull() || fileName.isEmpty()) return;


    dbgPrintf("Exporting database file to %s", Q_CSTR(fileName));
    if(_ingredientsTable->submitAll()) {
        _db.commit();
    }

    if(!QFile::copy(_dbPath, fileName)) {
        dbgPrintf("Copy failed!");
        QMessageBox::critical(this, "Export Failed", "Could not create new file for export!");
    }
}

bool MainWindow::_importDb(QString filepath) {
    if(!_dbPath.isNull() && !_dbPath.isEmpty()) {
        int ret = QMessageBox::warning(this, "Warning",
                                        "Importing the selected database will overwrite the current database. Continue?",
                                       QMessageBox::Yes | QMessageBox::Cancel);

        if(ret == QMessageBox::Cancel) return false;
    }

    dbgPrintf("Importing database file: %s", Q_CSTR(filepath));
    _db.close();

    if(!QDir::current().remove(_dbPath)) {
        dbgPrintf("Unable to remove current database file!");
        return false;
    }

    if(!QFile::copy(filepath, _dbPath)) {
        dbgPrintf("Copy failed!");
        return false;
    }

    QFile file(_dbPath);

    bool success = true;

    if(!file.setPermissions(QFile::WriteUser|QFile::ReadUser|QFile::ReadOwner|QFile::WriteOwner)) {
        qCritical() << "Failed to set permissions on new DB file!\n";
        success = false;
    }
\
    success &= _dbInit();
    success &= _tableInit();
    return success;
}

void MainWindow::on_actionImport_triggered(void) {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    QDir::current().absolutePath(),
                                                    "Database Files (*.db)");

    if(fileName.isNull() || fileName.isEmpty()) return;

    _importDb(fileName);

}

void MainWindow::onAddRecipeClick(bool) {
    _recipeTable->insertRows(_recipeTable->rowCount(), 1);
    if(_recipeTable->submitAll()) {
        _recipeTable->database().commit();
    }

}

void MainWindow::onDeleteRecipeClick(bool) {
    // check which row(s) is/are selected (if any)
    QItemSelectionModel *select = _ui->recipeTableView->selectionModel();

    QModelIndexList selectedRows = select->selectedRows();
    if(selectedRows.size()) {

        for(auto it: selectedRows) _recipeTable->removeRows(it.row(), 1);
        if(_recipeTable->submitAll()) {
            _recipeTable->database().commit();
        }
        _recipeTable->select();

        //_rationTable->setFilter("recipeName=''");
        //_rationTable->select();

    } else {
        QMessageBox::critical(this, "Delete from RecipeTable Failed", "Please select at least one entire row for deletion.");

    }
}

void MainWindow::onRecipeSelectionChanged(const QModelIndex& selected, const QModelIndex& deselected) {
    //_ui->rationTableView->setEnabled(selected.isValid());
    _ui->rationTableView->setModel(_rationTable);
    _rationTable->insertHeaderData();
    _ui->rationTableView->setColumnHidden(RationTable::COL_RECIPE, true);

    if(selected.isValid()) {

        QString recipeName = selected.data().toString();
        _rationTable->setFilter("recipeName = '" + recipeName + "'");
        _rationTable->select();

    } else {
        qDebug() << "INFALIVD SHIT";
    }
}

void MainWindow::onRecipeValueChanged(int row, int col, QVariant oldValue, QVariant newValue, int editRole) {
    auto selectedList = _ui->recipeTableView->selectionModel()->selectedRows();

    for(auto& index : selectedList) {
        if(index.row() == row) {
            _ui->recipeTableView->selectionModel()->clearCurrentIndex();
            _ui->recipeTableView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
            break;
        }
    }
}


void MainWindow::onAddNutrientClick(bool) {
    _nutrientTable->insertRows(_nutrientTable->rowCount(), 1);

    int num = 0;
    char nameBuff[NUTRIENT_NAME_SIZE];
    do {
        sprintf(nameBuff, "New_Nutrient_%d", ++num);
    } while(_nutrientTable->rowFromName(nameBuff) != -1);

    const auto& newIndex = _nutrientTable->index(_nutrientTable->rowCount()-1, NutrientTable::COL_NAME);
    Q_ASSERT(newIndex.isValid());
    _nutrientTable->setData(newIndex, QString(nameBuff));
    if(_nutrientTable->submitAll()) {
        _nutrientTable->database().commit();
    }
    _ingredientsTable->addSqlColumn(nameBuff, "real");

}

void MainWindow::onDeleteNutrientClick(bool) {
    // check which row(s) is/are selected (if any)
    QItemSelectionModel *select = _ui->nutrientsTableView->selectionModel();

    QModelIndexList selectedRows = select->selectedRows();
    if(selectedRows.size()) {

        for(auto it: selectedRows) {
            _ingredientsTable->dropSqlColumn(it.data().toString());
            _nutrientTable->removeRows(it.row(), 1);
        }
        if(_nutrientTable->submitAll()) {
            _nutrientTable->database().commit();
        }
        _nutrientTable->select();

    } else {
        QMessageBox::critical(this, "Delete from NutrientTable Failed", "Please select at least one entire row for deletion.");
    }
}


void MainWindow::onAnimalComboBoxChange(QString text) {
    if(!text.isNull() && !text.isEmpty()) {
        int row = _ui->animalComboBox->currentIndex();
        if(row < _animalNutritionReqTable->rowCount()) {
            auto req = _animalNutritionReqTable->nutritionReqFromRow(row);
            _ui->currentBodyWeightSpinBox->setValue(req.weight);
            _ui->expectedSellWeightSpinBox->setMinimum(req.weight+1.0f);
        }

    }

}

void MainWindow::on_actionUse_Default_triggered() {
    _importDb(DEFAULT_DB_PATH);
}
