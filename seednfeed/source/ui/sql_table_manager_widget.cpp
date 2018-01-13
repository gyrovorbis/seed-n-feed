#include "ui/sql_table_manager_widget.h"
#include "ui_sql_table_manager_widget.h"

SqlTableManagerWidget::SqlTableManagerWidget(QString title, QWidget *parent):
    QWidget(parent),
    _ui(new Ui::SqlTableManagerWidget)
{
    _ui->setupUi(this);
    _ui->titleLabel->setText(title);

    connect(_ui->addButton, SIGNAL(clicked(bool)), this, SIGNAL(addClicked(bool)));
    connect(_ui->removeButton, SIGNAL(clicked(bool)), this, SIGNAL(removeClicked(bool)));
}

SqlTableView* SqlTableManagerWidget::getView(void) {
    return _ui->tableView;
}
