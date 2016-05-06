#!/bin/sh
SYSTEM_ROOT=`pwd`
if [ ! -f $SYSTEM_ROOT/core/include/init.inc ]; then
	echo "This script must be run from the matrix root directory."
	echo "Aborting."
	exit 1
fi

rm -r ${SYSTEM_ROOT}/data/private/Oven/*
rm -r ${SYSTEM_ROOT}/data/private/QueryStore/*
