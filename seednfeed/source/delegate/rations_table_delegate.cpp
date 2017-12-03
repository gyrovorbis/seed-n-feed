#include "delegate/rations_table_delegate.h"
#include "model/ration_table.h"
#include "model/ingredients_table.h"
#include "core/utilities.h"
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
            const auto* rationTable = static_cast<const RationTable*>(index.model());
            QStringList stringList = rationTable->getUnusedIngredientsList();
            QString prevValue = index.data().toString();
            if(!prevValue.isNull() && !prevValue.isEmpty()) stringList.push_back(prevValue);
            QComboBox* comboBox = new QComboBox(parent);
            //comboBox->setEditable(true);

            for(auto&& it : stringList) {
                comboBox->addItem(it, it);
            }

            QCompleter* completer = new QCompleter(stringList);

            comboBox->setCompleter(completer);

            return comboBox;
        }
        case RationTable::COL_DM:
        case RationTable::COL_COST_PER_DAY:
            return nullptr;
        default: {
            QLineEdit *lineEdit = new QLineEdit(parent);
            lineEdit->setValidator(new QDoubleValidator(0.01, std::numeric_limits<double>::max(), DOUBLE_VALIDATOR_DECIMALS_MAX));
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

    } //else Q_ASSERT(false);


}
