#ifndef INGREDIENTS_TABLE_H
#define INGREDIENTS_TABLE_H
#include <QString>

#include "model/sql_table.h"

#define INGREDIENT_NAME_SIZE    50

class NutrientTable;

struct Ingredient { //: public DynamicColumnsTableEntry {
            Ingredient(void);
            Ingredient(unsigned nutrientCount);
            Ingredient(Ingredient&& other);

    bool    nameValid = false;
    char    name[INGREDIENT_NAME_SIZE];
    bool    dmValid = false;
    float   dm;
    bool    tdnValid = false;
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

class IngredientsTable : public SqlTableModel {
private:
    NutrientTable*  _nutrientTable = nullptr;
public:

    enum COLUMNS {
        COL_NAME,
        COL_DM,
        COL_TDN,
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




//========= INLINEZ ==========
#if 0
template<typename T>
inline T DynamicColumnsTableEntry::getDynamicColumnValue(int index) {
    Q_ASSERT(index >= 0 && index < _dynColCount);
    return _dynColVariants->value<T>();
}

template<typename T>
inline void DynamicColumnsTableEntry::setDynamicColumnValue(int index, T&& value) {
    Q_ASSERT(index >= 0 && index < _dynColCount);
    _dynColVariants[index] = std::forward<T>(value);
}

inline QVariant::Type DynamicColumnsTableEntry::getDynamicColumnType(int index) const {
    Q_ASSERT(index >= 0 && index < _dynColCount);
    return _dynColVariants[index].type();
}

inline QString DynamicColumnsTableEntry::getDynamicColumnTypeName(int index) const {
    Q_ASSERT(index >= 0 && index < _dynColCount);
    return _dynColVariants[index].typeName();
}
#endif

#endif // NUTRITIONALVALUE_H
