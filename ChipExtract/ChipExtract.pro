#-------------------------------------------------
#
# Project created by QtCreator 2016-02-11T15:48:19
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

Release:TARGET = ChipExtract
Debug:TARGET = ChipExtractd
TEMPLATE = app
LIBS += -L$$_PRO_FILE_PWD_/../lib
LIBS += -lRaknet -lMdb
INCLUDEPATH += ../Raknet/Include ../Mdb ../DatServer/
DEPENDPATH += ../Raknet/Source ../Raknet/Include ../Mdb
win32:LIBS += -lWs2_32 -ladvapi32

Release:LIBS += -L$$_PRO_FILE_PWD_/../lib/release
Debug:LIBS += -L$$_PRO_FILE_PWD_/../lib/debug

SOURCES += main.cpp\
    extractwindow.cpp \
    clientthread.cpp \
    renderimage.cpp \
    connectview.cpp \
    globalconst.cpp \
    paramdialog.cpp \
    searchobject.cpp \
    objectdb.cpp

HEADERS  += extractwindow.h \
    clientthread.h \
    renderimage.h \
    connectview.h \
    globalconst.h \
    paramdialog.h \
    searchobject.h \
    objectdb.h

FORMS    += \
    extractwindow.ui \
    paramdialog.ui

DESTDIR = $$_PRO_FILE_PWD_/../app
