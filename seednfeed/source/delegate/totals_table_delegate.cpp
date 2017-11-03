#include "delegate/totals_table_delegate.h"
#include <QPainter>

TotalsTableDelegate::TotalsTableDelegate(QObject* parent):
    QStyledItemDelegate(parent)
{}

void TotalsTableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyleOptionViewItem itemOption(option);
    initStyleOption(&itemOption, index);

    if(index.column() == TOTALS_TABLE_DIFF_COLUMN) {
        itemOption.features &= ~QStyleOptionViewItem::HasDecoration;
        QLinearGradient gradient(QPointF(option.rect.left(), option.rect.center().y()), QPointF(option.rect.right(), option.rect.center().y()));
        QColor bgColor = index.data(Qt::BackgroundRole).value<QColor>();
        gradient.setColorAt(0.0, bgColor);
        gradient.setColorAt(1.0, Qt::transparent);
        painter->fillRect(itemOption.rect, gradient);
    }

    QStyledItemDelegate::paint(painter, itemOption, index);

}
