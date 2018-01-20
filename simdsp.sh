#!/bin/sh
appname="SimDSP"

dirname="$(dirname -- "$(readlink -f -- "${0}")" )"

if [ "$dirname" = "." ]; then
	dirname="$PWD/$dirname/resources/"
fi

dirname="$dirname/resources/"

if [ -n "$LIBRARY_PATH" ]; then
  LIBRARY_PATH=$LIBRARY_PATH:"${dirname}/lib"
else
  LIBRARY_PATH="${dirname}/lib"
fi

LD_LIBRARY_PATH="${dirname}/lib"
export LD_LIBRARY_PATH
export LIBRARY_PATH

CPATH="${dirname}/include"
export CPATH

"$dirname/$appname" $*
