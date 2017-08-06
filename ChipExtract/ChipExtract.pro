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
INCLUDEPATH += $$_PRO_FILE_PWD_/../cvinclude/debug/
} else {
TARGET = ChipExtract
LIBS += -L$$_PRO_FILE_PWD_/../lib/release -lICLayer
INCLUDEPATH += $$_PRO_FILE_PWD_/../cvinclude/release/
}

TEMPLATE = app
LIBS += -L$$_PRO_FILE_PWD_/../lib
LIBS += -lRaknet -lMdb
INCLUDEPATH += $$_PRO_FILE_PWD_/../Raknet/Include $$_PRO_FILE_PWD_/../Mdb $$_PRO_FILE_PWD_/../DatServer
#DEPENDPATH += $$_PRO_FILE_PWD_/../Raknet/Source $$_PRO_FILE_PWD_/../Raknet/Include $$_PRO_FILE_PWD_/../Mdb


SOURCES += main.cpp\
    extractwindow.cpp \
    clientthread.cpp \
    renderimage.cpp \
    connectview.cpp \
    paramdialog.cpp \
    searchobject.cpp \
    extractparam.cpp \
    objectdb.cpp

HEADERS  += extractwindow.h \
    clientthread.h \
    renderimage.h \
    connectview.h \
    paramdialog.h \
    searchobject.h \
    extractparam.h \
    objectdb.h

FORMS    += \
    extractwindow.ui \
    paramdialog.ui

DESTDIR = $$_PRO_FILE_PWD_/../app


win32 {
Release:LIBS += -lopencv_core249 -lopencv_highgui249 -lopencv_imgproc249
Debug:LIBS += -lopencv_core249d -lopencv_highgui249d -lopencv_imgproc249d
LIBS += -lWs2_32 -ladvapi32
}

unix {
LIBS += -lopencv_highgui -lopencv_imgproc -lopencv_core -lpthread -lm -ldl -lstdc++
}
