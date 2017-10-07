#ifndef ANIMALPROJECTION_H
#define ANIMALPROJECTION_H
#include <QVector>
#include "ration_table.h"


class AnimalProjection
{
    int id = 0;
    int totalHead = 0;
    float currentBodyWeightIbs = 0.0;
    float expectedSellWeightIbs = 0.0;
    float slaughterWeightsIbs = 0.0;
    int expectedEndorsementLengthWks = 0;
    //QVector<Ration> rations;

public:
    AnimalProjection();
};

#endif // ANIMALPROJECTION_H
