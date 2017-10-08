#include "animal_nutrition_req_table.h"

AnimalNutritionReqTable::AnimalNutritionReqTable(QObject* parent, QSqlDatabase db):
    QSqlTableModel(parent, db)
{

}

Qt::ItemFlags AnimalNutritionReqTable::flags(const QModelIndex &) const {
    return (Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsEditable);
}
