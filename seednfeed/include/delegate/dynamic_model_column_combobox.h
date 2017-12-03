#ifndef DYNAMIC_MODEL_COLUMN_COMBOBOX_H
#define DYNAMIC_MODEL_COLUMN_COMBOBOX_H

#include <QComboBox>

class QAbstractItemModel;

class DynamicModelColumnComboBox: public QComboBox {
private:
    int                 _srcColumn          = -1;
    QAbstractItemModel* _srcModel           =  nullptr;
    int                 _dstColumn          = -1;
    QAbstractItemModel* _dstModel           =  nullptr;
    bool                _allowDuplicates    = true;
public:
                        DynamicModelColumnComboBox(QWidget* parent=nullptr);
    void                setSrcModelColumn(QAbstractItemModel* model, int col);
    void                setDstModelColumn(QAbstractItemModel* model, int col);
    void                populate(void);


    bool                areDuplicatesAllowed(void) const;
    void                setAllowDuplicates(const bool val=true);


    //Overridden from QComboBox
    virtual void        showPopup(void) override;
};

#endif // DYNAMIC_MODEL_COLUMN_COMBOBOX_H
