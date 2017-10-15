#include "dynamic_model_column_combobox.h"
#include <QAbstractItemModel>

DynamicModelColumnComboBox::DynamicModelColumnComboBox(QWidget *parent):
    QComboBox(parent)
{}

void DynamicModelColumnComboBox::populate(void) {
    QString previousText;
    Q_ASSERT(_column != -1 && _model);

    if(!currentText().isNull()) {
        previousText = currentText();
    }

    //clear existing items
    while(count() > 0) removeItem(0);

    //repopulate based on current table values
    for(int r = 0; r < _model->rowCount(); ++r) {
        addItem(_model->data(_model->index(r, _column)).toString());
    }

    if(!previousText.isNull()) setCurrentText(previousText);
}

void DynamicModelColumnComboBox::showPopup(void) {
    populate();
    QComboBox::showPopup();
}

void DynamicModelColumnComboBox::setModelColumn(QAbstractItemModel *model, int col) {
    _column = col;
    _model = model;
}
