#!/bin/sh

PROJ=dawg
PROJ_DISTS=dawg-1*
MAKE=make
CMAKE=cmake
REPOS=`svn info | grep URL: | perl -pe 's!^URL: (.+)/releng$!$1!'`

echo 
echo Building distributions for $REPOS ...

xcode=/Xcode2.5
xcodep=${xcode}/usr
PATH="${xcodep}/bin:${PATH}"; export PATH
CFLAGS="$CFLAGS -I${xcodep}/include"; export CFLAGS
CXXFLAGS="$CXXFLAGS -I${xcodep}/include"; export CXXFLAGS
LDFLAGS="-L${xcodep}/lib"; export LDFLAGS
CMAKE_OSX_SYSROOT="${xcode}/SDKs/MacOSX10.4u.sdk"; export CMAKE_OSX_SYSROOT

RELENG_DIR=`mktemp -q -d -t ${PROJ}-releng`
if [ $? -ne 0 ]; then
        echo "$0: Can't create temp directory, exiting..."
        exit 1
fi

echo Using temp directory $RELENG_DIR ...
echo

DEST_DIR=`pwd`
SOURCE_DIR="${RELENG_DIR}/source"
BUILD_DIR="${RELENG_DIR}/build"

svn co -q $REPOS $SOURCE_DIR || exit 1

mkdir $BUILD_DIR || exit 1
cd $BUILD_DIR || exit 1

$CMAKE $SOURCE_DIR -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES="ppc;i386" \
  -DCPACK_SYSTEM_NAME=Darwin8-universal \
  -DAPPLE_BUNDLE=ON
$MAKE
$MAKE package
$MAKE package_source

echo
echo Moving distribution packages ...

mv -v $PROJ_DISTS $DEST_DIR

echo
echo Cleaning up ...

cd $DEST_DIR
rm -rf $RELENG_DIR
