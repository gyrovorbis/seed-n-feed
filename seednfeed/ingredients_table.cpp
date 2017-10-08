#include "ingredients_table.h"


IngredientsTable::IngredientsTable(QObject* parent, QSqlDatabase db):
    QSqlTableModel(parent, db)
{

}

Qt::ItemFlags IngredientsTable::flags(const QModelIndex &index) const {
    return (Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsEditable);
}
