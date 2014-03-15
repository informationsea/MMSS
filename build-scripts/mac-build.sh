#!/bin/sh

function die {
    exit 1
}

cd "`dirname $0`"

DISTDIR=MMSS

if [ -f Makefile ]; then
    make clean
    rm Makefile
fi

if [ -f mmss.dmg ];then
    rm mmss.dmg
fi

if [ -d MMSS.app ];then
    rm -rf MMSS.app
fi

if [ -d $DISTDIR ];then
    rm -rf $DISTDIR
fi

qmake ../MMSS.pro
make -j5
macdeployqt MMSS.app

mkdir -p $DISTDIR

mv MMSS.app $DISTDIR/
cp ../LICENSE $DISTDIR/
cp ../README.md $DISTDIR/
ln -s /Applications $DISTDIR/Applications

hdiutil create -ov -srcfolder "$DISTDIR" -fs HFS+J -format UDBZ -volname "MMSS" mmss.dmg || die



