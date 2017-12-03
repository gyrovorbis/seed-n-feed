#include <QMenu>
#include <QFileDialog>

#include "ui/sql_table_view.h"
#include "core/tk_menu_stack.h"
#include "model/sql_table.h"

SqlTableView::SqlTableView(QWidget* parent):
    QTableView(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onCustomContextMenu(QPoint)));
}

void SqlTableView::onCustomContextMenu(const QPoint& point) {

    //auto index = indexAt(point);

    //if(index.isValid())

    MenuStack menu("Table Menu");

    menu.push("Export to CSV");
    menu.insertAction("Entire Table", [&](bool){
        QString fileName = QFileDialog::getSaveFileName(this, "Save File", QDir::current().absolutePath(), "Spreadsheet (CSV) File (*.csv)");

        if(fileName.isNull() || fileName.isEmpty()) return;

        SqlTableModel::exportCSV(model(), fileName);


    }, model());
    menu.insertAction("Selection Only", [&](bool) {


    }, /*selectionModel()->hasSelection()*/ false);

    menu.pop();

    menu.push("Import from CSV");
    menu.insertAction("Entire Table", [&](bool){
        QString fileName = QFileDialog::getOpenFileName(this, "Save File", QDir::current().absolutePath(), "Spreadsheet (CSV) File (*.csv)");

        if(fileName.isNull() || fileName.isEmpty()) return;

        SqlTableModel::importCSV(model(), fileName);


    }, true);
    menu.insertAction("Append Rows", [&](bool){}, false);
    menu.pop();

    menu.first()->exec(mapToGlobal(point));
}
