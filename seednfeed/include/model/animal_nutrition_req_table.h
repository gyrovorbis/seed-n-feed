#ifndef ANIMAL_NUTRITION_REQ_TABLE_H
#define ANIMAL_NUTRITION_REQ_TABLE_H

#include <QString>
#include "model/animal_table.h"

#define ANIMAL_NUTRITION_REQ_DESC_SIZE  50

class AnimalTable;

//Built-in calculation data
enum ANIMAL_TYPE {
    ANIMAL_TYPE_STEER_MEDIUM,
    ANIMAL_TYPE_STEER_LARGE,
    ANIMAL_TYPE_BULL_LARGE,
    ANIMAL_TYPE_HEIFER_MEDIUM,
    ANIMAL_TYPE_HEIFER_LARGE,
    ANIMAL_TYPE_CUSTOM,
    ANIMAL_TYPE_COUNT
};


struct AnimalEnergyEqnData {
    QString info;
    float   negCoeff;
    float   negExp;
    float   weightExp;
};


struct AnimalNutritionReq  {
    bool    animalValid = false;
    char    animal[ANIMAL_NAME_SIZE];
    bool    expectedWeightValid = false;
    bool    currentWeightValid = false;
    float   currentWeight;
    bool    dailyGainValid = false;
    float   dailyGain;
    bool    dmiValid = false;
    float   dmi;
    bool    proteinValid = false;
    float   protein;
    bool    nemValid = false;
    float   nem;
    bool    negValid = false;
    float   neg;
    bool    calciumValid = false;
    float   calcium;
    bool    phosphorusValid = false;
    float   phosphorus;
    bool    vitaValid   = false;
    float   vita;

    int validate(QStringList& detailedText) const;
};

class AnimalNutritionReqTable : public SqlTableModel {
public:

    enum COLUMNS {
        COL_ANIMAL,
        COL_WEIGHT_MATURE,
        COL_WEIGHT_CURRENT,
        COL_DAILYGAIN,
        COL_DMI,
        COL_TDN,
        COL_NEM,
        COL_NEG,
        COL_PROTEIN,
        COL_CALCIUM,
        COL_PHOSPHORUS,
        COL_VITA,
        COL_DYNAMIC
    };

                                AnimalNutritionReqTable(QObject* parent, QSqlDatabase database);
    virtual auto                flags(const QModelIndex &) const -> Qt::ItemFlags override;

    void                        insertHeaderData(void);
    //AnimalNutritionReq          nutritionReqFromRow(int row);
    //int                         rowFromDesc(QString desc);

    static const AnimalEnergyEqnData builtinEnergyTable[ANIMAL_TYPE_COUNT];


    void            setAnimalTable(AnimalTable* table);
    AnimalTable*    getAnimalTable(void) const;
    float           getGreatestMatureWeight(void) const;


private:
    AnimalTable* _animalTable = nullptr;

};


#endif // ANIMAL_NUTRITION_REQ_TABLE_H
