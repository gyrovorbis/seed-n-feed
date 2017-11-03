#ifndef ANIMAL_NUTRITION_REQ_TABLE_H
#define ANIMAL_NUTRITION_REQ_TABLE_H
#include <QString>
#include <QSqlTableModel>

#define ANIMAL_NUTRITION_REQ_DESC_SIZE  50

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


struct AnimalNutritionReq {
    bool    descValid = false;
    char    desc[ANIMAL_NUTRITION_REQ_DESC_SIZE];
    bool    typeValid = false;
    int     type;
    bool    weightValid = false;
    float   weight;
    bool    dailyGainValid = false;
    float   dailyGain;
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

class AnimalNutritionReqTable : public QSqlTableModel
{
public:

    enum COLUMNS {
        COL_DESC,
        COL_TYPE,
        COL_WEIGHT,
        COL_DAILYGAIN,
        COL_PROTEIN,
        COL_NEM,
        COL_NEG,
        COL_CALCIUM,
        COL_PHOSPHORUS,
        COL_VITA
    };

                                AnimalNutritionReqTable(QObject* parent, QSqlDatabase database);
    virtual auto                flags(const QModelIndex &) const -> Qt::ItemFlags override;

    void                        insertHeaderData(void);
    AnimalNutritionReq          nutritionReqFromRow(int row);
    int                         rowFromDesc(QString desc);

    static const AnimalEnergyEqnData builtinEnergyTable[ANIMAL_TYPE_COUNT];

};


#endif // ANIMAL_NUTRITION_REQ_TABLE_H
