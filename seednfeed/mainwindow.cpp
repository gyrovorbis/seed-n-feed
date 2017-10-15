#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QMessageBox>
#include <QFileInfo>
#include <QDebug>
#include <QtMath>
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
#include "animal_nutrition_table_delegate.h"
#include "totals_table_delegate.h"

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
    _printBuildInfo();
    _dbInit();

    ingredientsTable = new IngredientsTable(this, db);
    ingredientsTable->setTable("Ingredients");
    ingredientsTable->select();
    ui->ingredientsTableView->setModel(ingredientsTable);
    ui->ingredientsTableView->setItemDelegate(new IngredientsTableDelegate);
    ingredientsTable->insertHeaderData();

    rationTable = new RationTable(this);
    RationsTableDelegate::setIngredientsTable(ingredientsTable);

    animalNutritionReqTable = new AnimalNutritionReqTable(this, db);
    animalNutritionReqTable->setTable("AnimalNutritionReq");
    animalNutritionReqTable->select();
    ui->animalNutritionReqTableView->setModel(animalNutritionReqTable);
    ui->animalNutritionReqTableView->setItemDelegate(new AnimalNutritionTableDelegate);
    animalNutritionReqTable->insertHeaderData();

    ui->rationCalculatorTableView->setModel(rationTable);
    ui->rationCalculatorTableView->setItemDelegate(new RationsTableDelegate);

    ui->rationCalculatorTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->nutritionalTotalsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->calculationResultTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->animalNutritionReqTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->ingredientsTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->animalComboBox->setModelColumn(animalNutritionReqTable, AnimalNutritionReqTable::COL_DESC);
    ui->animalComboBox->populate();

    ui->startDateEdit->setDate(QDate::currentDate());

    ui->nutritionalTotalsTableWidget->setItemDelegate(new TotalsTableDelegate);

    connect(ui->addIngredientButton, SIGNAL(clicked(bool)), this , SLOT(onAddIngredientClick(bool)));
    connect(ui->deleteIngredientButton, SIGNAL(clicked(bool)), this, SLOT(onDeleteIngredientClick(bool)));
    connect(ui->addRationButton, SIGNAL(clicked(bool)), this, SLOT(onAddRationClick(bool)));
    connect(ui->deleteRationButton, SIGNAL(clicked(bool)), this, SLOT(onDeleteRationClick(bool)));
    connect(ui->addAnimalNutritionReqButton, SIGNAL(clicked(bool)), this , SLOT(onAddAnimalNutritionReqClick(bool)));
    connect(ui->deleteAnimalNutritionReqButton, SIGNAL(clicked(bool)), this, SLOT(onDeleteAnimalNutritionReqClick(bool)));
    connect(ingredientsTable, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(onIngredientsDataChanged(QModelIndex,QModelIndex,QVector<int>)));
    connect(ui->calculatePushButton, SIGNAL(clicked(bool)), this, SLOT(onCalculateButtonClick(bool)));
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
    db = QSqlDatabase::addDatabase(dbType);
    db.setDatabaseName(dbPath);

    if(!db.open()) {
        qCritical() << "Database connection failed: " << dbPath << db.lastError();
        success = false;
    } else {
        dbgPrintf("Successfully connected to database!");
    }

    if(/*createNewDb || */!db.tables().contains("Ingredients")) {
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
            qCritical() << "Create ingredients table query failed: " << query.lastError();
            success = false;
        } else {
            dbgPrintf("Create ingredients table query succeeded!");
        }
    }

    if(/*createNewDb || */!db.tables().contains("AnimalNutritionReq")) {
        QSqlQuery query;
        if(!query.exec("create table AnimalNutritionReq"
                  "(animaldescription varchar(" STRINGIFY_MACRO(ANIMAL_NUTRITION_REQ_DESC_SIZE) ") primary key, "
                  "type int,"
                  "weight real,"
                  "dailygain real,"
                  "protein real,"
                  "nem real,"
                  "neg real,"
                  "calcium real,"
                  "phosphorus real,"
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

bool MainWindow::_validateCalculationInputs(void) {
    QMessageBox msgBox;
    QString     detailedText;
    bool        success = true;
    int         rows = rationTable->rowCount();
    QStringList validatedIngredients;

    float startWeight = ui->currentBodyWeightSpinBox->value();
    float finishedWeight = ui->expectedSellWeightSpinBox->value();
    float slaughterWeight = ui->finishedWeightDoubleSpinBox->value();

    if(startWeight == 0.0f) {
        detailedText += QString("'Current Body Weight' (lbs) should be greater than 0.0!\n");
        success = false;
    }

    if(finishedWeight == 0.0f) {
        detailedText += QString("'Expected Sell Weight' (lbs) should be greater than 0.0!\n");
        success = false;
    } else if(finishedWeight <= startWeight) {
        detailedText += QString("'Expected Sell Weight' (lbs) should be greater than 'Current Weight' (lbs)!\n");
        success = false;
    }

    if(slaughterWeight == 0.0f) {
        detailedText += QString("'Finished (Slaughter) Weight' (lbs) should be greater than 0.0!\n");
        success = false;
    } else if(slaughterWeight < finishedWeight) {
        detailedText += QString("Finished (Slaughter) Weight' (lbs) should be greater than or equal to 'Expected Sell Weight' (lbs)!\n");
        success = false;
    }

    if(!rows) {
        success = false;
        detailedText += QString("No rations provided!\n");
    } else {

        for (int i = 0; i < rows; i++) {
            Ration ration = rationTable->rationFromRow(i);
            success &= ration.validate(detailedText);

            int ingredientRow = ingredientsTable->rowFromName(ration.ingredient);
            if(ingredientRow == -1) {
                detailedText += QString("Ingredient '") + QString(ration.ingredient) + QString("' could not be found!\n");
                success = false;
            } else {
                if(!validatedIngredients.contains(ration.ingredient)) {
                    Ingredient ingredient = ingredientsTable->ingredientFromRow(ingredientRow);
                    success &= ingredient.validate(detailedText);
                    validatedIngredients.push_back(ration.ingredient);
                }
            }
        }
    }

    int reqRow = animalNutritionReqTable->rowFromDesc(ui->animalComboBox->currentText());
    if(reqRow == -1) {
        detailedText += QString("No animal requirements selected!\n");
        success = false;
    } else {
        AnimalNutritionReq animalReq = animalNutritionReqTable->nutritionReqFromRow(reqRow);
        success &= animalReq.validate(detailedText);
    }

    if(!success) {
        msgBox.setText("Failed to calculate data!");
        msgBox.setInformativeText(QString("Missing one or more pieces of input data required to perform the calculations. See details for more information."));
        msgBox.setStandardButtons(QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);

        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setDetailedText(detailedText);
        QSpacerItem* horizontalSpacer = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        QGridLayout* layout = (QGridLayout*)msgBox.layout();
        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
        msgBox.exec();
    }
    return success;
}

void MainWindow::_clearTotalsTable(void) {
    for(int r = 0; r < ui->nutritionalTotalsTableWidget->rowCount(); ++r) {
        for(int c = 0; c < ui->nutritionalTotalsTableWidget->columnCount(); ++c) {
            auto* item = ui->nutritionalTotalsTableWidget->item(r, c);
            if(item) item->setData(Qt::EditRole, QString(""));
        }
    }
}

void MainWindow::_clearCalculationTable(void) {
    for(int r = 0; r < ui->calculationResultTableWidget->rowCount(); ++r) {
        for(int c = 0; c < ui->calculationResultTableWidget->columnCount(); ++c) {
            auto* item = ui->calculationResultTableWidget->item(r, c);
            if(item) item->setData(Qt::EditRole, QString(""));
        }
    }
}

void MainWindow::onCalculateButtonClick(bool) {
    _clearTotalsTable();
    _clearCalculationTable();
    if(!_validateCalculationInputs()) return;
    int rows = rationTable->rowCount();
    float totalNem = 0, totalNeg = 0, totalProtein = 0, totalCa = 0, totalPhosphorus = 0, totalVitA = 0;

    for (int i = 0; i < rows; i++) {
        Ration ration = rationTable->rationFromRow(i);
        int ingredientRow = ingredientsTable->rowFromName(ration.ingredient);

        if (ingredientRow != -1) {
            Ingredient ingredient = ingredientsTable->ingredientFromRow(ingredientRow);
            totalNem        += ingredient.nem;
            totalNeg        += ingredient.neg;
            totalProtein    += ingredient.protein;
            totalCa         += ingredient.ca;
            totalPhosphorus += ingredient.p;
            totalVitA       += ingredient.vita;
        }
    }

    char totalNemString[200], totalNegString[200], totalProteinString[200], totalCaString[200], totalPhosphorusString[200], totalVitAString[200];

    sprintf(totalNemString,         "%.2f", totalNem);
    sprintf(totalNegString,         "%.2f", totalNeg);
    sprintf(totalProteinString,     "%.2f", totalProtein);
    sprintf(totalCaString,          "%.2f", totalCa);
    sprintf(totalPhosphorusString,  "%.2f", totalPhosphorus);
    sprintf(totalVitAString,        "%.2f", totalVitA);


    auto createItem = [](QString label) {
        QTableWidgetItem* item = new QTableWidgetItem(label);
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        return item;
    };

    ui->nutritionalTotalsTableWidget->setItem(0, 0, createItem(totalNemString));
    ui->nutritionalTotalsTableWidget->setItem(1, 0, createItem(totalNegString));
    ui->nutritionalTotalsTableWidget->setItem(2, 0, createItem(totalProteinString));
    ui->nutritionalTotalsTableWidget->setItem(3, 0, createItem(totalCaString));
    ui->nutritionalTotalsTableWidget->setItem(4, 0, createItem(totalPhosphorusString));
    ui->nutritionalTotalsTableWidget->setItem(5, 0, createItem(totalVitAString));

    AnimalNutritionReq req = animalNutritionReqTable->nutritionReqFromRow(animalNutritionReqTable->rowFromDesc(ui->animalComboBox->currentText()));

    float sum[6] = {
        totalNem, totalNeg, totalProtein, totalCa, totalPhosphorus, totalVitA
    };
    float expected[6] = {
        req.nem, req.neg, req.protein, req.calcium, req.phosphorus, req.vita
    };


    for(unsigned i = 0; i < 6; ++i) {
        float diff = fabs(expected[i]-sum[i]);
        QTableWidgetItem* item = new QTableWidgetItem(QString::number(expected[i]) + QString(" (")+QString::number(sum[i]/expected[i]*100.0f)+QString(")"));
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        item->setData(Qt::EditRole, sum[i]/expected[i]);

        if(diff <= expected[i]*TOTALS_TABLE_DIFF_GREEN_RANGE) {
            item->setData(Qt::BackgroundRole, QColor(Qt::green));
        } else if(diff <= expected[i]*TOTALS_TABLE_DIFF_YELLOW_RANGE){
            item->setData(Qt::BackgroundRole, QColor(Qt::yellow));
        } else {
            item->setData(Qt::BackgroundRole, QColor(Qt::red));
        }
        ui->nutritionalTotalsTableWidget->setItem(i, 1, item);
    }

    if(req.type < ANIMAL_TYPE_CUSTOM) {
        auto& energyData = AnimalNutritionReqTable::builtinEnergyTable[req.type];
        ui->calculationResultTableWidget->setEnabled(true);

        //Compute average weight gained per day (lbs)
        float totalFeedPerDay = 0.0;
        float totalCostPerDay = 0.0;
        for (int i = 0; i < rows; i++) {
            Ration ration = rationTable->rationFromRow(i);
            totalFeedPerDay += ration.asFed;
            totalCostPerDay += ration.costPerDay;
        }

        float maintenanceEnergyFromRation = totalNem / totalFeedPerDay;
        float energyAvailForGainingFromRation = totalNeg / totalFeedPerDay;

        //standard metric conversion
        float currentWeightKgs = ui->currentBodyWeightSpinBox->value() * KG_PER_LB;

        //same for all cattle (steers, heifers, bulls, cows
        float dailyEnergyReqForCattle = (0.077 * qPow(currentWeightKgs, 0.75));

        float feedRequiredToMaintainWeight = dailyEnergyReqForCattle / maintenanceEnergyFromRation;
        float amountOfFeedLeftForGaining = totalFeedPerDay - feedRequiredToMaintainWeight;
        float netEnergyForGainingFromRemainingFeed = amountOfFeedLeftForGaining * energyAvailForGainingFromRation;

        //different depending on frames of shit
        float liveWeightGainForMedFrameSteerKg = energyData.negCoeff * qPow(netEnergyForGainingFromRemainingFeed, energyData.negExp) * qPow(currentWeightKgs, energyData.weightExp);
        float liveWeightGainForMedFrameSteerlb = liveWeightGainForMedFrameSteerKg / 0.453592;

        char liveWeightGainForMedFrameSteerlbString[200];
        sprintf(liveWeightGainForMedFrameSteerlbString, "%.2f", liveWeightGainForMedFrameSteerlb);
        ui->calculationResultTableWidget->setItem(3, 0, createItem(liveWeightGainForMedFrameSteerlbString));
        //END compute and display average weight gained per day (lbs)

        //display total cost per day
        char totalCostPerDayString[200];
        sprintf(totalCostPerDayString, "%.2f", totalCostPerDay);
        ui->calculationResultTableWidget->setItem(0, 0, createItem(totalCostPerDayString));

        //compute and display cost per pound gained
        float costPerPoundGained = totalCostPerDay / liveWeightGainForMedFrameSteerlb;
        char costPerPoundGainedString[200];
        sprintf(costPerPoundGainedString, "%.2f", costPerPoundGained);
        ui->calculationResultTableWidget->setItem(1, 0, createItem(costPerPoundGainedString));

        //compute and display days on feed
        float numDaysOnFeed = (ui->expectedSellWeightSpinBox->value() - ui->currentBodyWeightSpinBox->value())/liveWeightGainForMedFrameSteerlb;
        char numDaysOnFeedString[200];
        sprintf(numDaysOnFeedString, "%.2f", numDaysOnFeed);
        ui->calculationResultTableWidget->setItem(4, 0, createItem(numDaysOnFeedString));

        //compute and display cost per head
        float costPerHead = totalCostPerDay * numDaysOnFeed;
        char costPerHeadString[200];
        sprintf(costPerHeadString, "%.2f", costPerHead);
        ui->calculationResultTableWidget->setItem(2, 0, createItem(costPerHeadString));

        //compute and display weeks on feed
        float numWeeksOnFeed = numDaysOnFeed / 7.0;
        char numWeeksOnFeedString[200];
        sprintf(numWeeksOnFeedString, "%.2f", numWeeksOnFeed);
        ui->calculationResultTableWidget->setItem(5, 0, createItem(numWeeksOnFeedString));

        //compute and display sell date
        QDateTime sellDate = ui->startDateEdit->dateTime().addDays(numDaysOnFeed);
        ui->calculationResultTableWidget->setItem(6, 0, createItem(sellDate.date().toString()));
    } else {
        ui->calculationResultTableWidget->setEnabled(false);

    }
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

void MainWindow::onIngredientsDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) {
    auto updatePerRole = [&](int role) {
        if(role == Qt::EditRole) {
            for(int row = topLeft.row(); row <= bottomRight.row(); ++row) {
                for(int col = topLeft.column(); col <= bottomRight.column(); ++col) {
                    switch(col) {
                    case IngredientsTable::COL_DM: {
                        Ingredient ingr = ingredientsTable->ingredientFromRow(row);
                        rationTable->ingredientDMChanged(QString(ingr.name));
                    }
                    default: break;
                    }
                }
            }
        }
    };

    //roles param is optional and is assumed to be all roles if not provided!
    if(roles.size()) {
        for(auto role : roles) {
            updatePerRole(role);
        }
    } else updatePerRole(Qt::EditRole);
}

