#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QDir>

#define DATABASE_NAME      "seed_and_feed.db"
#define DATABASE_TYPE      "QSQLITE"
#define DEBUG_LOG_NAME     "debug_log.txt"
#define USER_DIR_NAME      "seednfeed"

class QToolButton;
class QAbstractItemModel;

namespace Ui {
    class MainWindow;
}

class ErrorWarningDialog;

class IngredientsTable;
class RationTable;
class AnimalNutritionReqTable;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum LOG_SEVERITY {
        LOG_VERBOSE,
        LOG_WARNING,
        LOG_ERROR,
    };

    explicit            MainWindow(QWidget* parent = nullptr);
    virtual             ~MainWindow(void);

    void                setStatusBarWarnings(QStringList list);
    void                setStatusBarErrors(QStringList list);

private:
    Ui::MainWindow*          _ui;
    QSqlDatabase             _db;
    IngredientsTable*        _ingredientsTable          = nullptr;
    AnimalNutritionReqTable* _animalNutritionReqTable   = nullptr;
    RationTable*             _rationTable               = nullptr;
    QDir                     _userDir;
    QString                  _dbPath;
    static FILE*             _dbgLogFile;

    QToolButton*             _statusWarnButton;
    QToolButton*             _statusErrorButton;
    QStringList              _warningList;
    QStringList              _errorList;
    ErrorWarningDialog*      _errorDialog;
    ErrorWarningDialog*      _warnDialog;

    virtual void        closeEvent(QCloseEvent *event) override;

    bool                _debugInit(void);
    bool                _dbInit(QString type=DATABASE_TYPE);
    bool                _tableInit(void);
    void                _printBuildInfo(void);
    void                _validateCalculationInputs(void);
    void                _clearTotalsTable(void);
    void                _clearCalculationTable(void);
    void                _initStatusBar(void);

    static void         logQ(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    static void         dbgPrintf(const char* str, ...);
    static void         _writeLog(const char* str);

private slots:
    void                onAddIngredientClick(bool);
    void                onDeleteIngredientClick(bool);

    void                onAddRationClick(bool);
    void                onDeleteRationClick(bool);

    void                onAddAnimalNutritionReqClick(bool);
    void                onDeleteAnimalNutritionReqClick(bool);

    void                onIngredientsDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);
    void                onCalculateButtonClick(bool);


    void                on_actionExport_triggered();
    void                on_actionImport_triggered();
};

#endif // MAINWINDOW_H