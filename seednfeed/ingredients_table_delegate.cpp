#include "ingredients_table_delegate.h"
#include <QLineEdit>

IngredientsTableDelegate::IngredientsTableDelegate(QObject *parent): QStyledItemDelegate(parent) {

}

#if 0
void IngredientsTableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

}
QSize IngredientsTableDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {

}

void IngredientsTableDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {

}

#endif
QWidget* IngredientsTableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const {
  QLineEdit *lineEdit = new QLineEdit(parent);
  lineEdit->setAutoFillBackground(true);
  lineEdit->installEventFilter(const_cast<IngredientsTableDelegate*>(this));
  return lineEdit;
}
void IngredientsTableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    QString value = index.model()->data(index, Qt::DisplayRole).toString();
    static_cast<QLineEdit*>(editor)->setText(value);
}
void IngredientsTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    model->setData(index, static_cast<QLineEdit*>(editor)->text());
}

