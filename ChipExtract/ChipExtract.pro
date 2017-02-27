#-------------------------------------------------
#
# Project created by QtCreator 2016-02-11T15:48:19
#
#-------------------------------------------------

QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG(debug, debug|release) {
TARGET = ChipExtractd
LIBS += -L$$_PRO_FILE_PWD_/../lib/debug -lICLayerd
} else {
TARGET = ChipExtract
LIBS += -L$$_PRO_FILE_PWD_/../lib/release -lICLayer
}

TEMPLATE = app
LIBS += -L$$_PRO_FILE_PWD_/../lib
LIBS += -lRaknet -lMdb
INCLUDEPATH += $$_PRO_FILE_PWD_/../Raknet/Include $$_PRO_FILE_PWD_/../Mdb $$_PRO_FILE_PWD_/../DatServer
DEPENDPATH += $$_PRO_FILE_PWD_/../Raknet/Source $$_PRO_FILE_PWD_/../Raknet/Include $$_PRO_FILE_PWD_/../Mdb
win32:LIBS += -lWs2_32 -ladvapi32


SOURCES += main.cpp\
    extractwindow.cpp \
    clientthread.cpp \
    renderimage.cpp \
    connectview.cpp \
    paramdialog.cpp \
    searchobject.cpp \
    objectdb.cpp

HEADERS  += extractwindow.h \
    clientthread.h \
    renderimage.h \
    connectview.h \
    paramdialog.h \
    searchobject.h \
    objectdb.h

FORMS    += \
    extractwindow.ui \
    paramdialog.ui

DESTDIR = $$_PRO_FILE_PWD_/../app
