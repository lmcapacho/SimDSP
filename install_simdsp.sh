#!/bin/sh
#
# this is a rough beginning of a linux install script for SimDSP
#
# first ensure SimDSP is unpacked in its final destination
# and then run this script

APPLICATIONSDIR="${HOME}/.local/share/applications"

APPDIR=$(dirname "$0")

cd $APPDIR

# install SimDSP desktop entry for user
desktop-file-edit --set-key=Exec --set-value="$(pwd)/simdsp" simdsp.desktop
xdg-desktop-menu install --novendor --mode user simdsp.desktop

# install image-files into user hicolor theme with specified size
# ~/.local/share/icons/hicolor/*size*
#    /apps
xdg-icon-resource install --noupdate --novendor --mode user --context apps \
	--size 256 icons/simdsp_icon.png simdsp

# update user databases
update-desktop-database "${APPLICATIONSDIR}"
xdg-icon-resource forceupdate --mode user

echo "installed SimDSP system icons"
