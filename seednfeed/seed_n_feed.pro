#-------------------------------------------------
#
# Project created by QtCreator 2017-10-01T16:47:42
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = seed_n_feed
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

RC_ICONS = resource/feedIco.ico

INCLUDEPATH += include

DESTDIR = ../executable

DEPENDPATH += \
        include \
        include/model \
        include/ui \
        include/delegate \
        include/core

SOURCES += \
        source/core/main.cpp \
        source/model/ingredients_table.cpp \
        source/model/ration_table.cpp \
        source/delegate/ingredients_table_delegate.cpp \
        source/delegate/rations_table_delegate.cpp \
        source/model/animal_nutrition_req_table.cpp \
        source/delegate/duplicate_column_model_validator.cpp \
        source/delegate/animal_nutrition_table_delegate.cpp \
        source/delegate/dynamic_model_column_combobox.cpp \
        source/delegate/totals_table_delegate.cpp \
        source/ui/error_warning_dialog.cpp \
    source/ui/mainwindow.cpp \
    source/model/sql_table.cpp \
    source/ui/sql_table_view.cpp \
    source/core/tk_menu_stack.cpp \
    source/model/recipe_table.cpp \
    source/model/nutrient_table.cpp \
    source/delegate/nutrients_table_delegate.cpp \
    source/delegate/recipe_table_delegate.cpp \
    source/model/animal_table.cpp \
    source/ui/sql_table_manager_widget.cpp

HEADERS += \
        include/model/ingredients_table.h \
        include/model/ration_table.h \
        include/delegate/ingredients_table_delegate.h \
        include/core/utilities.h \
        include/delegate/rations_table_delegate.h \
        include/model/animal_nutrition_req_table.h \
        include/delegate/duplicate_column_model_validator.h \
        include/delegate/animal_nutrition_table_delegate.h \
        include/delegate/dynamic_model_column_combobox.h \
        include/delegate/totals_table_delegate.h \
        include/ui/error_warning_dialog.h \
    include/ui/mainwindow.h \
    include/model/sql_table.h \
    include/ui/sql_table_view.h \
    include/core/tk_menu_stack.h \
    include/model/recipe_table.h \
    include/model/nutrient_table.h \
    include/delegate/nutrients_table_delegate.h \
    include/delegate/recipe_table_delegate.h \
    include/model/animal_table.h \
    include/ui/sql_table_manager_widget.h

FORMS += \
        form/error_warning_dialog.ui \
    form/mainwindow.ui \
    form/sql_table_manager_widget.ui

RESOURCES += \
    resource/resource.qrc

DISTFILES +=
