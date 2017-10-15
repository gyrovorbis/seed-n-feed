#include "ingredients_table_delegate.h"
#include "ingredients_table.h"
#include "duplicate_column_model_validator.h"
#include <QLineEdit>
#include <QDoubleValidator>
#include <limits>

IngredientsTableDelegate::IngredientsTableDelegate(QObject *parent): QStyledItemDelegate(parent)
{}

QWidget* IngredientsTableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem& /*option*/, const QModelIndex &index) const {
  QLineEdit *lineEdit = new QLineEdit(parent);
  QValidator* validator;
  lineEdit->setAutoFillBackground(true);
  lineEdit->installEventFilter(const_cast<IngredientsTableDelegate*>(this));

  switch(index.column()) {
  case IngredientsTable::COL_NAME:
      validator = new DuplicateColumnModelValidator(IngredientsTable::COL_NAME, index.model());
      break;
  default:
      validator = new QDoubleValidator(0.0, std::numeric_limits<double>::max(), 10);
  }

  lineEdit->setValidator(validator);

  return lineEdit;
}
void IngredientsTableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    QString value = index.model()->data(index, Qt::DisplayRole).toString();
    static_cast<QLineEdit*>(editor)->setText(value);
}
void IngredientsTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    model->setData(index, static_cast<QLineEdit*>(editor)->text());
}

