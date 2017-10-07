#include "ingredients_table.h"


IngredientsTable::IngredientsTable(QObject* parent, QSqlDatabase db):
    QSqlTableModel(parent, db)
{

}
