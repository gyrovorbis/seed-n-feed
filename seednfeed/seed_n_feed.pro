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


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    animalprojection.cpp \
    ingredients_table.cpp \
    ration_table.cpp \
    ingredients_table_delegate.cpp \
    animal_nutrition_req_table.cpp

HEADERS += \
        mainwindow.h \
    animalprojection.h \
    ingredients_table.h \
    ration_table.h \
    ingredients_table_delegate.h \
    utilities.h \
    animal_nutrition_req_table.h

FORMS += \
        mainwindow.ui
