#ifndef DYNAMIC_MODEL_COLUMN_COMBOBOX_H
#define DYNAMIC_MODEL_COLUMN_COMBOBOX_H

#include <QComboBox>

class QAbstractItemModel;

class DynamicModelColumnComboBox: public QComboBox {
private:
    int                 _column = -1;
    QAbstractItemModel* _model  = nullptr;
public:
                        DynamicModelColumnComboBox(QWidget* parent=nullptr);
    void                setModelColumn(QAbstractItemModel* model, int col);
    void                populate(void);


    //Overridden from QComboBox
    virtual void        showPopup(void) override;
};

#endif // DYNAMIC_MODEL_COLUMN_COMBOBOX_H
