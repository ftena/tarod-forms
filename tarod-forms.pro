TEMPLATE = app
INCLUDEPATH += .

HEADERS     = bookdelegate.h initdb.h \
    mainwindow.h \
    addorderwindow.h \
    tools.h
RESOURCES   = \
    tarod_forms.qrc
SOURCES     = bookdelegate.cpp main.cpp \
    mainwindow.cpp \
    addorderwindow.cpp
FORMS       = \
    mainwindow.ui \
    addorderwindow.ui

QT += sql widgets widgets

DISTFILES +=

