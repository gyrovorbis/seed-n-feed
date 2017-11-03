#ifndef INGREDIENTS_TABLE_DELEGATE_H
#define INGREDIENTS_TABLE_DELEGATE_H

#include <QStyledItemDelegate>

class IngredientsTableDelegate: public QStyledItemDelegate {
public:
                        IngredientsTableDelegate(QObject *parent = nullptr);

    virtual QWidget*    createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual void        setEditorData(QWidget *editor, const QModelIndex &index) const override;
    virtual void        setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};

#endif // INGREDIENTS_TABLE_DELEGATE_H
