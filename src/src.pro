# Diese Datei wurde mit dem qmake-Manager von KDevelop erstellt. 
# ------------------------------------------- 
# Unterordner relativ zum Projektordner: ./src
# Das Target ist eine Anwendung:  ../bin/qavimator

FORMS += mainapplicationform.ui \
         settingsdialogform.ui 
IDLS += mainapplicationform.ui \
        settingsdialogform.ui 
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
           timelineview.h \
           keyframelist.h \
           bvhnode.h \
           settings.h \
           settingsdialog.h  \
 icons.h
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
           timelineview.cpp \
           keyframelist.cpp \
           bvhnode.cpp \
           settings.cpp \
           settingsdialog.cpp 
mainapplicationform.ui.target = mainapplicationform.ui
mainapplicationform.ui.commands = $$IDL_COMPILER
QT = qt3support
TARGETDEPS += ../libquat/libquat.a
LIBS += ../libquat/libquat.a \
-lglut \
-lGLU
INCLUDEPATH += ../libquat \
/usr/include
QMAKE_CXXFLAGS_DEBUG += -g3
TARGET = ../bin/qavimator
CONFIG += debug \
warn_on \
qt \
opengl \
thread
TEMPLATE = app
macx {
    DEFINES += MACOSX
    LIBS += -framework OpenGL -framework AGL -framework GLUT -bind_at_load
    LIBS -= -lGLU -lglut
    INCLUDEPATH += /Developer/SDKs/MacOSX10.4u.sdk/System/Library/Frameworks/GLUT.framework/Versions/A/Headers
    QMAKE_POST_LINK += sh fixbundle.sh
}
