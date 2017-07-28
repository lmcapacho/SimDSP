#!/bin/bash
# Based into fritzing release script
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
$QT_HOME/bin/qmake
make

cd $app_folder
echo "Compiling SimDSP app..."
$QT_HOME/bin/qmake
make

release_name=simdsp-$relname.linux.$arch
release_folder="$current_dir/$release_name"
echo "making release folder: $release_folder"
mkdir $release_folder

echo "copying release files"
cp -rf simdsp.sh install_simdsp.sh simdsp.desktop README.md LICENSE $release_folder/
mv build/SimDSP $release_folder/
chmod +x $release_folder/install_simdsp.sh

cd $release_folder

echo "making library folders"
mkdir lib
mkdir lib/imageformats
mkdir lib/sqldrivers
mkdir lib/platforms

cd lib

echo "copying sdcore libraries"
cp -d $app_folder/sdcore/build/libsdcore.so* .

echo "copying qt libraries"
cp -d $QT_HOME/lib/libQt5PrintSupport.so* $QT_HOME/lib/libQt5Widgets.so* $QT_HOME/lib/libQt5Gui.so* $QT_HOME/lib/libQt5Core.so* $QT_HOME/lib/libicui18n.so* $QT_HOME/lib/libicuuc.so.* $QT_HOME/lib/libicudata.so* .

echo "copying qt plugins"
cp $QT_HOME/plugins/imageformats/libqjpeg.so imageformats
cp $QT_HOME/plugins/sqldrivers/libqsqlite.so sqldrivers
cp $QT_HOME/plugins/platforms/libqxcb.so platforms

mv ../SimDSP .
mv ../simdsp.sh ../simdsp
chmod +x ../simdsp

cd $current_dir

echo "compressing...."
tar -cjf ./$release_name.tar.bz2 $release_name

echo "cleaning up"
rm -rf $release_folder

echo "done!"
