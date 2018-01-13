#include <QMessageBox>
#include "delegate/dynamic_model_column_combobox.h"
#include "model/sql_table.h"

DynamicModelColumnComboBox::DynamicModelColumnComboBox(QWidget *parent):
    QComboBox(parent)
{}

void DynamicModelColumnComboBox::populate(void) {
    QStringList list;
    QString previousText;
    Q_ASSERT(_srcColumn != -1 && _srcModel);


    if(!currentText().isNull() && !currentText().isEmpty()) {
        previousText = currentText();
    }

    //clear existing items
    blockSignals(true);
    while(count() > 0) removeItem(0);

    _srcModel->protectedSelect();
    _srcModel->pushFilter();

    //repopulate based on current table values
    for(int r = 0; r < _srcModel->rowCount(); ++r) {
        auto idx = _srcModel->index(r, _srcColumn);
        Q_ASSERT(idx.isValid());
        if(!_filterCb || (_filterCb && _filterCb(idx))) {
            //addItem(idx.data().toString())
            QString str = idx.data().toString();
            if(!list.contains(str)) list << str;
        }
    }

    for(int i = 0; i < list.count(); ++i) {
        addItem(list[i]);
        setItemData(count()-1, list[i], Qt::ToolTipRole);
    }

    blockSignals(false);


    if(!previousText.isNull() && !previousText.isEmpty()) setCurrentText(previousText);
    else if(count()) {
        setCurrentText("");
        setCurrentIndex(0);
    }

    _srcModel->popFilter();

}

void DynamicModelColumnComboBox::showPopup(void) {
    populate();
    if(count() == 0) {
        if(!_emptyErrStr.isNull()) {
            QMessageBox::critical(nullptr, "No Data Found", _emptyErrStr);
        }
    }
    QComboBox::showPopup();
}

void DynamicModelColumnComboBox::setSrcModelColumn(SqlTableModel *model, int col) {
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
