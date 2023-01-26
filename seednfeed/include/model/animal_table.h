#ifndef ANIMAL_TABLE_H
#define ANIMAL_TABLE_H

#include "model/sql_table.h"

#define ANIMAL_TABLE_NAME   "Animals"
#define ANIMAL_NAME_SIZE    200

struct Animal {
    char name[ANIMAL_NAME_SIZE];

    int validate(QStringList& detailedText) const;
};

class AnimalTable : public SqlTableModel {
public:

    enum COLUMNS {
        COL_NAME,
        COL_DESC
    };

                            AnimalTable(QObject* parent, QSqlDatabase database);
    virtual Qt::ItemFlags   flags(const QModelIndex &index) const override;

    Animal                  recipeFromRow(int row);
    int                     rowFromName(QString name);
    void                    insertHeaderData(void);
};

#endif // ANIMAL_TABLE_H
