#ifndef INGREDIENTS_TABLE_H
#define INGREDIENTS_TABLE_H
#include <QString>
#include <QSqlTableModel>


class IngredientsTable : public QSqlTableModel
{
//    QString name = "";
//    float dryMatterPercentage = 0.0;
//    float netEnergyForMaintenance = 0.0;
//    float netEnergyForGain = 0.0;
//    float proteinLbs = 0.0;
//    float calciumLbs = 0.0;
//    float phosphorusLbs = 0.0;
//    float vitaminA_IU = 0.0;

public:
    IngredientsTable(QObject* parent, QSqlDatabase database);
};

#endif // NUTRITIONALVALUE_H
