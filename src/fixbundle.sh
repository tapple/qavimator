#!/bin/sh
# Copy extra files to the Mac OS/X application bundle
## Christine Montgomery, August 2006

cd ../bin

# Bundle the data files in the right place
mkdir -p qavimator.app/Contents/MacOS/data
cp data/* qavimator.app/Contents/MacOS/data

# Bundle libqt-mt with qavimator.app
mkdir -p qavimator.app/Contents/Frameworks
cp $QTDIR/lib/libqt-mt.3.dylib qavimator.app/Contents/Frameworks
install_name_tool \
        -id @executable_path/../Frameworks/libqt-mt.3.dylib \
        qavimator.app/Contents/Frameworks/libqt-mt.3.dylib

install_name_tool \
        -change /opt/local/lib/libqt-mt.3.dylib \
        @executable_path/../Frameworks/libqt-mt.3.dylib \
        qavimator.app/Contents/MacOS/qavimator

