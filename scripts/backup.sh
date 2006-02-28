#!/bin/bash
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
#* $Id: backup.sh,v 1.4.2.1 2006/02/28 05:24:28 dmckee Exp $
#*
#*/
#
#	When using with argument --remotedb=user@host it connect by ssh and
#	dump the database there.
#

# Creates a backup

SYSTEM_ROOT=".";

if [ ! -f ${SYSTEM_ROOT}/data/private/conf/main.inc ]; then
	echo "This isn't being run from the system root folder. Aborting."
	exit 1
fi

if [[ $1 == --remotedb=* ]];then
	remote=${1##*=}
	shift
fi

if [ ! -z $1 ]; then
	tempvar=$1
	# If the last 3 chars are "tar" then we're specifying a filename.
	# Otherwise, it's a directory.
	lastchars=`echo ${tempvar} | awk -F'/' '{ print $(NF) }' | awk -F'.' '{ print $(NF) }'`
	# If it doesn't exist or is a space, the next check for tar gives an error
	# [ too many arguments
	# So set it temporarily. We only need to check it here, so setting it like this doesn't matter.
	if [ -z ${lastchars} ]; then
		lastchars="tmp"
	fi
	if [ ${lastchars} = "tar" ]; then
		# It's a specific file.
		temp=${tempvar}
		backupdir=`dirname $temp`
		if [ ! -d $backupdir ]; then
			mkdir -p $backupdir
			if [ $? -gt 0 ]; then
				echo "Unable to create directory ${backupdir}. Your problem."
				exit 2
			fi
		fi
		backupfilename=`basename $temp`
	else
		backupdir=${tempvar}
		if [ ! -d ${backupdir} ]; then
			mkdir -p ${backupdir}
			if [ $? -gt 0 ]; then
				echo "Unable to create directory ${backupdir}. Your problem."
				echo "Aborting."
				exit 3
			fi
		fi
	fi
fi

# OK, what we are doing here is using PHP to do the parsing of the DSN for us (much less error prone :)
# see the output of DB::parseDSN
php_code="<?php
require_once '${SYSTEM_ROOT}/data/private/conf/main.inc';
require_once 'DB.php';
\$dsn = DB::parseDSN(SQ_CONF_DB_DSN);
foreach(\$dsn as \$k => \$v) {
	echo 'DB_'.strtoupper(\$k).'=\"'.addslashes(\$v).'\";';
}
?>"
eval `echo "${php_code}" | php`

set | grep "^DB_"

dumpfile=${SYSTEM_ROOT}/matrix-`date +%Y-%m-%d_%H-%M`.dump
[[ -n $remote ]] && remotefile="/tmp/matrix-`date +%Y-%m-%d_%H-%M`.dump"

case "${DB_PHPTYPE}" in
	"pgsql")
		args="";
		if [ "${DB_USERNAME}" != "" ]; then
			args="${args} -U ${DB_USERNAME}";
		fi
		if [ "${DB_PASSWORD}" != "" ]; then
			echo "I can't pass the password automatically because the psql command only supports prompting.";
			args="${args} -W";
		fi
		if [ "${DB_HOSTSPEC}" != "" ]; then
			args="${args} -h ${DB_HOSTSPEC}";
		fi
		if [ "${DB_PORT}" != "" ]; then
			args="${args} -p ${DB_PORT}";
		fi
		if [[ -z $remote ]];then
			pg_dump ${args} "${DB_DATABASE}"  > ${dumpfile}
		else
			ssh ${remote} "pg_dump ${args} > ${remotefile}"
			scp ${remote}:${remotefile} ${dumpfile}
			[[ $? == 0 ]] && ssh ${remote} "rm -f ${remotefile}"
		fi
		if [ $? -gt 0 ]; then
			echo "Unable to create dumpfile ${dumpfile}."
			echo "Aborting."
			exit 5
		fi
	;;
	"oci8")
		args="";
		if [ "${DB_USERNAME}" != "" ]; then
			args="${args} ${DB_USERNAME}";
		fi
		if [ "${DB_PASSWORD}" != "" ]; then
			args="${args}/${DB_PASSWORD}";
		fi
		if [ "${DB_HOSTSPEC}" != "" ]; then
			sid=${DB_HOSTSPEC#*SID=}
			sid=${sid%%)*}
			args="${args}@${sid}";
		fi
		if [[ -z $remote ]];then
			args="$args file=${dumpfile}"
		else
			args="$args file=${remotefile}"
		fi
		if [[ -z $remote ]];then
			exp ${args}
		else
			ssh ${remote} "exp ${args}"
			scp ${remote}:${remotefile} ${dumpfile}
			[[ $? == 0 ]] && ssh ${remote} "rm -f ${remotefile}"
		fi
		if [ $? -gt 0 ]; then
			echo "Unable to create dumpfile ${dumpfile}."
			echo "Aborting."
			exit 5
		fi
	;;

	*)
		echo "ERROR: DATABASE TYPE '${DB_TYPE}' NOT KNOWN" >&2;
		exit 6
esac

if [ -z ${backupdir} ]; then
	backupdir="."
else
	if [ ! -d ${backupdir} ]; then
		mkdir -p ${backupdir}
		if [ $? -gt 0 ]; then
			echo "Unable to make directory ${backupdir}. Exiting now."
			exit 7
		fi
	fi
fi

if [ -z ${backupfilename} ]; then
	backupfilename="matrix-`date +%Y-%m-%d_%H-%M`-backup.tar"
fi

#
# We do the tar and gzip separately so systems like Solaris
# that don't support gzip in tar will work.
#

#
# -C `dirname $SYSTEM_ROOT` means we change to the directory below this one.
# The `basename $SYSTEM_ROOT` means we tar up this directory only (no full paths) in the tarball
# If we're not specifying the file location, then we need to make sure this won't be a recursive tarball!
# Hence the --exclude....
#

tar -C `dirname ${SYSTEM_ROOT}` -cv -f ${backupdir}/${backupfilename} `basename ${SYSTEM_ROOT}` --exclude=${backupfilename} --exclude=${SYSTEM_ROOT}/cache/* --exclude=matrix-*-backup.tar*

gzip -f ${backupdir}/${backupfilename}
if [ $? -gt 0 ]; then
	echo "Unable to gzip tarball ${backupdir}/${backupfilename}."
	echo "Aborting."
	exit 9
fi

rm -f ${dumpfile}
if [ $? -gt 0 ]; then
	echo "Unable to clean up dumpfile ${dumpfile}."
	echo "Aborting."
	exit 10
fi

echo ""
echo "Your system is backed up to ${backupdir}/${backupfilename}.gz"
echo ""

exit 0
