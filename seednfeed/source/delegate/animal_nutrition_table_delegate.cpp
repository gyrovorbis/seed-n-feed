#include "model/animal_nutrition_req_table.h"
#include "delegate/animal_nutrition_table_delegate.h"
#include "delegate/dynamic_model_column_combobox.h"
#include "delegate/duplicate_column_model_validator.h"
#include "core/utilities.h"
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleValidator>
#include <QApplication>
#include <limits>

using ReqTable = AnimalNutritionReqTable;

AnimalNutritionTableDelegate::AnimalNutritionTableDelegate(QObject *parent):
    QStyledItemDelegate(parent)
{}

QWidget* AnimalNutritionTableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem& /*option*/, const QModelIndex &index) const {
     auto* reqTable = static_cast<const AnimalNutritionReqTable*>(index.model());
    if(index.column() == AnimalNutritionReqTable::COL_ANIMAL) {

        auto* combo = new DynamicModelColumnComboBox(parent);
        combo->setSrcModelColumn(reqTable->getAnimalTable(), AnimalTable::COL_NAME);
        return combo;
    }  else {

        QLineEdit *lineEdit = new QLineEdit(parent);
        QValidator* validator;
        lineEdit->setAutoFillBackground(true);
        lineEdit->installEventFilter(const_cast<AnimalNutritionTableDelegate*>(this));

        switch(index.column()) {
        case ReqTable::COL_WEIGHT_MATURE:
            validator = new QDoubleValidator(reqTable->getVariant(index.row(), ReqTable::COL_WEIGHT_CURRENT).toDouble(), std::numeric_limits<double>::max(), DOUBLE_VALIDATOR_DECIMALS_MAX);
            break;
        case ReqTable::COL_WEIGHT_CURRENT:
            validator = new QDoubleValidator(0.0, reqTable->getVariant(index.row(), ReqTable::COL_WEIGHT_MATURE).toDouble(), DOUBLE_VALIDATOR_DECIMALS_MAX);
            break;
        case ReqTable::COL_DMI:
        case ReqTable::COL_TDN:
            validator = new QDoubleValidator(0.0, 100.0, DOUBLE_VALIDATOR_DECIMALS_MAX);
            break;
        default:
            validator = new QDoubleValidator(0.0, std::numeric_limits<double>::max(), DOUBLE_VALIDATOR_DECIMALS_MAX);
            break;
        }

        lineEdit->setValidator(validator);
        return lineEdit;

    }

}
void AnimalNutritionTableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {

    if(index.column() == AnimalNutritionReqTable::COL_ANIMAL) {
        QString name = index.data().toString();
        int index = static_cast<QComboBox*>(editor)->findText(name);
        static_cast<QComboBox*>(editor)->setEditText(name);
    } else {
        QString value = index.model()->data(index, Qt::EditRole).toString();
        static_cast<QLineEdit*>(editor)->setText(value);
   }

}

void AnimalNutritionTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    if(index.column() == AnimalNutritionReqTable::COL_ANIMAL) {
        model->setData(index, static_cast<QComboBox*>(editor)->currentText());
    }
    else  model->setData(index, static_cast<QLineEdit*>(editor)->text());
}

void AnimalNutritionTableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyleOptionViewItem itemOption(option);
    initStyleOption(&itemOption, index);

    switch(index.column()) {

        default:
            QStyledItemDelegate::paint(painter, option, index);
            break;
    }
}

