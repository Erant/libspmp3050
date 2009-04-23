#!/bin/bash

BASEPATH=`pwd`
INSTALLPATH=`pwd`/install
BUILDPATH=`pwd`/build
CORES=6

# before running this script, make sure you have done "sudo apt-get build-dep gcc"


mkdir -p "$INSTALLPATH"
mkdir -p "$BUILDPATH"
mkdir -p "$BUILDPATH/binutils"
mkdir -p "$BUILDPATH/gcc"

# binutils

echo "Fetching binutils"
wget -c http://ftp.gnu.org/gnu/binutils/binutils-2.19.1.tar.gz  || exit
echo "Unpacking binutils"
tar xzf binutils-2.19.1.tar.gz || exit
echo "Configuring binutils"
cd "$BUILDPATH/binutils" || exit
../../binutils-2.19.1/configure --prefix="$INSTALLPATH" --target=arm-elf --disable-nls --with-cpu=arm926ej-s --disable-werror || exit 
#echo "Patching Makefile"
# ed ",s/WARN_CFLAGS = -W -Wall -Wstrict-prototypes -Wmissing-prototypes/WARN_CFLAGS = -W -Wall -Wstrict-prototypes -Wmissing-prototypes/" gas/Makefile || exit
echo "Building binutils"
make -j${CORES} || exit
echo "Installing binutils"
make install || exit

# gcc
cd "$BASEPATH" || exit
echo "Fetching gcc"
wget -c http://ftp.gnu.org/gnu/gcc/gcc-4.3.3/gcc-4.3.3.tar.gz  || exit
echo "Unpacking gcc"
tar xzf gcc-4.3.3.tar.gz || exit
cd "$BUILDPATH/gcc" || exit
echo "Configuring gcc"
../../gcc-4.3.3/configure --prefix="$INSTALLPATH" --target=arm-elf --with-gnu-as --with-gnu-ld --with-cpu=arm926ej-s --disable-nls --enable-languages=c --disable-libssp
echo "Building gcc"
make -j${CORES} || exit
echo "Installing gcc"
make install || exit
