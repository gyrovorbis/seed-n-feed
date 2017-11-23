#include "model/recipe_table.h"
#include "core/utilities.h"
#include <QDebug>

RecipeTable::RecipeTable(QObject *parent, QSqlDatabase database):
           QSqlTableModel(parent, database)
{
    setEditStrategy(QSqlTableModel::OnFieldChange);
}

Qt::ItemFlags RecipeTable::flags(const QModelIndex &) const {
    return (Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsEditable);
}

void RecipeTable::insertHeaderData(void) {
    setHeaderData(COL_NAME,       Qt::Horizontal, "Name");
}
