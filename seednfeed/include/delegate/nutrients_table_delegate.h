#ifndef NUTRIENTS_TABLE_DELEGATE_H
#define NUTRIENTS_TABLE_DELEGATE_H

#include <QStyledItemDelegate>

class NutrientsTableDelegate: public QStyledItemDelegate {
public:
                        NutrientsTableDelegate(QObject *parent = nullptr);

    virtual QWidget*    createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual void        setEditorData(QWidget *editor, const QModelIndex &index) const override;
    virtual void        setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};



#endif // NUTRIENTS_TABLE_DELEGATE_H
