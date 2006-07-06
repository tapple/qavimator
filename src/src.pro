# Diese Datei wurde mit dem qmake-Manager von KDevelop erstellt. 
# ------------------------------------------- 
# Unterordner relativ zum Projektordner: ./src
# Das Target ist eine Anwendung:  ../bin/qavimator

FORMS += mainapplicationform.ui 
IDLS += mainapplicationform.ui 
HEADERS += qavimator.h \
           animationview.h \
           animation.h \
           camera.h \
           math3d.h \
           bvh.h \
           iktree.h \
           slparts.h \
           rotation.h \
           prop.h \
           timeline.h \
           timelineview.h 
SOURCES += qavimator.cpp \
           main.cpp \
           animationview.cpp \
           animation.cpp \
           camera.cpp \
           math3d.cpp \
           iktree.cpp \
           bvh.cpp \
           slpartsfemale.cpp \
           slpartsmale.cpp \
           rotation.cpp \
           prop.cpp \
           timeline.cpp \
           timelineview.cpp 
mainapplicationform.ui.target = mainapplicationform.ui
mainapplicationform.ui.commands = $$IDL_COMPILER
QT = qt3support
TARGETDEPS += ../libquat/liblibquat.a
LIBS += ../libquat/liblibquat.a \
-lglut \
-lGLU
INCLUDEPATH += ../libquat \
/usr/include
TARGET = ../bin/qavimator
CONFIG += debug \
warn_on \
qt \
opengl \
thread
TEMPLATE = app
