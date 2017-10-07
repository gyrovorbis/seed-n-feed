#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>

namespace Ui {
class MainWindow;
}

class IngredientsTable;
class RationTable;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    IngredientsTable *ingredientsTable;
    RationTable *rationTable;
private slots :
    void onAddIngredientClick(bool);
    void onDeleteIngredientClick(bool);

};

#endif // MAINWINDOW_H
