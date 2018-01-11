#include "model/ration_table.h"
#include "delegate/rations_table_delegate.h"
#include "model/ingredients_table.h"
#include "core/utilities.h"
#include "ui/mainwindow.h"
#include <QMessageBox>

RationTable::RationTable(QObject* parent, QSqlDatabase db):
    SqlTableModel(parent, db)
{
    setEditStrategy(QSqlTableModel::OnFieldChange);
}

Qt::ItemFlags RationTable::flags(const QModelIndex& /*index*/) const {
    return (Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsEditable);
}

void RationTable::setIngredientsTable(IngredientsTable* table) {
    _ingredientsTable = table;

   // connect(_ingredientsTable, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(onIngredientsDataChanged(QModelIndex,QModelIndex,QVector<int>)));
    connect(_ingredientsTable, SIGNAL(cellDataChanged(int,int,QVariant,QVariant,int)), this, SLOT(onIngredientsCellValueChanged(int,int,QVariant,QVariant,int)));
}

void RationTable::setRecipeTable(RecipeTable *table) {
    _recipeTable = table;
   // connect(_recipeTable, SIGNAL(cellDataChanged(int,int,QVariant,QVariant,int)), this, SLOT(onRecipeCellValueChanged(int,int,QVariant,QVariant,int)));
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
        MainWindow::dbgPrintf("Requesting invalid Ration row index: %d", row);
    }

    return ration;
}


void RationTable::onIngredientsCellValueChanged(int row, int col, QVariant oldValue, QVariant newValue, int role) {
    if(role == Qt::EditRole) {
        if(col == IngredientsTable::COL_DM) {
            QString name = _ingredientsTable->index(row, IngredientsTable::COL_NAME).data(role).toString();
            iterateForColumnVariants(RationTable::COL_INGREDIENT, [&](int row, QVariant data) {
                if(data.toString() == name) {
                    RationsTableDelegate::_updateReadOnlyColumns(this, index(row, COL_INGREDIENT));
                }
            });
        }
    }
}

unsigned RationTable::_ingredientDMChanged(QString ingredientName) {
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
        return QSqlTableModel::data(index, role);

    }
}

void RationTable::insertHeaderData(void) {
    setHeaderData(COL_RECIPE,           Qt::Horizontal, "Recipe");
    setHeaderData(COL_INGREDIENT,       Qt::Horizontal, "Ingredient");
    setHeaderData(COL_AS_FED,           Qt::Horizontal, "As fed, lbs");
    setHeaderData(COL_COST_PER_UNIT,    Qt::Horizontal, "Cost, $/Unit");
    setHeaderData(COL_WEIGHT,           Qt::Horizontal, "Weight, lbs/Unit");
    setHeaderData(COL_COST_PER_DAY,     Qt::Horizontal, "Cost/day");
    setHeaderData(COL_DM,               Qt::Horizontal, "DM, lbs");
}

QStringList RationTable::getUnusedIngredientsList(int excludeRow) const {
    QStringList ingredientsList;
    _ingredientsTable->iterateForColumnVariants(IngredientsTable::COL_NAME, [&](int, QVariant value) {
        ingredientsList.push_back(value.toString());
    });

    iterateForColumnVariants(RationTable::COL_INGREDIENT, [&](int row, QVariant value) {
        if(excludeRow == -1 || excludeRow != row) ingredientsList.removeAll(value.toString());
    });
    return ingredientsList;
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

   validateField(ingredientValid,   "Ingredient");
   validateField(asFedValid,        "As Fed, lbs");
   validateField(costPerUnitValid,  "Cost Per Unit ($/Unit)");
   validateField(weightValid,       "Weight (lbs/unit)");
   //validateField(costPerDayValid, "Cost Per Day");
   //validateField(dm, "DM (lbs)");

    return errors;

}


