#-------------------------------------------------
#
# Project created by QtCreator 2016-02-10T09:53:27
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

LIBS += -L$$_PRO_FILE_PWD_/../lib

CONFIG(debug, debug|release) {
TARGET = DatServerd
LIBS += -L$$_PRO_FILE_PWD_/../lib/debug -lVWExtractd
INCLUDEPATH += $$_PRO_FILE_PWD_/../cvinclude/debug/
} else {
TARGET = DatServer
LIBS += -L$$_PRO_FILE_PWD_/../lib/release -lVWExtract
INCLUDEPATH += $$_PRO_FILE_PWD_/../cvinclude/release/
}

TEMPLATE = app
LIBS += -lRaknet -lMdb
INCLUDEPATH += $$_PRO_FILE_PWD_/../Raknet/Include $$_PRO_FILE_PWD_/../Mdb
DEPENDPATH += $$_PRO_FILE_PWD_/../Raknet/Source $$_PRO_FILE_PWD_/../Raknet/Include $$_PRO_FILE_PWD_/../Mdb
win32:LIBS += -lWs2_32 -ladvapi32

SOURCES += main.cpp\
        serverwindow.cpp \
    serverthread.cpp \
    iclayer.cpp \
    serverperclient.cpp \
    cellextract.cpp

HEADERS  += serverwindow.h \
    serverthread.h \
    communication.hpp \
    iclayer.h \
    serverperclient.h \
    cellextract.h \
    objextract.h \
    markobj.h \
    vwextract.h

FORMS    += serverwindow.ui
DESTDIR = $$_PRO_FILE_PWD_/../app

win32 {
Release:LIBS += -lopencv_calib3d249 -lopencv_contrib249 -lopencv_core249 -lopencv_features2d249 -lopencv_flann249
Release:LIBS += -lopencv_gpu249 -lopencv_highgui249 -lopencv_imgproc249 -lopencv_legacy249 -lopencv_ml249
Release:LIBS += -lopencv_nonfree249 -lopencv_objdetect249 -lopencv_photo249 -lopencv_stitching249 -lopencv_superres249
Release:LIBS += -lopencv_ts249 -lopencv_video249 -lopencv_videostab249

Debug:LIBS += -lopencv_calib3d249d -lopencv_contrib249d -lopencv_core249d -lopencv_features2d249d -lopencv_flann249d
Debug:LIBS += -lopencv_gpu249d -lopencv_highgui249d -lopencv_imgproc249d -lopencv_legacy249d -lopencv_ml249d
Debug:LIBS += -lopencv_nonfree249d -lopencv_objdetect249d -lopencv_photo249d -lopencv_stitching249d -lopencv_superres249d
Debug:LIBS += -lopencv_ts249d -lopencv_video249d -lopencv_videostab249d
}

unix {
LIBS += -lopencv_contrib -lopencv_stitching -lopencv_nonfree -lopencv_superres -lopencv_ocl -lopencv_ts
LIBS += -lopencv_videostab -lopencv_gpu -lopencv_photo -lopencv_objdetect -lopencv_legacy -lopencv_video
LIBS += -lopencv_ml -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lIlmImf -llibjasper
LIBS += -llibtiff -llibpng -llibjpeg -lopencv_imgproc -lopencv_flann -lopencv_core -lzlib
LIBS += -lswscale-ffmpeg -lavutil-ffmpeg -lavformat-ffmpeg -lavcodec-ffmpeg -ldc1394 -lgstvideo-0.10
LIBS += -lgstapp-0.10 -lxml2 -lgmodule-2.0 -lgstreamer-0.10 -lgstbase-0.10 -lgthread-2.0 -lfreetype -lfontconfig
LIBS += -lglib-2.0 -lgobject-2.0 -lpango-1.0 -lpangoft2-1.0 -lgio-2.0 -lgdk_pixbuf-2.0 -lcairo -latk-1.0 -lpangocairo-1.0 -lgdk-x11-2.0 -lgtk-x11-2.0 -lrt -lpthread -lm -ldl -lstdc++
}
