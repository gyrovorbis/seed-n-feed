#ifndef ANIMAL_NUTRITION_REQ_TABLE_H
#define ANIMAL_NUTRITION_REQ_TABLE_H
#include <QString>
#include <QSqlTableModel>


class AnimalNutritionReqTable : public QSqlTableModel
{
public:
    AnimalNutritionReqTable(QObject* parent, QSqlDatabase database);
    virtual Qt::ItemFlags flags(const QModelIndex &) const override;
};

#endif // ANIMAL_NUTRITION_REQ_TABLE_H
