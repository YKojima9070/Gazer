greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

#Switch OpenCV or QCamera
#DEFINES += GAZER_USE_QT_CAMERA=1
#QT += multimediawidgets

TEMPLATE = app
TARGET = Gazer2_customized

QT += core gui multimedia

SOURCES += capture_thread.cpp \
           main.cpp \
           mainwindow.cpp \
           utilities.cpp

HEADERS += capture_thread.h \
           mainwindow.h \
           utilities.h

FORMS +=

////////////
//超重要！！//
////////////
//カメラアクセス用のinfo.plistアクセス用
QMAKE_INFO_PLIST = MyAppInfo.plist


unix:!mac{
    INCLUDEPATH += /path/to/opencv/include/opencv4
    LIBS += -L/path/to/opencv/lib -lopencv_world
}

unix:mac{
    INCLUDEPATH += /Users/yuki/source/opencv-4.4.0/include
    LIBS += -L/Users/yuki/source/opencv/lib \
    -lopencv_world
    ##-lopencv_core \
    ##-lopencv_imgproc \
    ##-lopencv_imgcodecs \
    ##-lopencv_video \
    ##-lopencv_videoio
}

unix:win32{
    INCLUDEPATH += /path/to/opencv/include/opencv4
    LIBS += -lc:/path/to/opencv/lib/opencv_world
}
