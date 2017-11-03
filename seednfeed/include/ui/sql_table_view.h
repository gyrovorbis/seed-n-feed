#ifndef SQL_TABLE_VIEW_H
#define SQL_TABLE_VIEW_H

#include <QTableView>

class SqlTableView: public QTableView {
    Q_OBJECT
public:

    explicit    SqlTableView(QWidget* parent=nullptr);
    virtual     ~SqlTableView(void) = default;

public slots:

    void        onCustomContextMenu(const QPoint& point);

};

#endif // SQL_TABLE_VIEW_H
