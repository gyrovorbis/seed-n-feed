#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QDir>

#define DATABASE_NAME   "seed_and_feed.db"
#define DATABASE_TYPE   "QSQLITE"
#define DEBUG_LOG_NAME  "debug_log.txt"
#define USER_DIR_NAME   "seednfeed"

namespace Ui {
    class MainWindow;
}

class IngredientsTable;
class RationTable;

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

private:
    Ui::MainWindow*     ui;
    QSqlDatabase        db;
    IngredientsTable*   ingredientsTable;
    RationTable*        rationTable;
    QDir                userDir;
    static FILE*        dbgLogFile;

    virtual void        closeEvent(QCloseEvent *event) override;

    bool                _debugInit(void);
    bool                _dbInit(void);

    static void         logQ(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    static void         dbgPrintf(const char* str, ...);
    static void         _writeLog(const char* str);

private slots:
    void                onAddIngredientClick(bool);
    void                onDeleteIngredientClick(bool);
    void                onAddRationClick(bool);
    void                onDeleteRationClick(bool);
};

#endif // MAINWINDOW_H
