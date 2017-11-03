#include "delegate/duplicate_column_model_validator.h"
#include <QAbstractItemModel>

DuplicateColumnModelValidator::DuplicateColumnModelValidator(int col, const QAbstractItemModel* model):
    _column(col),
    _model(model)
{}

QValidator::State DuplicateColumnModelValidator::validate(QString &input, int& /*pos*/) const {
    Q_ASSERT(_column != -1 && _model);

    if(input.isNull() || input.isEmpty()) {
        return QValidator::Intermediate;
    } else {
        for(int r = 0; r < _model->rowCount(); ++r) {
            if(_model->data(_model->index(r, _column), Qt::EditRole).toString() == input) {
                return QValidator::Intermediate;
            }
        }
    }
    return QValidator::Acceptable;

}
