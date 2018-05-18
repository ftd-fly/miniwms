#-------------------------------------------------
#
# Project created by QtCreator 2017-12-04T09:44:25
#
#-------------------------------------------------

QT       += core gui serialport websockets sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HrgForkliftDispatcher
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
    sql/sql.cpp \
    global.cpp \
    network/websocketclient.cpp \
    network/websocketserver.cpp \
    configure.cpp \
    ui/widgettypea.cpp \
    ui/widgettypeb.cpp \
    ui/widgettypec.cpp \
    ui/widgetgood.cpp \
    controlcenter.cpp \
    agvconnector.cpp \
    radiofrequency.cpp \
    crc16.cpp \
    ui/centerwidget.cpp \
    ui/qyhclicklabel.cpp \
    serialthread.cpp

HEADERS += \
        mainwindow.h \
    sql/sql.h \
    global.h \
    network/websocketclient.h \
    network/websocketserver.h \
    configure.h \
    ui/widgettypea.h \
    ui/widgettypeb.h \
    ui/widgettypec.h \
    ui/widgetgood.h \
    controlcenter.h \
    agvconnector.h \
    radiofrequency.h \
    crc16.h \
    task.h \
    ui/centerwidget.h \
    ui/qyhclicklabel.h \
    serialthread.h


RESOURCES += \
    hrgforkliftdispatcher.qrc
