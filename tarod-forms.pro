TEMPLATE = app
INCLUDEPATH += .

HEADERS     = bookdelegate.h initdb.h \
    mainwindow.h
RESOURCES   = \
    tarod_forms.qrc
SOURCES     = bookdelegate.cpp main.cpp \
    mainwindow.cpp
FORMS       = \
    mainwindow.ui

QT += sql widgets widgets

DISTFILES +=

