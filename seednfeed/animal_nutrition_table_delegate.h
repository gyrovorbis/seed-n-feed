#ifndef ANIMAL_NUTRITION_DELEGATE_H
#define ANIMAL_NUTRITION_DELEGATE_H

#include <QStyledItemDelegate>

class AnimalNutritionTableDelegate: public QStyledItemDelegate {
public:
                        AnimalNutritionTableDelegate(QObject *parent = nullptr);

    virtual QWidget*    createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual void        setEditorData(QWidget *editor, const QModelIndex &index) const override;
    virtual void        setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};

#endif // ANIMAL_NUTRITION_DELEGATE_H
