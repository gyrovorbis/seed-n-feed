#include "model/ration_table.h"
#include "delegate/rations_table_delegate.h"
#include "core/utilities.h"
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
            ration.ingredientValid = false;
        } else {
            strcpy(ration.ingredient, Q_CSTR(name));
            ration.ingredientValid = true;
        }

        ration.asFed        = index(row, COL_AS_FED).data().toFloat(&ration.asFedValid);
        ration.costPerUnit  = index(row, COL_COST_PER_UNIT).data().toFloat(&ration.costPerUnitValid);
        ration.weight       = index(row, COL_WEIGHT).data().toFloat(&ration.weightValid);
        ration.costPerDay   = index(row, COL_COST_PER_DAY).data().toFloat(&ration.costPerDayValid);
        ration.dm           = index(row, COL_DM).data().toFloat(&ration.dmValid);

    } else {
        qCritical() << "Requesting invalid Ration row index: " << row;
    }

    return ration;
}


unsigned RationTable::ingredientDMChanged(QString ingredientName) {
    int updatedCount = 0;
    for(int r = 0; r < rowCount(); ++r) {
        Ration ration = rationFromRow(r);
        if(ingredientName == QString(ration.ingredient)) {
            ++updatedCount;
            RationsTableDelegate::_updateReadOnlyColumns(this, index(r, COL_INGREDIENT));
        }
    }
    return updatedCount;
}

QVariant RationTable::data(const QModelIndex &index, int role) const {
    switch(index.column()) {
    case COL_DM:
    case COL_COST_PER_DAY:
        switch(role) {
        case Qt::BackgroundRole: return QColor(Qt::lightGray);
        default: break;
        }
    default:
        return QStandardItemModel::data(index, role);

    }
}

int Ration::validate(QStringList& detailedText) const {
    int errors = 0;
    auto validateField = [&](bool valid, QString name) {
        if(!valid) {
            if(ingredientValid) detailedText += QString("Ration '") + QString(ingredient) + QString("' ");
            detailedText += QString("invalid field: ") + name + QString("\n");
            ++errors;
        }
    };

   validateField(ingredientValid, "Ingredient");
   validateField(asFedValid, "As Fed, lbs");
   validateField(costPerUnitValid, "Cost Per Unit ($/Unit)");
   validateField(weightValid, "Weight (lbs/unit)");
   //validateField(costPerDayValid, "Cost Per Day");
   //validateField(dm, "DM (lbs)");

    return errors;

}
