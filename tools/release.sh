#!/bin/bash
# Based into fritzing release script (https://github.com/fritzing/fritzing-app)
# Using linuxdeployqt (https://github.com/probonopd/linuxdeployqt)

arch_aux=`uname -m`

echo
script_path="$(readlink -f "$0")"
script_folder=$(dirname $script_path)
echo "script_folder $script_folder"

current_dir=$(pwd)

echo ""
echo "NOTE: Don't forget to set this script's QT_HOME variable"
echo "      Execute this script from outside the SimDSP folder"
echo ""

if [ "$arch_aux" == 'x86_64' ] ; then
  arch='AMD64'
  QT_HOME="/home/lmcapacho/Qt/5.7/gcc_64"
else
  arch='i386'
  QT_HOME="/home/lmcapacho/Qt/5.5/gcc"	
fi

if [ "$1" = "" ] ; then
  echo "Usage: $0 <need a version string such as '1.0.0b' (without the quotes)>"
  exit
else
  relname=$1
fi

app_folder=$(dirname $script_folder)
sdcore_folder="$app_folder/sdcore"

echo "appfolder $app_folder"
echo "sdcorefolder $sdcore_folder"

cd $sdcore_folder
echo "Compiling SimDSP core..."
$QT_HOME/bin/qmake -config release
make

cd $app_folder
echo "Compiling SimDSP app..."
$QT_HOME/bin/qmake -config release
make

release_name=simdsp-$relname.linux.$arch
release_folder="$current_dir/$release_name"
echo "making release folder: $release_folder"
mkdir $release_folder

echo "copying release files"
cp -rf simdsp.sh install_simdsp.sh simdsp.desktop README.md LICENSE $release_folder/
mkdir $release_folder/icons
cp resources/images/simdsp_icon.png $release_folder/icons/
cp build/SimDSP $release_folder/
chmod +x $release_folder/install_simdsp.sh

export LD_LIBRARY_PATH=$sdcore_folder/build
cd $release_folder

echo "making include folder"
mkdir include

echo "copying headers files"
cp -rf $sdcore_folder/simdsp.h $sdcore_folder/sdfunctions.h include/

echo "copying libraries"
wget -c "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt*.AppImage
./linuxdeployqt*.AppImage SimDSP -bundle-non-qt-libs
#linuxdeployqt SimDSP -bundle-non-qt-libs

echo "copying sdcore libraries"
cp -d $sdcore_folder/build/libsdcore.so* lib/

rm AppRun

mv plugins/* lib/
rm -rf plugins

mv SimDSP lib/
mv simdsp.sh simdsp
chmod +x simdsp

rm -f linuxdeployqt*.AppImage

cd $current_dir

echo "compressing...."
tar -cjf ./$release_name.tar.bz2 $release_name

echo "cleaning up"
rm -rf $release_folder

echo "done!"
