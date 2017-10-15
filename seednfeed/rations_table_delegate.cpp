#include "rations_table_delegate.h"
#include "ration_table.h"
#include "ingredients_table.h"
#include <QComboBox>
#include <QCompleter>
#include <QLineEdit>
#include <cmath>

//===== STATIC =====
IngredientsTable* RationsTableDelegate::ingredientsTable = nullptr;

void RationsTableDelegate::setIngredientsTable(IngredientsTable *table) {
    ingredientsTable = table;
}

//===== INSTANCE =====
RationsTableDelegate::RationsTableDelegate(QObject *parent):
    QStyledItemDelegate(parent)
{}

QWidget* RationsTableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const {
    switch(index.column()) {
        case RationTable::COL_INGREDIENT: {
            QComboBox* comboBox = new QComboBox(parent);
            //comboBox->setEditable(true);

            int rows = ingredientsTable->rowCount();
            QStringList stringList;
            for(int i = 0; i < rows; ++i) stringList.append(ingredientsTable->index(i, 0).data().toString());

            for(auto&& it : stringList) {
                comboBox->addItem(it, it);
            }

            QCompleter* completer = new QCompleter(stringList);

            comboBox->setCompleter(completer);

            return comboBox;
        }
        default: {
            QLineEdit *lineEdit = new QLineEdit(parent);
            lineEdit->setValidator(new QDoubleValidator(0.01, std::numeric_limits<double>::max(), 10));
            return lineEdit;
        }
    }
}
void RationsTableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    switch(index.column()) {
    case RationTable::COL_INGREDIENT: {
        QString value = index.model()->data(index, Qt::DisplayRole).toString();
        static_cast<QComboBox*>(editor)->setEditText(value);
        break;
    }
    default:
        return QStyledItemDelegate::setEditorData(editor, index);
    };


}
void RationsTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    switch(index.column()) {
    case RationTable::COL_INGREDIENT: {
        QComboBox* comboBox = static_cast<QComboBox*>(editor);
        model->setData(index, comboBox->currentText());
        break;
    }
    default:
        QStyledItemDelegate::setModelData(editor, model, index);
    };
    _updateReadOnlyColumns(model, index);
}

void RationsTableDelegate::_updateReadOnlyColumns(QAbstractItemModel* model, const QModelIndex& index) {
    RationTable* rationTable = static_cast<RationTable*>(model);
    Ration ration = rationTable->rationFromRow(index.row());
    int row = ingredientsTable->rowFromName(ration.ingredient);

    if(row != -1) {
        Ingredient ingredient = ingredientsTable->ingredientFromRow(row);
        float costPerDay    = (ration.asFed*ration.costPerUnit)/ration.weight;
        float dm            = (ration.asFed*ingredient.dm);

        if(std::isnan(costPerDay) || std::isinf(costPerDay)) {
            rationTable->setData(rationTable->index(index.row(), RationTable::COL_COST_PER_DAY), QString());
        } else {
            rationTable->setData(rationTable->index(index.row(), RationTable::COL_COST_PER_DAY), costPerDay);
        }

        if(std::isnan(dm) || std::isinf(dm)) {
            rationTable->setData(rationTable->index(index.row(), RationTable::COL_DM), QString());
        } else {
            rationTable->setData(rationTable->index(index.row(), RationTable::COL_DM), dm);
        }

    }


}
