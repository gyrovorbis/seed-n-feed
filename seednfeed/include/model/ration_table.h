#ifndef RATION_TABLE_H
#define RATION_TABLE_H

#include <QString>
#include <QStandardItemModel>
#include "ingredients_table.h"

#define RATION_ITEM_TYPE_SIZE   50

struct Ration {
    bool    ingredientValid = false;
    char    ingredient[INGREDIENT_NAME_SIZE];
    bool    asFedValid = false;
    float   asFed;
    bool    costPerUnitValid = false;
    float   costPerUnit;
    bool    weightValid = false;
    float   weight;
    bool    costPerDayValid = false;
    float   costPerDay;
    bool    dmValid = false;
    float   dm;

    int validate(QStringList& detailedText) const;
};

class RationTable : public QStandardItemModel {

public:
    enum COLUMNS {
        COL_INGREDIENT,
        COL_AS_FED,
        COL_COST_PER_UNIT,
        COL_WEIGHT,
        COL_COST_PER_DAY,
        COL_DM
    };

                            RationTable(QObject* parent=nullptr);
    virtual Qt::ItemFlags   flags(const QModelIndex &index) const override;
    virtual QVariant        data(const QModelIndex &index, int role) const override;

    Ration                  rationFromRow(int row);

    //returns number of affected/updated rows
    unsigned                ingredientDMChanged(QString ingredientName);
};

#endif // RATION_TABLE_H
