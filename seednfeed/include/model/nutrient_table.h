#ifndef NUTRIENT_TABLE_H
#define NUTRIENT_TABLE_H

#include "model/sql_table.h"

#define NUTRIENT_NAME_SIZE    50

class QStringList;

class IngredientsTable;

struct Nutrient {
    char name[NUTRIENT_NAME_SIZE];
    bool nameValid = false;

    int validate(QStringList& detailedText) const;
};

class NutrientTable : public SqlTableModel {
private:
    IngredientsTable*   _ingredientsTable;
public:

    enum COLUMNS {
        COL_NAME
    };

                            NutrientTable(QObject* parent, QSqlDatabase database);
    virtual Qt::ItemFlags   flags(const QModelIndex &index) const override;
    virtual bool            setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole) override;

    Nutrient                nutrientFromRow(int row);
    int                     rowFromName(QString name);
    void                    insertHeaderData(void);

    void                    setIngredientsTable(IngredientsTable* table);
};


#endif // NUTRIENT_TABLE_H
