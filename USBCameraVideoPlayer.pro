#-------------------------------------------------
#
# Project created by QtCreator 2022-05-15T16:12:30
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = USBCameraVideoPlayer
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
        VideoCapture/videocapture.cpp \
    imagewidget.cpp \
    VideoWriter/videowriter.cpp \
    VideoRecordControlWidget/videorecordcontrolwidget.cpp \
    VideoCaptureControlWidget/videocapturecontrolwidget.cpp

HEADERS += \
        mainwindow.h \
        VideoCapture/videocapture.h \
    imagewidget.h \
    VideoWriter/videowriter.h \
    Utility/utility.h \
    VideoRecordControlWidget/videorecordcontrolwidget.h \
    VideoCaptureControlWidget/videocapturecontrolwidget.h

FORMS += \
        mainwindow.ui \
    VideoRecordControlWidget/videorecordcontrolwidget.ui \
    VideoCaptureControlWidget/videocapturecontrolwidget.ui


unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += /usr/lib/aarch64-linux-gnu/pkgconfig/gstreamer-1.0.pc
unix: PKGCONFIG += /usr/lib/aarch64-linux-gnu/pkgconfig/gstreamer-app-1.0.pc
