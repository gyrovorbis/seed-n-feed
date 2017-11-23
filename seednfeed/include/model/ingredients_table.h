#ifndef INGREDIENTS_TABLE_H
#define INGREDIENTS_TABLE_H
#include <QString>

#include "model/sql_table.h"

#define INGREDIENT_NAME_SIZE    50

class QStringList;
class NutrientTable;

struct Ingredient {
            Ingredient(void);
            Ingredient(unsigned nutrientCount);
            Ingredient(Ingredient&& other);

            ~Ingredient(void);

    bool    nameValid = false;
    char    name[INGREDIENT_NAME_SIZE];
    bool    dmValid = false;
    float   dm;
    bool    nemValid = false;
    float   nem;
    bool    negValid = false;
    float   neg;
    bool    proteinValid = false;
    float   protein;
    bool    caValid = false;
    float   ca;
    bool    pValid = false;
    float   p;
    bool    vitaValid = false;
    float   vita;

    void        setNutrientCount(unsigned count);
    unsigned    getNutrientCount(void) const;
    bool        isNutrientValid(unsigned index) const;
    void        setNutrientValue(unsigned index, float value);
    void        setNutrientValid(unsigned index, bool value=true);
    float       getNutrientValue(unsigned index) const;


    int validate(QStringList& detailedText) const;

private:
    unsigned    _nutrientsCount = 0;
    float*      _nutrientsValues = nullptr;
    bool*       _nutrientsValid  = nullptr;

};

class IngredientsTable : public SqlTableModel {
private:
    NutrientTable*  _nutrientTable = nullptr;
public:

    enum COLUMNS {
        COL_NAME,
        COL_DM,
        COL_NEM,
        COL_NEG,
        COL_PROTEIN,
        COL_CA,
        COL_P,
        COL_VITA,
        COL_DYNAMIC
    };

                            IngredientsTable(QObject* parent, QSqlDatabase database);
    virtual Qt::ItemFlags   flags(const QModelIndex &index) const override;

    Ingredient              ingredientFromRow(int row);
    int                     rowFromName(QString name);
    void                    insertHeaderData(void);

    void                    setNutrientTable(NutrientTable* table);
};


#endif // NUTRITIONALVALUE_H
