#include "delegate/dynamic_model_column_combobox.h"
#include <QAbstractItemModel>

DynamicModelColumnComboBox::DynamicModelColumnComboBox(QWidget *parent):
    QComboBox(parent)
{}

void DynamicModelColumnComboBox::populate(void) {
    QString previousText;
    Q_ASSERT(_srcColumn != -1 && _srcModel);

    if(!currentText().isNull() && !currentText().isEmpty()) {
        previousText = currentText();
    }

    //clear existing items
    while(count() > 0) removeItem(0);

    //repopulate based on current table values
    for(int r = 0; r < _srcModel->rowCount(); ++r) {
        addItem(_srcModel->index(r, _srcColumn).data().toString());
    }

    if(!previousText.isNull() && !previousText.isEmpty()) setCurrentText(previousText);
    else if(count()) {
        setCurrentText("");
        setCurrentIndex(0);
    }
}

void DynamicModelColumnComboBox::showPopup(void) {
    populate();
    QComboBox::showPopup();
}

void DynamicModelColumnComboBox::setSrcModelColumn(QAbstractItemModel *model, int col) {
    _srcColumn = col;
    _srcModel = model;
}

void DynamicModelColumnComboBox::setDstModelColumn(QAbstractItemModel *model, int col) {
    _dstColumn = col;
    _dstModel = model;
}

bool DynamicModelColumnComboBox::areDuplicatesAllowed(void) const {
    return _allowDuplicates;
}

void DynamicModelColumnComboBox::setAllowDuplicates(const bool val) {
    _allowDuplicates = val;
}
