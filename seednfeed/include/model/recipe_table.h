#ifndef RECIPE_TABLE_H
#define RECIPE_TABLE_H

#include "model/sql_table.h"

#define RECIPE_NAME_SIZE    50

struct Recipe {
    char name[RECIPE_NAME_SIZE];

    int validate(QStringList& detailedText) const;
};

class RecipeTable : public SqlTableModel {
public:

    enum COLUMNS {
        COL_NAME,
        COL_DESC
    };

                            RecipeTable(QObject* parent, QSqlDatabase database);
    virtual Qt::ItemFlags   flags(const QModelIndex &index) const override;

    Recipe                  recipeFromRow(int row);
    int                     rowFromName(QString name);
    void                    insertHeaderData(void);
};

#endif // RECIPE_TABLE_H
