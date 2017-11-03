#include <QStringListModel>

#include "ui/error_warning_dialog.h"
#include "ui_error_warning_dialog.h"

ErrorWarningDialog::ErrorWarningDialog(QString _name, QWidget *parent) :
    QDialog(parent),
    name(_name),
    ui(new Ui::ErrorWarningDialog)
{
    ui->setupUi(this);
}

ErrorWarningDialog::~ErrorWarningDialog(void) {
    delete ui;
}


void ErrorWarningDialog::setDescriptionText(QString text) {
    ui->descLabel->setText(std::move(text));
}

void ErrorWarningDialog::setEntryList(QStringList list) {
    //update the list view
    for(auto& str : list) str.chop(1);
    auto* model = new QStringListModel(std::move(list));
    auto* prev = ui->listView->model();
    ui->listView->setModel(model);
    delete prev;

    //update the general description text
    ui->descLabel->setText(list.size()? name + QString(" table could not be tabulated with the provided animal and rations due to the following issues:") :
                                        QString("All issues have been resolved."));
}
