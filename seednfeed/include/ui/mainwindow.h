#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QDir>
#include <QVariant>

#define DATABASE_NAME      "seed_and_feed.db"
#define DATABASE_TYPE      "QSQLITE"
#define DEBUG_LOG_NAME     "debug_log.txt"
#define USER_DIR_NAME      "seednfeed"
#define DEFAULT_DB_PATH     ":/defaults.db"

class QToolButton;
class QAbstractItemModel;

namespace Ui {
    class MainWindow;
}

class ErrorWarningDialog;

class IngredientsTable;
class RationTable;
class RecipeTable;
class AnimalNutritionReqTable;
class NutrientTable;

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
    RecipeTable*             _recipeTable               = nullptr;
    NutrientTable*           _nutrientTable             = nullptr;
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
    void                _updateTotalsTableRows(void);
    bool                _importDb(QString filepath);
    bool                _loadDefaultDatabase(void);

    static void         logQ(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    static void         dbgPrintf(const char* str, ...);
    static void         _writeLog(const char* str);

private slots:
    void                onAddIngredientClick(bool);
    void                onDeleteIngredientClick(bool);

    void                onAddNutrientClick(bool);
    void                onDeleteNutrientClick(bool);

    void                onAddRecipeClick(bool);
    void                onDeleteRecipeClick(bool);
    void                onRecipeSelectionChanged(const QModelIndex& selected, const QModelIndex& deselected);
    void                onRecipeValueChanged(int row, int col, QVariant oldValue, QVariant newValue, int editRole);

    void                onAddRationClick(bool);
    void                onDeleteRationClick(bool);

    void                onAddAnimalNutritionReqClick(bool);
    void                onDeleteAnimalNutritionReqClick(bool);

    void                onCalculateButtonClick(bool);
    void                onAnimalComboBoxChange(QString text);

    void                on_actionExport_triggered();
    void                on_actionImport_triggered();
    void on_actionUse_Default_triggered();
};

#endif // MAINWINDOW_H
