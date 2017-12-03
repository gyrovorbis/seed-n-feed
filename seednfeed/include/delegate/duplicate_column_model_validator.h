#ifndef DUPLICATE_COLUMN_MODEL_VALIDATOR_H
#define DUPLICATE_COLUMN_MODEL_VALIDATOR_H

#include <QValidator>

class QAbstractItemModel;

class DuplicateColumnModelValidator: public QValidator {
private:
    int                         _column             = 1;
    const QAbstractItemModel*   _model              = nullptr;
    bool                        _sqlValidation      = false;
    static constexpr const char _invalidSqlChars[]  = {
        ']', '[', '-', '*', '/', '\\', '<', '>', ',', '=', '~', '^', ')', '(', '\'', ';', ':',
        '{', '}', '|', '"', '.', '?', '`', '+', '#', '$', '!', '@', '#', '&', '%'
    };

public:
                    DuplicateColumnModelValidator(int col, const QAbstractItemModel* model, bool sqlValidation=false);

    virtual auto    validate(QString& input, int& pos) const -> State override;
    void            setSqlValidationEnabled(bool val=true);

    static bool     validateSqlName(QString input);
};

#endif // NAME_COLUMN_MODEL_VALIDATOR_H
