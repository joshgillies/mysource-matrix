#!/bin/bash
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
#* $Id: backup.sh,v 1.11.8.1 2008/05/15 05:26:19 bshkara Exp $
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

if [[ -n ${PHP} ]] && [[ -e ${PHP} ]];then
	PHP=${PHP}
elif which php-cli 2>/dev/null >/dev/null;then
	PHP="php-cli"
elif which php 2>/dev/null >/dev/null;then
	PHP="php"
else
	echo "Cannot find the php binary please be sure to install it"
	exit 1
fi

# OK, what we are doing here is using PHP to do the parsing of the DSN for us (much less error prone :)
php_code="<?php
require_once '${SYSTEM_ROOT}/data/private/conf/db.inc';
if (\$db_conf['db']['type'] === 'pgsql') {
	\$start_pos = strpos(\$db_conf['db']['DSN'], ':') + 1;
	\$dsn = preg_split('/[\s;]/', substr(\$db_conf['db']['DSN'], \$start_pos));
	foreach(\$dsn as \$v) {
		list(\$k, \$v) = explode('=', \$v);
		echo 'DB_'.strtoupper(\$k).'=\"'.addslashes(\$v).'\";';
	}
} else {
	echo 'DB_HOST=\"'.\$db_conf['db']['DSN'].'\";';
}
echo 'DB_TYPE=\"'.\$db_conf['db']['type'].'\";';
echo 'DB_USERNAME=\"'.\$db_conf['db']['user'].'\";';
echo 'DB_PASSWORD=\"'.\$db_conf['db']['password'].'\";';
?>"

eval `echo "${php_code}" | $PHP`

set | grep "^DB_"

dumpfile=${SYSTEM_ROOT}/matrix-`date +%Y-%m-%d_%H-%M`.dump
[[ -n $remote ]] && remotefile="/tmp/matrix-`date +%Y-%m-%d_%H-%M`.dump"

case "${DB_TYPE}" in
	"pgsql")
		args="";
		if [ "${DB_USERNAME}" != "" ]; then
			args="${args} -U ${DB_USERNAME}";
		fi
		if [ "${DB_PASSWORD}" != "" ]; then
			echo "I can't pass the password automatically because the psql command only supports prompting.";
			args="${args} -W";
		fi
		if [ "${DB_HOST}" != "" ]; then
			args="${args} -h ${DB_HOST}";
		fi
		if [ "${DB_PORT}" != "" ]; then
			args="${args} -p ${DB_PORT}";
		fi
		if [[ -z $remote ]];then
			pg_dump ${args} "${DB_DBNAME}"  > ${dumpfile}
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
	"oci")
		args="";
		if [ "${DB_USERNAME}" != "" ]; then
			args="${args} ${DB_USERNAME}";
		fi
		if [ "${DB_PASSWORD}" != "" ]; then
			args="${args}/${DB_PASSWORD}";
		fi
		if [ "${DB_HOST}" != "" ]; then
			sid=${DB_HOST#*SID=}
			sid=${sid%%)*}
			args="${args}@${sid}";
		fi
		if [[ -z $remote ]];then
			args="$args file=${dumpfile}"
		else
			args="$args file=${remotefile}"
		fi

		# consistent=y makes sure all tables are correct to the same date
		args="$args consistent=y"

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

tar --exclude=${backupfilename} --exclude=${SYSTEM_ROOT}/cache/* --exclude=matrix-*-backup.tar* -C `dirname ${SYSTEM_ROOT}` -cv -f ${backupdir}/${backupfilename} `basename ${SYSTEM_ROOT}`

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
