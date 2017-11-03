#ifndef ERROR_WARNING_DIALOG_H
#define ERROR_WARNING_DIALOG_H

#include <QDialog>

namespace Ui {
    class ErrorWarningDialog;
}

class ErrorWarningDialog : public QDialog {
    Q_OBJECT
public:
    explicit    ErrorWarningDialog(QString _name, QWidget *parent=nullptr);
    virtual     ~ErrorWarningDialog(void);

    void        setDescriptionText(QString text);
    void        setEntryList(QStringList list);

private:
    QString                 name;
    Ui::ErrorWarningDialog* ui;
};

#endif // ERROR_WARNING_DIALOG_H
