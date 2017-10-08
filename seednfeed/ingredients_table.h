#ifndef INGREDIENTS_TABLE_H
#define INGREDIENTS_TABLE_H
#include <QString>
#include <QSqlTableModel>


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
