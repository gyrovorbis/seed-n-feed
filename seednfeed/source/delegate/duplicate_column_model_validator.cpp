#include "delegate/duplicate_column_model_validator.h"
#include <QAbstractItemModel>

//===== STATICS =====

constexpr const char DuplicateColumnModelValidator::_invalidSqlChars[];

bool DuplicateColumnModelValidator::validateSqlName(QString input) {
    for(unsigned i = 0; i < sizeof(_invalidSqlChars); ++i) {
        if(input.contains(_invalidSqlChars[i])) {
            return false;
        }
    }
    return true;
}


//===== NONSTATIC =====


DuplicateColumnModelValidator::DuplicateColumnModelValidator(int col, const QAbstractItemModel* model, bool sqlValidation):
    _column(col),
    _model(model),
    _sqlValidation(sqlValidation)
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

        if(_sqlValidation && !validateSqlName(input)) return QValidator::Invalid;
    }
    return QValidator::Acceptable;
}



void DuplicateColumnModelValidator::setSqlValidationEnabled(bool val) {
    _sqlValidation = val;
}
