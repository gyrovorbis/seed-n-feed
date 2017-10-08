#include "ration_table.h"

RationTable::RationTable(QObject* parent, QSqlDatabase db):
    QSqlTableModel(parent, db)
{

}

Qt::ItemFlags RationTable::flags(const QModelIndex &/*index*/) const {
    return (Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsEditable);
}
