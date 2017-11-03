#ifndef INGREDIENTS_TABLE_H
#define INGREDIENTS_TABLE_H
#include <QString>
#include <QSqlTableModel>

#define INGREDIENT_NAME_SIZE    50

class QStringList;

struct Ingredient {
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

    int validate(QStringList& detailedText) const;
};

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

    Ingredient              ingredientFromRow(int row);
    int                     rowFromName(QString name);
    void                    insertHeaderData(void);
};

#endif // NUTRITIONALVALUE_H
