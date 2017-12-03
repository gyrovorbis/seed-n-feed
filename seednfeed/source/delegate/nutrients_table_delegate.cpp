#include "delegate/nutrients_table_delegate.h"
#include "model/nutrient_table.h"
#include "delegate/duplicate_column_model_validator.h"
#include "core/utilities.h"
#include <QLineEdit>
#include <QDoubleValidator>
#include <limits>

NutrientsTableDelegate::NutrientsTableDelegate(QObject *parent):
    QStyledItemDelegate(parent)
{

}

QWidget* NutrientsTableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem& /*option*/, const QModelIndex &index) const {
  QLineEdit *lineEdit = new QLineEdit(parent);
  QValidator* validator;
  lineEdit->setAutoFillBackground(true);
  lineEdit->installEventFilter(const_cast<NutrientsTableDelegate*>(this));

  switch(index.column()) {
  case NutrientTable::COL_NAME:
      validator = new DuplicateColumnModelValidator(NutrientTable::COL_NAME, index.model(), true);
      break;
  default:
      validator = new QDoubleValidator(0.0, std::numeric_limits<double>::max(), DOUBLE_VALIDATOR_DECIMALS_MAX);
  }

  lineEdit->setValidator(validator);

  return lineEdit;
}
void NutrientsTableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    QString value = index.model()->data(index, Qt::DisplayRole).toString();
    static_cast<QLineEdit*>(editor)->setText(value);
}
void NutrientsTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    model->setData(index, static_cast<QLineEdit*>(editor)->text());
}

