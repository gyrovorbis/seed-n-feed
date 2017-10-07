#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ingredients_table.h"
#include "ration_table.h"
//#include "qtdir/src/sql/drivers/sqlite/qsql_sqlite.cpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    db = QSqlDatabase::addDatabase("QPSQL");
    //db.setHostName("gyrovorbis");
    db.setDatabaseName("/Users/scheelerl/Documents/seed_and_feed.db");
    //db.setUserName("girgis");
    //db.setPassword("girgisClan");
    bool ok = db.open();

    ingredientsTable = new IngredientsTable(this, db);
    ingredientsTable->setTable("Ingredients");
    ingredientsTable->setEditStrategy(QSqlTableModel::OnManualSubmit);
    ingredientsTable->select();
    ingredientsTable->insertColumns(ingredientsTable->columnCount(), 8);
    ingredientsTable->setHeaderData(0, Qt::Horizontal, "Ingredient");
    ingredientsTable->setHeaderData(1, Qt::Horizontal, "DM, %");
    ingredientsTable->setHeaderData(2, Qt::Horizontal, "NEm");
    ingredientsTable->setHeaderData(3, Qt::Horizontal, "NEg");
    ingredientsTable->setHeaderData(4, Qt::Horizontal, "Protein, lbs");
    ingredientsTable->setHeaderData(5, Qt::Horizontal, "Ca, lbs");
    ingredientsTable->setHeaderData(6, Qt::Horizontal, "P, lbs");
    ingredientsTable->setHeaderData(7, Qt::Horizontal, "Vit A, IU");

    //ingredientsTable->insertRows(ingredientsTable->rowCount(), 4);
    ui->ingredientsTableView->setModel(ingredientsTable);

    rationTable = new RationTable(this, db);
    rationTable->setTable("Rations");
    rationTable->setEditStrategy(QSqlTableModel::OnRowChange);
    rationTable->select();
    rationTable->insertColumns(rationTable->columnCount(), 6);
    rationTable->setHeaderData(0, Qt::Horizontal, "Ingredient");
    rationTable->setHeaderData(1, Qt::Horizontal, "As fed, lbs");
    rationTable->setHeaderData(2, Qt::Horizontal, "Cost, $/Unit");
    rationTable->setHeaderData(3, Qt::Horizontal, "Weight, lbs/Unit");
    rationTable->setHeaderData(4, Qt::Horizontal, "Cost/day");
    rationTable->setHeaderData(5, Qt::Horizontal, "DM, lbs");

    ui->rationCalculatorTableView->setModel(rationTable);

    connect(ui->addIngredientButton, SIGNAL(clicked(bool)), this , SLOT(onAddIngredientClick(bool)));
    //connect(ui->deleteIngredientButton, SIGNAL(clicked(bool)), this, SLOT(onDeleteIngredientClick(bool)));

}

void MainWindow::onAddIngredientClick(bool) {
    ingredientsTable->insertRows(ingredientsTable->rowCount(), 1);
    ingredientsTable->submitAll();
    //ingredientsTable->setData(0, "hi", Qt::EditRole)
}

void MainWindow::onDeleteIngredientClick(bool) {
    // check which row(s) is/are selected (if any)
    QItemSelectionModel *select = ui->ingredientsTableView->selectionModel();
    QModelIndexList selectedRows = select->selectedRows();

   // ingredientsTable->removeRows()

//    for (int i = 0; i < ingredientsTable->rowCount(); i++ ) {
//        ingredientsTable->deleteRowFromTable(selectedRows[i]);
//    }
   // connect(ui->ingredientsTableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT();
}

MainWindow::~MainWindow()
{
    delete ui;
}
