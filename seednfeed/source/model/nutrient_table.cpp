#include "model/nutrient_table.h"
#include "model/ingredients_table.h"
#include "core/utilities.h"
#include <QDebug>

NutrientTable::NutrientTable(QObject *parent, QSqlDatabase database):
           SqlTableModel(parent, database)
{
    setEditStrategy(QSqlTableModel::OnFieldChange);
}

Qt::ItemFlags NutrientTable::flags(const QModelIndex &) const {
    return (Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsEditable);
}

void NutrientTable::insertHeaderData(void) {
    setHeaderData(COL_NAME,       Qt::Horizontal, "Name");
}


Nutrient NutrientTable::nutrientFromRow(int row) {
    Nutrient nutrient;
    memset(&nutrient, 0, sizeof(Nutrient));

    if(row < rowCount()) {
        auto name = index(row, COL_NAME).data().toString();
        if(name.isEmpty() || name.isNull()) {
            nutrient.name[0] = '\0';
            nutrient.nameValid = false;
        } else {
            strcpy(nutrient.name, Q_CSTR(name));
            nutrient.nameValid = true;
        }

    } else {
        qCritical() << "Requesting invalid Nutrient row index: " << row;
    }

    return nutrient;
}

int NutrientTable::rowFromName(QString name) {
    for(int i = 0; i < rowCount(); ++i) {
        if(index(i, COL_NAME).data().toString() == name) return i;
    }
    return -1;
}

void NutrientTable::setIngredientsTable(IngredientsTable* table) {
    _ingredientsTable = table;
}

bool NutrientTable::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(index.column() == COL_NAME && role == Qt::EditRole) {
        QString oldValue = data(index).toString();

        //check whether we're renaming an existing column
        if(!oldValue.isNull() && !oldValue.isEmpty()) {
            //check whether or not rename was successful
            if(!_ingredientsTable->renameSqlColumn(oldValue, value.toString())) {
                return false;
            }
        }
    }

    return SqlTableModel::setData(index, value, role);

}
