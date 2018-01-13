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
#define BUILTIN_NUTR_COUNT  8

class QToolButton;
class QAbstractItemModel;
class QTableWidgetItem;

namespace Ui {
    class MainWindow;
}

class ErrorWarningDialog;
class SqlTableManagerWidget;

class IngredientsTable;
class RationTable;
class RecipeTable;
class AnimalNutritionReqTable;
class AnimalTable;
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

    enum MAIN_TABS {
        MAIN_TAB_CALC,
        MAIN_TAB_RECIPES,
        MAIN_TAB_INGREDIENTS,
        MAIN_TAB_ANIMALS,
        MAIN_TAB_NUTRIENTS
    };

    explicit            MainWindow(QWidget* parent = nullptr);
    virtual             ~MainWindow(void);

    void                setStatusBarWarnings(QStringList list);
    void                setStatusBarErrors(QStringList list);

    static void         dbgPrintf(const char* str, ...);
    static void         dbgPush(void);
    static void         dbgPop(int depth=1);

    static auto         _createItem(const char* fmt, ...) -> QTableWidgetItem*;
    static QSqlDatabase getDb(void);
    static MainWindow*  getInstance(void);

public:
    static MainWindow*       _instance;
    Ui::MainWindow*          _ui;
    SqlTableManagerWidget*   _recipeWidget              = nullptr;
    SqlTableManagerWidget*   _rationWidget              = nullptr;
    SqlTableManagerWidget*   _animalWidget              = nullptr;
    SqlTableManagerWidget*   _reqWidget                 = nullptr;
    SqlTableManagerWidget*   _ingWidget                 = nullptr;
    SqlTableManagerWidget*   _nutWidget                 = nullptr;
    static QSqlDatabase      _db;
    IngredientsTable*        _ingredientsTable          = nullptr;
    AnimalNutritionReqTable* _animalNutritionReqTable   = nullptr;
    RationTable*             _rationTable               = nullptr;
    RecipeTable*             _recipeTable               = nullptr;
    NutrientTable*           _nutrientTable             = nullptr;
    AnimalTable*             _animalTable               = nullptr;
    QDir                     _userDir;
    QString                  _dbPath;
    static FILE*             _dbgLogFile;
    static int               _logDepth;

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
    int                 _getCalcAnimalRow(void) const;
    int                 _getCalcRecipeRow(void) const;
    int                 _getCalcNutritionReqsRow(void) const;

    void                _updateCalculationTabWidgets(void);
    void                _updateCalculationSellWeight(void);

    void                _parseAnimalRawData(void);
    void                _parseIngredientsRowData(void);

    static void         logQ(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    static void         _writeLog(const char* str);

private slots:
    void                onMainTabChange(int index);
    void                onAddIngredientClick(bool);
    void                onDeleteIngredientClick(bool);

    void                onAddNutrientClick(bool);
    void                onDeleteNutrientClick(bool);

    void                onAddRecipeClick(bool);
    void                onDeleteRecipeClick(bool);
    void                onRecipeSelectionChanged(const QModelIndex& selected, const QModelIndex& deselected);
    void                onRecipeValueChanged(int row, int col, QVariant oldValue, QVariant newValue, int role);

    void                onAddRationClick(bool);
    void                onDeleteRationClick(bool);

    void                onAddAnimalNutritionReqClick(bool);
    void                onDeleteAnimalNutritionReqClick(bool);

    void                onCalculateButtonClick(bool);
    void                onRecipeComboBoxChange(QString text);
    void                onAnimalComboBoxChange(QString text);
    void                onMatureWeightComboBoxChange(QString text);
    void                onCurrentWeightComboBoxChange(QString text);
    void                onAverageDailyGainComboBoxChange(QString text);

    void                on_actionExport_triggered();
    void                on_actionImport_triggered();
    void                on_actionUse_Default_triggered();
    void                on_actionAbout_triggered();

    void                on_addAnimalTypeButton_clicked(bool);
    void                on_deleteAnimalTypeButton_clicked(bool);
    void                onAnimalSelectionChanged(const QModelIndex& selected, const QModelIndex& deselected);
    void                onAnimalValueChanged(int row, int col, QVariant oldValue, QVariant newValue, int role);
};

#endif // MAINWINDOW_H
