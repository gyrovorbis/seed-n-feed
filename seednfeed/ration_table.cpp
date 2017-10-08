#include "ration_table.h"

RationTable::RationTable(QObject* parent): QStandardItemModel(parent)
{
    setColumnCount(6);
    setHeaderData(COL_INGREDIENT,       Qt::Horizontal, "Ingredient");
    setHeaderData(COL_AS_FED,           Qt::Horizontal, "As fed, lbs");
    setHeaderData(COL_COST_PER_UNIT,    Qt::Horizontal, "Cost, $/Unit");
    setHeaderData(COL_WEIGHT,           Qt::Horizontal, "Weight, lbs/Unit");
    setHeaderData(COL_COST_PER_DAY,     Qt::Horizontal, "Cost/day");
    setHeaderData(COL_DM,               Qt::Horizontal, "DM, lbs");
}

Qt::ItemFlags RationTable::flags(const QModelIndex &index) const {
    Qt::ItemFlags flags = (Qt::ItemIsEnabled|Qt::ItemIsSelectable);
    if(index.column() <= 3) flags |= Qt::ItemIsEditable;
    return flags;
}
