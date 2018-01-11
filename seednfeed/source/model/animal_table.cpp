#include "model/animal_table.h"
#include "core/utilities.h"

AnimalTable::AnimalTable(QObject *parent, QSqlDatabase database):
           SqlTableModel(parent, database)
{
    setEditStrategy(QSqlTableModel::OnFieldChange);
}

Qt::ItemFlags AnimalTable::flags(const QModelIndex &) const {
    return (Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsEditable);
}

void AnimalTable::insertHeaderData(void) {
    setHeaderData(COL_NAME,       Qt::Horizontal, "Type Name");
}
