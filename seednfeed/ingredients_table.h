#ifndef INGREDIENTS_TABLE_H
#define INGREDIENTS_TABLE_H
#include <QString>
#include <QSqlTableModel>

#define INGREDIENT_NAME_SIZE    50
#if 0
"(name varchar(50) primary key, "
"dm real, "
"nem real,"
"neg real,"
 "protein real,"
 "ca real,"
 "p real,"
     "vita real)"))
#endif

class IngredientsTable : public QSqlTableModel {
public:

    enum COLUMNS {
        COL_NAME,
        COL_DM,
        COL_NEM,
        COL_NEG,
        COL_PROTEIN,
        COL_CA,
        COL_P,
        COL_VITA
    };

                            IngredientsTable(QObject* parent, QSqlDatabase database);
    virtual Qt::ItemFlags   flags(const QModelIndex &index) const override;
};

#endif // NUTRITIONALVALUE_H
