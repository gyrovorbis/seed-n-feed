#ifndef RATION_TABLE_H
#define RATION_TABLE_H

#include <QString>
#include "model/sql_table.h"
#include "ingredients_table.h"
#include "recipe_table.h"

#define RATION_ITEM_TYPE_SIZE   50

class IngredientsTable;

struct Ration {
    bool    ingredientValid = false;
    char    ingredient[INGREDIENT_NAME_SIZE];
    bool    recipeValid     = false;
    char    recipe[RECIPE_NAME_SIZE];
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

class RationTable : public SqlTableModel {
    Q_OBJECT
protected:
    IngredientsTable*       _ingredientsTable = nullptr;
    RecipeTable*            _recipeTable      = nullptr;
public:
    enum COLUMNS {
        COL_RECIPE,
        COL_INGREDIENT,
        COL_AS_FED,
        COL_COST_PER_UNIT,
        COL_WEIGHT,
        COL_COST_PER_DAY,
        COL_DM
    };

                            RationTable(QObject* parent, QSqlDatabase db);
    virtual Qt::ItemFlags   flags(const QModelIndex &index) const override;
    virtual QVariant        data(const QModelIndex &index, int role) const override;


    bool                    tryAppendRow(void);
    void                    insertHeaderData(void);

    Ration                  rationFromRow(int row);

    QStringList             getUnusedIngredientsList(void) const;

    //returns number of affected/updated rows
    unsigned                _ingredientDMChanged(QString ingredientName);
    unsigned                _ingredientNameChanged(QString oldName, QString newName);
    unsigned                _recipeNameChanged(QString oldName, QString newName);

    void                    setIngredientsTable(IngredientsTable* table);
    void                    setRecipeTable(RecipeTable* table);

private slots:
    void                    onIngredientsDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);
    void                    onIngredientsCellValueChanged(int row, int col, QVariant oldValue, QVariant newValue, int role);
    void                    onRecipeCellValueChanged(int row, int col, QVariant oldValue, QVariant newValue, int role);
};

#endif // RATION_TABLE_H
