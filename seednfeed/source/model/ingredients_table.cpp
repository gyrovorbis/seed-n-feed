#include "model/ingredients_table.h"
#include "model/nutrient_table.h"
#include "core/utilities.h"
#include <QSqlQuery>
#include <QSqlError>
#include "ui/mainwindow.h"

Ingredient::Ingredient(void): Ingredient(0) {}
Ingredient::Ingredient(unsigned nutrientCount) {
    //setDynamicColumnCount(nutrientCount);
}

Ingredient::Ingredient(Ingredient&& other)//:
    //DynamicColumnsTableEntry(std::move(other))
{
    memcpy(this+offsetof(Ingredient, nameValid), &other+offsetof(Ingredient, nameValid), offsetof(Ingredient, vita)-offsetof(Ingredient, nameValid));
}


IngredientsTable::IngredientsTable(QObject* parent, QSqlDatabase db):
    SqlTableModel(parent, db)
{
    setEditStrategy(QSqlTableModel::OnFieldChange);
}

Qt::ItemFlags IngredientsTable::flags(const QModelIndex& /*index*/) const {
    return (Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsEditable);
}

Ingredient IngredientsTable::ingredientFromRow(int row) {
    Q_ASSERT(_nutrientTable);

    Ingredient ingredient(_nutrientTable->rowCount());

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

        for(int i = 0; i < _nutrientTable->rowCount(); ++i) {
            bool valid;
            float val = index(row, COL_DYNAMIC+i).data().toFloat(&valid);
          #if 0
            ingredient.setDynamicColumnValue(i, val);
            ingredient.setDynamicColumnValue(i, valid);
#endif
        }

    } else {
        MainWindow::dbgPrintf("Requesting invalid Ingredient row index: %d", row);
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

    addHeaderData(COL_NAME,     Qt::Horizontal, "Name");
    addHeaderData(COL_DM,       Qt::Horizontal, "DM", "Dry Matter (% of total ration weight)");
    addHeaderData(COL_TDN,      Qt::Horizontal, "TDN", "Total Digestible Nutrients (% of DM)");
    addHeaderData(COL_NEM,      Qt::Horizontal, "NEm", "Energy Required for Maintenance (MCal/cwt)");
    addHeaderData(COL_NEG,      Qt::Horizontal, "NEg", "Energy Required for Gain (MCal/cwt)");
    addHeaderData(COL_PROTEIN,  Qt::Horizontal, "CP", "Crude Protein (% of DM)");
    addHeaderData(COL_CA,       Qt::Horizontal, "Ca", "Calcium (% of DM)");
    addHeaderData(COL_P,        Qt::Horizontal, "P", "Phosphorus (% of DM)");
    addHeaderData(COL_VITA,     Qt::Horizontal, "Vit A");

    Q_ASSERT(_nutrientTable);
    for(int i = 0; i < _nutrientTable->rowCount(); ++i) {
        Nutrient nutrient = _nutrientTable->nutrientFromRow(i);
        Q_ASSERT(nutrient.nameValid);
        setHeaderData(COL_DYNAMIC+i, Qt::Horizontal, nutrient.name);
    }
}

void IngredientsTable::setNutrientTable(NutrientTable* table) {
    _nutrientTable = table;
}

int Ingredient::validate(QStringList &detailedText) const {
    int errors = 0;
    auto validateField = [&](bool valid, QString name) {
        if(!valid) {
            if(nameValid) detailedText += QString("Ingredient '") + QString(this->name) + QString("' ");
            detailedText += QString("invalid field: ") + name + QString("\n");
            ++errors;
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
   return errors;
}


