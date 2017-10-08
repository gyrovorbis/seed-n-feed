#include "rations_table_delegate.h"
#include "ration_table.h"
#include "ingredients_table.h"
#include <QComboBox>
#include <QCompleter>

//===== STATIC =====
IngredientsTable* RationsTableDelegate::ingredientsTable = nullptr;

void RationsTableDelegate::setIngredientsTable(IngredientsTable *table) {
    ingredientsTable = table;
}

//===== INSTANCE =====
RationsTableDelegate::RationsTableDelegate(QObject *parent): QStyledItemDelegate(parent) {

}

#if 0
void RationsTableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

}
QSize RationsTableDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {

}

void RationsTableDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {

}

#endif
QWidget* RationsTableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    switch(index.column()) {
    case RationTable::COL_INGREDIENT: {
        QComboBox* comboBox = new QComboBox(parent);
        //comboBox->setEditable(true);

        int rows = ingredientsTable->rowCount();
        QStringList stringList;
        for(unsigned i = 0; i < rows; ++i) stringList.append(ingredientsTable->index(i, 0).data().toString());

        for(auto&& it : stringList) {
            comboBox->addItem(it, it);
        }

        QCompleter* completer = new QCompleter(stringList);

        comboBox->setCompleter(completer);

        return comboBox;

    }
    default:
        return QStyledItemDelegate::createEditor(parent, option, index);
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

     //   float costPerDay = ingredientsTable->index(comboBox->currentIndex, IngredientsTable::COL))

      //  model->setData(index.model()->index(index.row(), RationTable::COL_COST_PER_DAY),);
      //  model->setData(index.model()->index(index.row(), RationTable::COL_DM), );
        break;
    }
    default:
        return QStyledItemDelegate::setEditorData(editor, index);
    };
}

void RationsTableDelegate::_updateReadOnlyColumns(int row) {


}
