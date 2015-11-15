#!/bin/sh

PSXDEV=

if [ $PATH ] ; then
  PATH=/bin:$PATH
else
  PATH=/bin
fi

if [ $LD_LIBRARY_PATH ] ; then
  LD_LIBRARY_PATH=/lib:$LD_LIBRARY_PATH
else
  LD_LIBRARY_PATH=/lib
fi

alias psx-file='file -m $PSXDEV/share/magic'

export PSXDEV PATH LD_LIBRARY_PATH
