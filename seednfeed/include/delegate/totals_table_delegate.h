#ifndef TOTALS_TABLE_DELEGATE_H
#define TOTALS_TABLE_DELEGATE_H

#include <QStyledItemDelegate>

#define TOTALS_TABLE_DIFF_GREEN_RANGE   0.05f
#define TOTALS_TABLE_DIFF_YELLOW_RANGE  0.1f
#define TOTALS_TABLE_DIFF_COLUMN        2

class TotalsTableDelegate: public QStyledItemDelegate {
private:
public:
                        TotalsTableDelegate(QObject* parent = nullptr);

    virtual void        paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};


#endif // TOTALS_TABLE_DELEGATE_H
