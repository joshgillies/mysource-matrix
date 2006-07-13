#!/bin/sh
#/**
#* +--------------------------------------------------------------------+
#* | Squiz.net Open Source Licence                                      |
#* +--------------------------------------------------------------------+
#* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
#* +--------------------------------------------------------------------+
#* | This source file may be used subject to, and only in accordance    |
#* | with, the Squiz Open Source Licence Agreement found at             |
#* | http://www.squiz.net/licence.                                      |
#* | Make sure you have read and accept the terms of that licence,      |
#* | including its limitations of liability and disclaimers, before     |
#* | using this software in any way. Your use of this software is       |
#* | deemed to constitute agreement to be bound by that licence. If you |
#* | modify, adapt or enhance this software, you agree to assign your   |
#* | intellectual property rights in the modification, adaptation and   |
#* | enhancement to Squiz Pty Ltd for use and distribution under that   |
#* | licence.                                                           |
#* +--------------------------------------------------------------------+
#*
#* $Id: session_cleanup.sh,v 1.1 2006/07/13 00:55:46 tbarrett Exp $
#*
#*/

#/**
#* Cleans out session files that haven't been modified in the time allocated
#* to garbage collection.
#*
#* Works on GNU Linux, Solaris 9, Solaris 10. (SunOS support depends on xpg4
#* POSIX compliant binaries)
#*
#* Allows for modifications of the cache directory from within Matrix
#*
#*/

if [ ! -n "$1" ] || [ ! -d "$1" ]; then
	echo You must pass the system root of a MySource Matrix installation as the first argument to this script.
	exit 1;
fi;

DEBUG=0;
if [ -n "$2" ]; then
	DEBUG=1;
fi;

if [ ! -r $1/data/private/conf/main.inc ] || [ ! -r $1/core/include/mysource.inc ]; then
	echo "The directory '$1' doesn't seem to contain a valid MySource Matrix installation.";
	exit 1;
fi;

SYSTEM_TAG=`echo $1 | sed -e 's/\//_/g'`
TMPFILE="/tmp/${SYSTEM_TAG}-sessionclean.filelist"
OS=`uname`
HEAD=`which head`
XARGS=`which xargs`

if [ ! -x $HEAD ]; then
	echo 'head not found. Aborting.'
	exit 1;
fi;

if [ ! -x $XARGS ]; then
	echo 'xargs not found. Aborting.'
	exit 1;
fi;

if [ "$OS" = "SunOS" ]; then

	# xpg4 = posix compliant system utils
	# find doesn't have mmin

	if [ ! -d /usr/xpg4/bin ]; then
		echo "POSIX compatable binaries not found in /usr/xpg4, you will need to add them to use this script."
		exit 1;
	fi;

	GREP=/usr/xpg4/bin/grep
	SED=/usr/xpg4/bin/sed
	FIND=/usr/xpg4/bin/find
	FIND_TIMEARG="-mtime"
	FIND_TIMEOFFSET="86400"
else
	GREP=`which grep`
	SED=`which sed`
	FIND=`which find`
	FIND_TIMEARG="-mmin"
	FIND_TIMEOFFSET="60"
fi;

if [ ! -x $GREP ]; then
	echo 'grep not found. Aborting.'
	exit 1;
fi;

if [ ! -x $SED ]; then
	echo 'sed not found. Aborting.'
	exit 1;
fi;

if [ ! -x $FIND ]; then
	echo 'find not found. Aborting.'
	exit 1;
fi;

SYSTEM_ROOT=$1;

SESSION_MATRIXLIFE=`$GREP -E "SQ_CONF_SESSION_GC_MAXLIFETIME',[ ]?[0-9]+" ${SYSTEM_ROOT}/data/private/conf/main.inc | $HEAD -n 1 | $SED -e 's/[^0-9]//g'`;
SESSION_LOCATION=`$GREP -E "session_save_path\(.*\);" ${SYSTEM_ROOT}/core/include/mysource.inc | $HEAD -n 1 | $SED -e 's/session_save_path(//' -e 's/[\W]//g' -e "s,SQ_SYSTEM_ROOT,$SYSTEM_ROOT," -e "s,');\$,," -e "s,.',,"`;

SESSION_LIFETIME=`expr $SESSION_MATRIXLIFE / $FIND_TIMEOFFSET`;
if [ $SESSION_LIFETIME -le 0 ]; then
	#this way, we don't just destroy every single session file we find, regardless of how stupid our operating environment is....
	echo "Session lifetime is less than $FIND_TIMEOFFSET seconds, falling back onto $FIND_TIMEOFFSET seconds as the duration of session files";
	SESSION_LIFETIME=1;
	SESSION_MATRIXLIFE=$FIND_TIMEOFFSET;
fi

if [ $SESSION_LIFETIME -gt 0 ]; then

	if [ -w $SESSION_LOCATION ]; then
	
		if [ -f $TMPFILE ]; then
			echo "The file $TMPFILE exists. Check that this script isn't already running and remove the files if it's orphaned. Aborting."
			exit 1;
		fi;

		`$FIND $SESSION_LOCATION -name 'sess_????????????????????????????????' $FIND_TIMEARG +$SESSION_LIFETIME > $TMPFILE`
		if [ -s $TMPFILE ]; then
			if [ $DEBUG -ne 0 ]; then echo Removing all sessions matching mask 'sess_????????????????????????????????' older than $SESSION_MATRIXLIFE seconds from $SESSION_LOCATION; fi;
			`cat $TMPFILE | $XARGS rm`
		else
			if [ $DEBUG -ne 0 ]; then echo Couldn\'t find any session files worth worrying about; fi;
		fi;
		`rm $TMPFILE`

	else
		echo You don\'t have permission to delete files from $SESSION_LOCATION;
	fi;

fi;

#EOF

