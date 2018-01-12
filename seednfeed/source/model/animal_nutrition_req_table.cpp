#include "model/animal_nutrition_req_table.h"
#include "core/utilities.h"

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
        0.8936f,4
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
    SqlTableModel(parent, db)
{
    setEditStrategy(QSqlTableModel::OnFieldChange);
}

Qt::ItemFlags AnimalNutritionReqTable::flags(const QModelIndex &) const {
    return (Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsEditable);
}

void AnimalNutritionReqTable::insertHeaderData(void) {
    addHeaderData(COL_ANIMAL,           Qt::Horizontal, "Animal");
    addHeaderData(COL_WEIGHT_MATURE,    Qt::Horizontal, "Wm", "Mature (Expected Fullly Grown) Weight (lbs)");
    addHeaderData(COL_WEIGHT_CURRENT,   Qt::Horizontal, "Wc", "Current or Starting Weight (lbs)");
    addHeaderData(COL_DAILYGAIN,        Qt::Horizontal, "ADG", "Average Daily Gain (lbs/day)");
    addHeaderData(COL_DMI,              Qt::Horizontal, "DMI", "Daily Dry Matter Intake (lbs/day)");
    addHeaderData(COL_TDN,              Qt::Horizontal, "TDN", "Daily Total Digestible Nutrients (% of DMI)");
    addHeaderData(COL_NEM,              Qt::Horizontal, "NEm", "Daily Energy Required for Maintenance (MCal/day)");
    addHeaderData(COL_NEG,              Qt::Horizontal, "NEg", "Daily Energy Required for Gain (MCal/day)");
    addHeaderData(COL_PROTEIN,          Qt::Horizontal, "CP", "Daily Crude Protein (% of DMI)");
    addHeaderData(COL_CALCIUM,          Qt::Horizontal, "Ca", "Daily Calcium Requirement (% of DMI)");
    addHeaderData(COL_PHOSPHORUS,       Qt::Horizontal, "P", "Daily Phospohorus Requirements (% of DMI)");
    addHeaderData(COL_VITA,             Qt::Horizontal, "VitA");
}
#if 0
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


#endif
#if 0

int AnimalNutritionReqTable::rowFromDesc(QString desc) {
    for(int r = 0; r < rowCount(); ++r) {
        if(index(r, COL_DESC).data().toString() == desc) {
            return r;
        }
    }
    return -1;
}
#endif

int AnimalNutritionReq::validate(QStringList& detailedText) const {
    int errors = 0;

#if 0
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
#endif
    return errors;

}

void AnimalNutritionReqTable::setAnimalTable(AnimalTable *table) { _animalTable = table; }

AnimalTable* AnimalNutritionReqTable::getAnimalTable(void) const { return _animalTable; }

float AnimalNutritionReqTable::getGreatestMatureWeight(void) const {
    float max = 1.0f;
    iterateForColumnVariants(COL_WEIGHT_MATURE, [&](int, QVariant data) {
        bool ok;
        float val = data.toFloat(&ok);
        if(ok && val > max) max = val;
    });
    return max;
}
