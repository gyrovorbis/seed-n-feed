#include "ingredients_table.h"


IngredientsTable::IngredientsTable(QObject* parent, QSqlDatabase db):
    QSqlTableModel(parent, db)
{
    setHeaderData(COL_NAME,     Qt::Horizontal, "Ingredient");
    setHeaderData(COL_DM,       Qt::Horizontal, "DM, %");
    setHeaderData(COL_NEM,      Qt::Horizontal, "NEm");
    setHeaderData(COL_NEG,      Qt::Horizontal, "NEg");
    setHeaderData(COL_PROTEIN,  Qt::Horizontal, "Protein, lbs");
    setHeaderData(COL_CA,       Qt::Horizontal, "Ca, lbs");
    setHeaderData(COL_P,        Qt::Horizontal, "P, lbs");
    setHeaderData(COL_VITA,     Qt::Horizontal, "Vit A, IU");
    setEditStrategy(QSqlTableModel::OnFieldChange);
}

Qt::ItemFlags IngredientsTable::flags(const QModelIndex &index) const {
    return (Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsEditable);
}
