#!/bin/bash
# Based into fritzing release script (https://github.com/fritzing/fritzing-app)
# Using linuxdeployqt (https://github.com/probonopd/linuxdeployqt)
set -euo pipefail

arch_aux=`uname -m`

echo ""
echo "NOTE: Execute this script from outside the SimDSP folder"
echo ""


usage ()
{
  echo 'Usage : release.sh -version <version> -qt-path <qt_path>'
  exit
}

if [ "$#" -ne 4 ]
then
  usage
fi

while [ "$#" -gt 0 ]; do
case "$1" in
        "-version" )	shift
						RELNAME=$1
						;;
        "-qt-path" )	shift
                    	QT_HOME=$1
                    	;;
        * )         	echo "$1" " invalid option"
        				exit
    esac
    shift
done

echo
script_path="$(readlink -f "$0")"
script_folder=$(dirname $script_path)
echo "script_folder $script_folder"

current_dir=$(pwd)

if [ "$arch_aux" == 'x86_64' ] ; then
  arch='AMD64'
else
  arch='i386'
fi

if [ ! -x "$QT_HOME/bin/qmake" ]; then
  if [ "$arch_aux" == 'x86_64' ] && [ -x "$QT_HOME/gcc_64/bin/qmake" ]; then
    QT_HOME="$QT_HOME/gcc_64"
  elif [ -x "$QT_HOME/gcc/bin/qmake" ]; then
    QT_HOME="$QT_HOME/gcc"
  fi
fi

app_folder=$(dirname $script_folder)
sdcore_folder="$app_folder/src/sdcore"
build_folder="$app_folder/src/build"
include_folder="$app_folder/src/include"
gui_folder="$app_folder/src/gui"
examples_folder="$app_folder/src/examples"

echo "appfolder $app_folder"
echo "sdcorefolder $sdcore_folder"

cd "$sdcore_folder"
echo "Compiling SimDSP core..."
"$QT_HOME/bin/qmake" -config release
make

cd "$app_folder"
echo "Compiling SimDSP app..."
"$QT_HOME/bin/qmake" -config release
make

release_name=simdsp-$RELNAME.linux.$arch
release_folder="$current_dir/$release_name"
echo "making release folder: $release_folder"
rm -rf "$release_folder"
mkdir -p "$release_folder"

echo "copying release files"
cp -rf scripts/simdsp.sh scripts/install_simdsp.sh scripts/simdsp.desktop README.md LICENSE "$release_folder/"
mkdir -p "$release_folder/icons"
cp "$gui_folder/resources/images/simdsp_icon.png" "$release_folder/icons/"
cp "$build_folder/SimDSP" "$release_folder/"
chmod +x "$release_folder/install_simdsp.sh"

export LD_LIBRARY_PATH="$build_folder"
cd "$release_folder"

echo "making include folder"
mkdir include

echo "copying examples"
mkdir examples
cp -rf "$examples_folder" .

echo "copying headers files"
cp -rf "$include_folder/simdsp.h" "$include_folder/sdfunctions.h" include/

echo "copying libraries"
wget -c "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt*.AppImage
./linuxdeployqt*.AppImage SimDSP -bundle-non-qt-libs -qmake="$QT_HOME/bin/qmake"

echo "copying sdcore libraries"
cp -d "$build_folder"/libsdcore.so* lib/
cp -d "$app_folder"/dependencies/rtaudio/libs/linux-64/librtaudio.so* lib/

rm AppRun

if [ -d plugins ]; then
  for plugin_dir in plugins/*; do
    [ -e "$plugin_dir" ] || continue
    mv "$plugin_dir" lib/
  done
  rm -rf plugins
fi

mkdir resources
mv SimDSP resources
mv lib include icons examples resources
if [ -d doc ]; then mv doc resources; fi
if [ -d translations ]; then mv translations resources; fi
rm -f qt.conf

mv simdsp.sh simdsp
chmod +x simdsp

rm -f linuxdeployqt*.AppImage

cd $current_dir

echo "compressing...."
tar -czf "./$release_name.tar.gz" "$release_name"

echo "cleaning up"
rm -rf "$release_folder"

echo "done!"
