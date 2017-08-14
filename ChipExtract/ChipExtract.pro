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
LIBS += -L$$_PRO_FILE_PWD_/../lib
LIBS += -lopencv_contrib -lopencv_stitching -lopencv_nonfree -lopencv_superres -lopencv_ocl -lopencv_ts
LIBS += -lopencv_videostab -lopencv_gpu -lopencv_photo -lopencv_objdetect -lopencv_legacy -lopencv_video
LIBS += -lopencv_ml -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lIlmImf -llibjasper
LIBS += -llibtiff -llibpng -llibjpeg -lopencv_imgproc -lopencv_flann -lopencv_core -lzlib
LIBS += -lswscale-ffmpeg -lavutil-ffmpeg -lavformat-ffmpeg -lavcodec-ffmpeg -ldc1394 -lgstvideo-0.10
LIBS += -lgstapp-0.10 -lxml2 -lgmodule-2.0 -lgstreamer-0.10 -lgstbase-0.10 -lgthread-2.0 -lfreetype -lfontconfig
LIBS += -lglib-2.0 -lgobject-2.0 -lpango-1.0 -lpangoft2-1.0 -lgio-2.0 -lgdk_pixbuf-2.0 -lcairo -latk-1.0 -lpangocairo-1.0 -lgdk-x11-2.0 -lgtk-x11-2.0 -lrt -lpthread -lm -ldl -lstdc++
}
