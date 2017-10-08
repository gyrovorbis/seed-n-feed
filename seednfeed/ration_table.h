#ifndef RATION_TABLE_H
#define RATION_TABLE_H
#include <QString>
#include "ingredients_table.h"

class RationTable : public QSqlTableModel
{
//    float amountPerFeedingIbs = 0.0;
//    float costPerUnit = 0.0;
//    float weightPerUnitIbs = 0.0;
//    float costPerDay = 0.0;
//    float dryMatterIbs = 0.0;
//    //Ingredient ingredient;

//    float calculateCostPerDay();
//    float calculateDryMatterIbs();

public:
    RationTable(QObject* parent, QSqlDatabase database);

    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
};

#endif // RATION_TABLE_H
