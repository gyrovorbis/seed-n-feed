#ifndef DUPLICATE_COLUMN_MODEL_VALIDATOR_H
#define DUPLICATE_COLUMN_MODEL_VALIDATOR_H

#include <QValidator>

class QAbstractItemModel;

class DuplicateColumnModelValidator: public QValidator {
private:
    int                         _column = 1;
    const QAbstractItemModel*   _model = nullptr;
public:

                    DuplicateColumnModelValidator(int col, const QAbstractItemModel* model);

    virtual auto    validate(QString& input, int& pos) const -> State override;

};

#endif // NAME_COLUMN_MODEL_VALIDATOR_H
