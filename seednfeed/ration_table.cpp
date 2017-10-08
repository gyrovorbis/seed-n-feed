#include "ration_table.h"
#include "utilities.h"
#include <QDebug>

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


Ration RationTable::rationFromRow(int row) {
    Ration ration;
    memset(&ration, 0, sizeof(Ration));

    if(row < rowCount()) {
        auto name = index(row, COL_INGREDIENT).data().toString();
        if(name.isEmpty() || name.isNull()) {
            ration.ingredient[0] = '\0';
        } else {
            strcpy(ration.ingredient, Q_CSTR(name));
        }

        ration.asFed        = index(row, COL_AS_FED).data().toFloat();
        ration.costPerUnit  = index(row, COL_COST_PER_UNIT).data().toFloat();
        ration.weight       = index(row, COL_WEIGHT).data().toFloat();
        ration.costPerDay   = index(row, COL_COST_PER_DAY).data().toFloat();
        ration.dm           = index(row, COL_DM).data().toFloat();

    } else {
        qCritical() << "Requesting invalid Ration row index: " << row;
    }

    return ration;
}

