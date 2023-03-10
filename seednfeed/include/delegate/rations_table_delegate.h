#ifndef RATIONS_TABLE_DELEGATE_H
#define RATIONS_TABLE_DELEGATE_H

#include <QStyledItemDelegate>

class IngredientsTable;

class RationsTableDelegate: public QStyledItemDelegate {
private:
    static IngredientsTable*       ingredientsTable;
public:
                        RationsTableDelegate(QObject *parent = nullptr);

    static void         setIngredientsTable(IngredientsTable* table);

    virtual QWidget*    createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual void        setEditorData(QWidget *editor, const QModelIndex &index) const override;
    virtual void        setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    static void         _updateReadOnlyColumns(QAbstractItemModel* model, const QModelIndex& index);
};


#endif // RATION_TABLE_DELEGATE_H
