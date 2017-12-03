#include "model/animal_nutrition_req_table.h"
#include "core/utilities.h"
#include <QDebug>

const AnimalEnergyEqnData AnimalNutritionReqTable::builtinEnergyTable[] = {
    {
        "Medium-frame steer calves",
        13.91f,
        0.9116f,
        -0.6837
    },{
        "Large-frame steer calves, medium-frame yearling steers, medium-frame bull calves",
        15.54f,
        0.9116f,
        -0.6837f
    },{
        "Large-frame bull calves, large-frame yearling steers",
        17.35f,
        0.9116f,
        -0.6837f
    },{
        "Medium-frame heifer calves",
        10.96f,
        0.8936f,
        -0.6702f
    },{
        "Large-frame heifer calves, yearling heifers",
        12.21f,
        0.8936f,
        -0.6702f
    },{
        "Custom",
        12.21f,
        0.8936f,
        -0.6702f
    }
};


AnimalNutritionReqTable::AnimalNutritionReqTable(QObject* parent, QSqlDatabase db):
    QSqlTableModel(parent, db)
{
    setEditStrategy(QSqlTableModel::OnFieldChange);
}

Qt::ItemFlags AnimalNutritionReqTable::flags(const QModelIndex &) const {
    return (Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsEditable);
}

void AnimalNutritionReqTable::insertHeaderData(void) {
    setHeaderData(COL_DESC,         Qt::Horizontal, "Description");
    setHeaderData(COL_TYPE,         Qt::Horizontal, "Type");
    setHeaderData(COL_WEIGHT,       Qt::Horizontal, "Weight (lb)");
    setHeaderData(COL_DAILYGAIN,    Qt::Horizontal, "Daily Gain (lb/day)");
    setHeaderData(COL_DMI,          Qt::Horizontal, "Dry Matter Intake (lb/day)");
    setHeaderData(COL_NEM,          Qt::Horizontal, "NEm (MCal/day)");
    setHeaderData(COL_NEG,          Qt::Horizontal, "NEg (MCal/day");
    setHeaderData(COL_PROTEIN,      Qt::Horizontal, "Protein (g/day)");
    setHeaderData(COL_CALCIUM,      Qt::Horizontal, "Calcium (g/day)");
    setHeaderData(COL_PHOSPHORUS,   Qt::Horizontal, "Phosphorus (g/day)");
  //  setHeaderData(COL_VITA,         Qt::Horizontal, "VitA (g/day)");
}

AnimalNutritionReq AnimalNutritionReqTable::nutritionReqFromRow(int row) {
    AnimalNutritionReq req;
    memset(&req, 0, sizeof(AnimalNutritionReq));

    if(row < rowCount()) {
        auto name = index(row, COL_DESC).data().toString();
        if(name.isEmpty() || name.isNull()) {
            req.desc[0] = '\0';
            req.descValid = false;
        } else {
            strcpy(req.desc, Q_CSTR(name));
            req.descValid = true;
        }

        req.type        = index(row, COL_TYPE).data().toInt(&req.typeValid);
        req.weight      = index(row, COL_WEIGHT).data().toFloat(&req.weightValid);
        req.dailyGain   = index(row, COL_DAILYGAIN).data().toFloat(&req.dailyGainValid);
        req.dmi         = index(row, COL_DMI).data().toFloat(&req.dmiValid);
        req.protein     = index(row, COL_PROTEIN).data().toFloat(&req.proteinValid);
        req.nem         = index(row, COL_NEM).data().toFloat(&req.nemValid);
        req.neg         = index(row, COL_NEG).data().toFloat(&req.negValid);
        req.calcium     = index(row, COL_CALCIUM).data().toFloat(&req.calciumValid);
        req.phosphorus  = index(row, COL_PHOSPHORUS).data().toFloat(&req.phosphorusValid);
        //req.vita        = index(row, COL_VITA).data().toFloat(&req.vitaValid);

    } else {
        qCritical() << "Requesting invalid AnimalNutritionReqTable row index: " << row;
    }

    return req;
}


int AnimalNutritionReqTable::rowFromDesc(QString desc) {
    for(int r = 0; r < rowCount(); ++r) {
        if(index(r, COL_DESC).data().toString() == desc) {
            return r;
        }
    }
    return -1;
}

int AnimalNutritionReq::validate(QStringList& detailedText) const {
    int errors = 0;
    auto validateField = [&](bool valid, QString name) {
        if(!valid) {
            if(descValid) detailedText += QString("Nutrition Requirements '") + QString(desc) + QString("' - ");
            detailedText += QString("invalid field: ") + name + QString("\n");
            ++errors;
        }
    };

   validateField(descValid,         "Description");
   validateField(typeValid,         "Type");
   validateField(weightValid,       "Weight (lb)");
   validateField(dailyGainValid,    "Avg Daily Gain (lb)");
   validateField(dmiValid,          "Dry Matter Intake (lb/day)");
   validateField(proteinValid,      "Protein (g/day)");
   validateField(nemValid,          "NeM (MCal/day)");
   validateField(negValid,          "NeG (MCal/day)");
   validateField(calciumValid,      "Calcium (g/day)");
   validateField(phosphorusValid,   "Phosphorus (g/day)");
   //validateField(vitaValid,         "Vitamin A (g/day)");

    return errors;

}
