#-------------------------------------------------
#
# Project created by QtCreator 2019-08-12T18:32:40
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = untitled32
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

CONFIG += c++17
CONFIG += console

SOURCES += \
        $$PWD/main.cpp \
        $$PWD/MainWindow.cpp

SOURCES += $$PWD/ImageTool.cpp
HEADERS += $$PWD/ImageTool.hpp

HEADERS += \
        $$PWD/MainWindow.hpp

FORMS += \
        MainWindow.ui

INCLUDEPATH += C:/Data/2/opencv/build/include

CONFIG(debug,debug|release){#debug ...
    LIBS += -LC:/Data/2/opencv/build/x64/vc15/bin -lopencv_world411d
}else{#release ...
    LIBS += -LC:/Data/2/opencv/build/x64/vc15/bin -lopencv_world411
    DEFINES *= NDEBUG
}




