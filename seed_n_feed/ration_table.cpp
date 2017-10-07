#include "ration_table.h"

RationTable::RationTable(QObject* parent, QSqlDatabase db):
    QSqlTableModel(parent, db)
{

}
