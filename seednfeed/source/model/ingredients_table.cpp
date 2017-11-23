#include "model/ingredients_table.h"
#include "model/nutrient_table.h"
#include "core/utilities.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

Ingredient::Ingredient(void): Ingredient(0) {}
Ingredient::Ingredient(unsigned nutrientCount) {
    setNutrientCount(nutrientCount);
}

Ingredient::Ingredient(Ingredient&& other):
    _nutrientsCount(other._nutrientsCount),
    _nutrientsValues(other._nutrientsValues),
    _nutrientsValid(other._nutrientsValid)
{
    other._nutrientsCount = 0;
    other._nutrientsValid = nullptr;
    other._nutrientsValues = nullptr;
}

Ingredient::~Ingredient(void) {
    setNutrientCount(0);
}


void Ingredient::setNutrientCount(unsigned count) {
    if(_nutrientsCount == count) return;

    if(!_nutrientsValid) { // initial allocation
        if(count) {
            _nutrientsValid  = (bool*)malloc(sizeof(bool)*count);
            _nutrientsValues = (float*)malloc(sizeof(float)*count);
            memset(_nutrientsValid, 0, sizeof(bool)*count);
        }
    } else {
        if(count) { //resizing
            _nutrientsValid  = (bool*)realloc(_nutrientsValid, sizeof(bool)*count);
            _nutrientsValues = (float*)realloc(_nutrientsValues, sizeof(float)*count);
            memset(_nutrientsValid, 0, sizeof(bool)*count);
        } else { //deleting
            free(_nutrientsValid);
            free(_nutrientsValues);
            _nutrientsValid  = nullptr;
            _nutrientsValues = nullptr;
        }
    }

    _nutrientsCount = count;

}

unsigned Ingredient::getNutrientCount(void) const {
    return _nutrientsCount;
}

bool Ingredient::isNutrientValid(unsigned index) const {
    return (index < _nutrientsCount)? _nutrientsValid[index] : false;
}

void Ingredient::setNutrientValue(unsigned index, float value) {
    Q_ASSERT(index < _nutrientsCount);
    _nutrientsValues[index] = value;
}

void Ingredient::setNutrientValid(unsigned index, bool value) {
    Q_ASSERT(index < _nutrientsCount);
    _nutrientsValid[index] = value;
}

float Ingredient::getNutrientValue(unsigned index) const {
    Q_ASSERT(index < _nutrientsCount);
    return _nutrientsValues[index];
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
            ingredient.setNutrientValue(i, index(i, COL_DYNAMIC+i).data().toFloat(&valid));
            ingredient.setNutrientValid(i, valid);
        }

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
#if 0
    setHeaderData(COL_NAME,     Qt::Horizontal, "Ingredient");
    setHeaderData(COL_DM,       Qt::Horizontal, "DM, %");
    setHeaderData(COL_NEM,      Qt::Horizontal, "NEm (MCal/lb)");
    setHeaderData(COL_NEG,      Qt::Horizontal, "NEg (MCal/lb)");
    setHeaderData(COL_PROTEIN,  Qt::Horizontal, "Protein, lbs");
    setHeaderData(COL_CA,       Qt::Horizontal, "Ca, lbs");
    setHeaderData(COL_P,        Qt::Horizontal, "P, lbs");
    setHeaderData(COL_VITA,     Qt::Horizontal, "Vit A, IU");

    Q_ASSERT(_nutrientTable);
    for(unsigned i = 0; i < _nutrientTable->rowCount(); ++i) {
        Nutrient nutrient = _nutrientTable->nutrientFromRow(i);
        Q_ASSERT(nutrient.nameValid);
        setHeaderData(COL_DYNAMIC+i, Qt::Horizontal, nutrient.name);
    }
#endif
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


