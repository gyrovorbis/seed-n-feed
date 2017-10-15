#include "ingredients_table.h"
#include "utilities.h"
#include <QDebug>


IngredientsTable::IngredientsTable(QObject* parent, QSqlDatabase db):
    QSqlTableModel(parent, db)
{
    setEditStrategy(QSqlTableModel::OnFieldChange);
}

Qt::ItemFlags IngredientsTable::flags(const QModelIndex& /*index*/) const {
    return (Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsEditable);
}

Ingredient IngredientsTable::ingredientFromRow(int row) {
    Ingredient ingredient;
    memset(&ingredient, 0, sizeof(Ingredient));

    if(row < rowCount()) {
        auto name = index(row, COL_NAME).data().toString();
        if(name.isEmpty() || name.isNull()) {
            ingredient.name[0] = '\0';
            ingredient.nameValid = false;
        } else {
            strcpy(ingredient.name, Q_CSTR(name));
            ingredient.nameValid = true;
        }

        ingredient.ca       = index(row, COL_CA).data().toFloat(&ingredient.caValid);
        ingredient.dm       = index(row, COL_DM).data().toFloat(&ingredient.dmValid);
        ingredient.nem      = index(row, COL_NEM).data().toFloat(&ingredient.nemValid);
        ingredient.neg      = index(row, COL_NEG).data().toFloat(&ingredient.negValid);
        ingredient.protein  = index(row, COL_PROTEIN).data().toFloat(&ingredient.proteinValid);
        ingredient.ca       = index(row, COL_CA).data().toFloat(&ingredient.caValid);
        ingredient.p        = index(row, COL_P).data().toFloat(&ingredient.pValid);
        ingredient.vita     = index(row, COL_VITA).data().toFloat(&ingredient.vitaValid);

    } else {
        qCritical() << "Requesting invalid Ingredient row index: " << row;
    }

    return ingredient;
}

int IngredientsTable::rowFromName(QString name) {
    for(int i = 0; i < rowCount(); ++i) {
        if(index(i, COL_NAME).data().toString() == name) return i;
    }
    return -1;
}

void IngredientsTable::insertHeaderData(void) {
    setHeaderData(COL_NAME,     Qt::Horizontal, "Ingredient");
    setHeaderData(COL_DM,       Qt::Horizontal, "DM, %");
    setHeaderData(COL_NEM,      Qt::Horizontal, "NEm (MCal/lb)");
    setHeaderData(COL_NEG,      Qt::Horizontal, "NEg (MCal/lb)");
    setHeaderData(COL_PROTEIN,  Qt::Horizontal, "Protein, lbs");
    setHeaderData(COL_CA,       Qt::Horizontal, "Ca, lbs");
    setHeaderData(COL_P,        Qt::Horizontal, "P, lbs");
    setHeaderData(COL_VITA,     Qt::Horizontal, "Vit A, IU");
}

bool Ingredient::validate(QString &detailedText) const {
    bool success = true;
    auto validateField = [&](bool valid, QString name) {
        if(!valid) {
            if(nameValid) detailedText += QString("Ingredient '") + QString(this->name) + QString("' ");
            detailedText += QString("invalid field: ") + name + QString("\n");
            success = false;
        }
    };

   validateField(nameValid, "Name");
   validateField(dmValid, "DM %");
   validateField(nemValid, "NEm (MCal/lb)");
   validateField(negValid, "NEg (MCal/lb)");
   validateField(proteinValid, "Protein, lbs");
   validateField(caValid, "Calcium, lbs");
   validateField(pValid, "P, lbs");
   validateField(vitaValid, "Vit A, IU");

    return success;

}
