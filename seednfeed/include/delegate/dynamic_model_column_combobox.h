#ifndef DYNAMIC_MODEL_COLUMN_COMBOBOX_H
#define DYNAMIC_MODEL_COLUMN_COMBOBOX_H

#include <QComboBox>
#include <functional>

class QAbstractItemModel;
class SqlTableModel;

class DynamicModelColumnComboBox: public QComboBox {
private:
    int                 _srcColumn          = -1;
    SqlTableModel*      _srcModel           =  nullptr;
    int                 _dstColumn          = -1;
    QAbstractItemModel* _dstModel           =  nullptr;
    bool                _allowDuplicates    = true;
    std::function<bool(const QModelIndex&)>
                        _filterCb;
    QString             _emptyErrStr;
public:
                        DynamicModelColumnComboBox(QWidget* parent=nullptr);
    void                setSrcModelColumn(SqlTableModel* model, int col);
    void                setDstModelColumn(QAbstractItemModel* model, int col);
    template <typename F>
    void                setFilterCallback(F&& func);
    void                populate(void);

    bool                areDuplicatesAllowed(void) const;
    void                setAllowDuplicates(const bool val=true);
    void                setEmptyErrorString(QString msg);



    //Overridden from QComboBox
    virtual void        showPopup(void) override;
};






//========== INLINEZ ==========
template<typename F>
inline void DynamicModelColumnComboBox::setFilterCallback(F&& func) {
    _filterCb = std::forward<F>(func);
}

inline void DynamicModelColumnComboBox::setEmptyErrorString(QString msg) {
    _emptyErrStr = msg;
}


#endif // DYNAMIC_MODEL_COLUMN_COMBOBOX_H
