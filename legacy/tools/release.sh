#!/bin/bash
# Based into fritzing release script (https://github.com/fritzing/fritzing-app)
# Using linuxdeployqt (https://github.com/probonopd/linuxdeployqt)

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

while [ "$1" != "" ]; do
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
  QT_HOME=$QT_HOME"/gcc_64"
else
  arch='i386'
  QT_HOME=$QT_HOME"/gcc"	
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

release_name=simdsp-$RELNAME.linux.$arch
release_folder="$current_dir/$release_name"
echo "making release folder: $release_folder"
mkdir $release_folder

echo "copying release files"
cp -rf simdsp.sh install_simdsp.sh simdsp.desktop README.md LICENSE $release_folder/
mkdir $release_folder/icons
cp resources/images/simdsp_icon.png $release_folder/icons/
cp SimDSP $release_folder/
chmod +x $release_folder/install_simdsp.sh

export LD_LIBRARY_PATH=$sdcore_folder/build
cd $release_folder

echo "making include folder"
mkdir include

echo "copying examples"
mkdir examples
cp -rf $app_folder/examples .

echo "copying headers files"
cp -rf $sdcore_folder/simdsp.h $sdcore_folder/sdfunctions.h include/

echo "copying libraries"
wget -c "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt*.AppImage
./linuxdeployqt*.AppImage SimDSP -bundle-non-qt-libs -qmake=$QT_HOME/bin/qmake

echo "copying sdcore libraries"
cp -d $sdcore_folder/build/libsdcore.so* lib/

rm AppRun

mv plugins/* lib/
rm -rf plugins

mkdir resources
mv SimDSP resources
mv lib include icons examples doc translations resources
rm -f qt.conf

mv simdsp.sh simdsp
chmod +x simdsp

rm -f linuxdeployqt*.AppImage

cd $current_dir

echo "compressing...."
tar -cjf ./$release_name.tar.bz2 $release_name

echo "cleaning up"
rm -rf $release_folder

echo "done!"
