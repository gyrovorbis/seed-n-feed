#ifndef SQL_TABLE_MANAGER_WIDGET_H
#define SQL_TABLE_MANAGER_WIDGET_H

#include <QWidget>

class SqlTableView;

namespace Ui {
    class SqlTableManagerWidget;
};


class SqlTableManagerWidget: public QWidget {
Q_OBJECT
public:
    explicit                    SqlTableManagerWidget(QString title1, QWidget* parent=nullptr);

    SqlTableView*               getView(void);

signals:

    void                        addClicked(bool);
    void                        removeClicked(bool);

private:

    Ui::SqlTableManagerWidget* _ui = nullptr;


};


#endif // SQL_TABLE_MANAGER_WIDGET_H
