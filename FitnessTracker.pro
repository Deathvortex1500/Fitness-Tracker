QT += core gui widgets sql
CONFIG += c++20
TEMPLATE = app
TARGET = Fitness

SOURCES += \
    src/main.cpp \
    src/datastore.cpp \
    src/chartwidget.cpp \
    src/mainwindow.cpp

HEADERS += \
    src/datastore.h \
    src/chartwidget.h \
    src/mainwindow.h

unix:!macx {
    target.path = /usr/bin
    INSTALLS += target
}
