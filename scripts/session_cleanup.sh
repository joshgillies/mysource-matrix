#!/bin/sh
#/**
#* +--------------------------------------------------------------------+
#* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
#* | ACN 084 670 600                                                    |
#* +--------------------------------------------------------------------+
#* | IMPORTANT: Your use of this Software is subject to the terms of    |
#* | the Licence provided in the file licence.txt. If you cannot find   |
#* | this file please contact Squiz (www.squiz.net) so we may provide   |
#* | you a copy.                                                        |
#* +--------------------------------------------------------------------+
#*
#* $Id: session_cleanup.sh,v 1.12 2011/02/22 03:01:25 csmith Exp $
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

# solaris 'which' is broken
# it returns a 0 status regardless
# of whether a file exists or not
# so we have to make up our own
# if the first word is 'no'
# the file isn't there.
file_exists()
{
	os=`uname`
	case "${os}" in
		"SunOS")
			found=`which $1 | cut -d' ' -f1`
			if [ "x$found" = "xno" ]; then
				RET=1
			else
				RET=0
			fi
			return $RET
		;;
		*)
			found=`which $1 2>/dev/null 1>/dev/null`
			return $?
		;;
	esac
}

SYSTEM_TAG=`echo $1 | sed -e 's/\//_/g'`
OS=`uname`
HEAD=`which head`

if [ ! -x $HEAD ]; then
	echo 'head not found. Aborting.'
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

if [ "x${PHP}" != "x" ]; then
	PHP="${PHP}"
else
	PHP=""
	file_exists "php-cli"
	if [ $? -eq 0 ]; then
		PHP=`which php-cli`
	else
		file_exists "php"
		if [ $? -eq 0 ]; then
			PHP=`which php`
		fi
	fi
fi

if [ "x${PHP}" = "x" ]; then
	echo "Cannot find the php binary please be sure to install it or export the path: export PHP=/path/to/bin/php"
	exit 1
fi

SYSTEM_ROOT=$1;

SESSION_MATRIXLIFE=`$GREP -E "SQ_CONF_SESSION_GC_MAXLIFETIME',[ ]?[0-9]+" ${SYSTEM_ROOT}/data/private/conf/main.inc | $HEAD -n 1 | $SED -e 's/[^0-9]//g'`;

# main.inc now stores whether the session save path is the default for php or not.
# if it's set to 1, then it's using the php default
# if it's set to 0, it's using a custom setting.
# it's easier to do it all in php code mainly for the extra session_save_path checks
# (to make sure it's not empty).
#
# we'll just export the bits we need to use.
#
php_code="<?php
require_once '${SYSTEM_ROOT}/core/include/init.inc';
require_once '${SYSTEM_ROOT}/data/private/conf/main.inc';

if (!defined('SQ_CONF_CUSTOM_SESSION_SAVE_PATH')) {
	define('SQ_CONF_CUSTOM_SESSION_SAVE_PATH', false);
}

if (!defined('SQ_CONF_SESSION_HANDLER')) {
	define('SQ_CONF_SESSION_HANDLER', 'default');
}

if (!defined('SQ_CONF_CUSTOM_SESSION_SAVE_PATH')) {
	define('SQ_CONF_CUSTOM_SESSION_SAVE_PATH', '');
}

\$var = 'SESSION_USING_DEFAULT_LOCATION';
echo \$var . '=\"' . (int)SQ_CONF_USE_DEFAULT_SESSION_SAVE_PATH . '\";';
echo 'export ' . \$var . ';';

\$var = 'SESSION_TYPE';

\$handler = strtolower(SQ_CONF_SESSION_HANDLER);
if (\$handler != '' && \$handler != 'default') {
	echo \$var . '=\"' . \$handler . '\";';
} else {
	echo \$var . '=\"file\";';
}
echo 'export ' . \$var . ';';

\$var = 'SESSION_LOCATION';

if (SQ_CONF_USE_DEFAULT_SESSION_SAVE_PATH == false || SQ_CONF_CUSTOM_SESSION_SAVE_PATH === '') {
	\$session_path = '${SYSTEM_ROOT}/cache';
} else {
	\$session_path = SQ_CONF_CUSTOM_SESSION_SAVE_PATH;
}
echo \$var . '=\"' . \$session_path . '\";';
echo 'export ' . \$var . ';';
";

eval `echo "${php_code}" | $PHP`

if [ $SESSION_TYPE != "file" ]; then
	if [ $DEBUG -ne 0 ]; then
		echo "session handler is set to not files. Nothing to do"
	fi;
	exit
fi

SESSION_LIFETIME=`expr $SESSION_MATRIXLIFE / $FIND_TIMEOFFSET`;

if [ $SESSION_LIFETIME -le 0 ]; then
	#this way, we don't just destroy every single session file we find, regardless of how stupid our operating environment is....
	echo "Session lifetime is less than $FIND_TIMEOFFSET seconds, falling back onto $FIND_TIMEOFFSET seconds as the duration of session files";
	SESSION_LIFETIME=1;
	SESSION_MATRIXLIFE=$FIND_TIMEOFFSET;
fi

if [ $SESSION_LIFETIME -gt 0 ]; then

	if [ -w $SESSION_LOCATION ]; then

	if [ $DEBUG -ne 0 ]; then echo Removing all sessions matching mask 'sess_*' older than $SESSION_MATRIXLIFE seconds from $SESSION_LOCATION; fi;
	`$FIND $SESSION_LOCATION -maxdepth 1 -name 'sess_*' $FIND_TIMEARG +$SESSION_LIFETIME -exec rm {} \;`

	else
		echo You don\'t have permission to delete files from $SESSION_LOCATION;
	fi;

fi;

#EOF

