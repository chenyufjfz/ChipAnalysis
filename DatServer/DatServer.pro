#-------------------------------------------------
#
# Project created by QtCreator 2016-02-10T09:53:27
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DatServer
TEMPLATE = app
LIBS += -L$$_PRO_FILE_PWD_/../lib
LIBS += -lRaknet -lMdb
INCLUDEPATH += ../Raknet/Include ../Mdb
DEPENDPATH += ../Raknet/Source ../Raknet/Include ../Mdb
win32:LIBS += -lWs2_32 -ladvapi32

SOURCES += main.cpp\
        serverwindow.cpp \
    serverthread.cpp \
    iclayer.cpp

HEADERS  += serverwindow.h \
    serverthread.h \
    communication.hpp \
    iclayer.h

FORMS    += serverwindow.ui
DESTDIR = $$_PRO_FILE_PWD_/../app
