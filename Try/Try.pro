QT += core gui

TARGET = Try
CONFIG += console
CONFIG -= app_bundle
LIBS += -L$$_PRO_FILE_PWD_/../lib
LIBS += -lRaknet -lMdb
TEMPLATE = app
INCLUDEPATH += ../Raknet/Include ../Mdb ../DatServer/
DEPENDPATH += ../Raknet/Source ../Raknet/Include ../Mdb
win32:LIBS += -lWs2_32 -ladvapi32
unix:LIBS += -lpthread
SOURCES += \
    test_raknet.cpp \
    main.cpp \
    test_lmdb.cpp \
    iclayer.cpp \
    test_element.cpp
DESTDIR = $$_PRO_FILE_PWD_/../app

HEADERS += \
    iclayer.h \
    iclayer_global.h

