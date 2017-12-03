#ifndef RECIPE_TABLE_DELEGATE_H
#define RECIPE_TABLE_DELEGATE_H

#include <QStyledItemDelegate>

class RecipeTableDelegate: public QStyledItemDelegate {
public:
                        RecipeTableDelegate(QObject *parent = nullptr);

    virtual QWidget*    createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual void        setEditorData(QWidget *editor, const QModelIndex &index) const override;
    virtual void        setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};



#endif // RECIPE_TABLE_DELEGATE_H
