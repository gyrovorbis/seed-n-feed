#ifndef RATION_TABLE_H
#define RATION_TABLE_H
#include <QString>
#include <QStandardItemModel>

#define RATION_ITEM_TYPE_SIZE   50

class RationsItem {

};

class RationTable : public QStandardItemModel {

public:
    enum COLUMNS {
        COL_INGREDIENT,
        COL_AS_FED,
        COL_COST_PER_UNIT,
        COL_WEIGHT,
        COL_COST_PER_DAY,
        COL_DM
    };

                            RationTable(QObject* parent=nullptr);
    virtual Qt::ItemFlags   flags(const QModelIndex &index) const override;
};

#endif // RATION_TABLE_H
