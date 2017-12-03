#include "model/animal_nutrition_req_table.h"
#include "delegate/animal_nutrition_table_delegate.h"
#include "delegate/duplicate_column_model_validator.h"
#include "core/utilities.h"
#include <QLineEdit>
#include <QDebug>
#include <QComboBox>
#include <QDoubleValidator>
#include <QApplication>
#include <limits>

AnimalNutritionTableDelegate::AnimalNutritionTableDelegate(QObject *parent):
    QStyledItemDelegate(parent)
{}

QWidget* AnimalNutritionTableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem& /*option*/, const QModelIndex &index) const {

    if(index.column() == AnimalNutritionReqTable::COL_TYPE) {
        QComboBox* combo = new QComboBox(parent);
        for(unsigned i = 0; i < ANIMAL_TYPE_CUSTOM; ++i) {
            combo->addItem(AnimalNutritionReqTable::builtinEnergyTable[i].info);
        }
        combo->addItem("Custom");
        return combo;

    }  else {

        QLineEdit *lineEdit = new QLineEdit(parent);
        QValidator* validator;
        lineEdit->setAutoFillBackground(true);
        lineEdit->installEventFilter(const_cast<AnimalNutritionTableDelegate*>(this));

        switch(index.column()) {
        case AnimalNutritionReqTable::COL_DESC:
            validator = new DuplicateColumnModelValidator(AnimalNutritionReqTable::COL_DESC, index.model());
            break;
        default:
            validator = new QDoubleValidator(0.0, std::numeric_limits<double>::max(), DOUBLE_VALIDATOR_DECIMALS_MAX);
        }

        lineEdit->setValidator(validator);
        return lineEdit;

    }
}
void AnimalNutritionTableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    if(index.column() == AnimalNutritionReqTable::COL_TYPE) {
        int i = index.model()->data(index, Qt::DisplayRole).toInt();
        static_cast<QComboBox*>(editor)->setCurrentIndex(i);
    } else {
        QString value = index.model()->data(index, Qt::DisplayRole).toString();
        static_cast<QLineEdit*>(editor)->setText(value);
    }
}

void AnimalNutritionTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    if(index.column() == AnimalNutritionReqTable::COL_TYPE) {
        model->setData(index, static_cast<QComboBox*>(editor)->currentIndex(), Qt::EditRole);
        model->setData(index, static_cast<QComboBox*>(editor)->currentText(), Qt::DisplayRole);
    }
    else model->setData(index, static_cast<QLineEdit*>(editor)->text());
}

void AnimalNutritionTableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyleOptionViewItem itemOption(option);
    initStyleOption(&itemOption, index);

    switch(index.column()) {
        case AnimalNutritionReqTable::COL_TYPE: {
            bool ok;
            int typeVal = index.data(Qt::EditRole).toInt(&ok);
            if(/*!ok || */typeVal >= ANIMAL_TYPE_COUNT) {
                qCritical() << "Invalid AnimalNutritionReq::TYPE " << typeVal;
            } else {
                itemOption.text = AnimalNutritionReqTable::builtinEnergyTable[typeVal].info;
                QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &itemOption, painter, nullptr);
                break;
            }
        }
        default:
            QStyledItemDelegate::paint(painter, itemOption, index);
            break;
    }
}

