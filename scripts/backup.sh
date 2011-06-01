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
#* $Id: backup.sh,v 1.17 2009/03/20 00:12:12 csmith Exp $
#*
#*/
#
# See print_usage for what the script does.
#

print_usage()
{
cat <<EOF
Backup a matrix system using this script.
The script will dump the database and tar & gzip the matrix folder.

Pass a matrix folder name as the first argument, eg:
$0 /path/to/matrix

If you want to, pass a folder name to place the backups in as the second argument, eg:
$0 /path/to/matrix /path/to/backup/folder

It defaults to the current dir.

You can view progress of the script by using --verbose:
$0 /path/to/matrix [/path/to/backup/folder] --verbose

You can specify a user to ssh to a remote server and do a backup through.
This is used for oracle backups since the 'exp' and 'expdb' utilities are server only (not in the oracle client package)
$0 /path/to/matrix [/path/to/backup/folder] [--verbose] --remotedb=username@hostname

It will try to ssh to the remote server using username@hostname

EOF
	exit 1
}

# print_info
# Prints a message if the script isn't being run via cron.
print_info()
{
	if [ ${CRON_RUN} -eq 0 ]; then
		echo $1
	fi
}
print_verbose()
{
	if [ ${VERBOSE} -eq 1 ]; then
		echo $1
	fi
}

# Usage:
# pg_dbdump $dbname $username $pass $host $port [$schema_only]
#
# even if pass/host/port are empty.
# If $schema_only is not supplied, a full database dump is created.

pg_dbdump()
{
	db=$1
	username=$2
	pass=$3
	host=$4
	port=$5

	schema_only=0
	if [ -n $6 ]; then
		schema_only=1
	fi

	if [ "${host}" = "" ]; then
		host='localhost'
	fi

	if [ "${port}" = "" ]; then
		port=5432
	fi

	if [ ${username} = "" ]; then
		print_verbose ""
		echo "You can't create a backup of database ${db} without a database username."
		echo "The database has not been included in the backup."
		print_verbose ""
		return
	fi

	pgdump=$(which pg_dump)
	if [ $? -gt 0 ]; then
		print_verbose ""
		echo "Unable to create postgres dump."
		echo "Make sure 'pg_dump' is in your path."
		echo "The database has not been included in the backup."
		print_verbose ""
		return
	fi

	pgpass_filename="${SYSTEM_ROOT}/.pgpass_${db}"

	print_verbose "Creating pgpass file (${pgpass_filename})"

	if [ -f ${pgpass_filename} ]; then
		print_info "pgpass file (${pgpass_filename}) already exists. Removing"
		rm -f ${pgpass_filename}
	fi

	pgpass_string="${host}:${port}:${db}:${username}"
	if [ "${pass}" != "" ]; then
		pgpass_string="${pgpass_string}:${pass}"
	fi

	print_verbose "Finished creating pgpass file."

	args="-i "
	if [ "${host}" != "localhost" ]; then
		args="${args} -h ${host} "
	fi
	args="${args} -p ${port} -U ${username}"

	dumpfileprefix=${db}

	if [ ${schema_only} -eq 1 ]; then
		args="${args} -s";
		dumpfileprefix="${dumpfileprefix}-schema"
		print_verbose "Doing a schema only dump of ${db}"
	else
		print_verbose "Doing a complete dump of ${db}"
	fi

	dumpfile=${SYSTEM_ROOT}/${dumpfileprefix}-`date +%Y-%m-%d_%H-%M`.dump

	echo "${dumpfile}" >> "${SYSTEM_ROOT}/.extra_backup_files"

	echo $pgpass_string > ${pgpass_filename}

	chmod 600 ${pgpass_filename}
	oldpassfile=$(echo $PGPASS)
	export PGPASSFILE=${pgpass_filename}

	print_verbose "Dumping database out to ${dumpfile} .. "

	outputfile="${SYSTEM_ROOT}/pgdumpoutput"
	${pgdump} ${args} "${db}" > ${dumpfile} 2>${outputfile}

	if [ $? -gt 0 ]; then
		print_verbose ""
		echo "*** Unable to create dumpfile ${dumpfile}."
		echo ""
		cat ${outputfile}
		print_verbose ""
	else
		print_verbose "Finished dumping database."
	fi

	rm -f ${outputfile}

	print_verbose "Cleaning up temp pgpass file .. "

	export PGPASSFILE=${oldpassfile}
	rm -f ${pgpass_filename}

	print_verbose "Finished cleaning up temp pgpass file."
}

if [ -z $1 ]; then
	print_usage
	exit 1
fi

SYSTEM_ROOT=$(readlink -f "$1")

shift 1

if [ ! -f ${SYSTEM_ROOT}/data/private/conf/main.inc ]; then
	echo "The directory you supplied is not a matrix system."
	echo ""
	print_usage
	exit 1
fi

backupdir="."
REMOTE_USER=""
VERBOSE=0

while true; do
	case "$1" in
		--verbose)
			VERBOSE=1
		;;
		--remotedb=*)
			REMOTE_USER=$1
		;;
		*)
			if [ "$1" != '' ]; then
				backupdir=$1
			fi
		;;
	esac
	if [ -z $2 ]; then
		break;
	fi

	shift
done

if [ ! -d "${backupdir}" ]; then
	mkdir -p "${backupdir}"
	if [ $? -gt 0 ]; then
		echo "Unable to create backup dir (${backupdir})."
		echo "Aborting"
		exit 1
	fi
fi

BACKUPFILENAME_PREFIX=$(basename $SYSTEM_ROOT)
backupfilename="${BACKUPFILENAME_PREFIX}-`date +%Y-%m-%d_%H-%M`-backup.tar"

CRON_RUN=0
tty -s
if [ $? -gt 0 ]; then
	CRON_RUN=1
fi

if [ -f "${SYSTEM_ROOT}/.extra_backup_files" ]; then
	rm -f "${SYSTEM_ROOT}/.extra_backup_files"
fi

touch "${SYSTEM_ROOT}/.extra_backup_files"

if [ ! -z ${PHP} ] && [ -e ${PHP} ]; then
	PHP=${PHP}
elif $(which php-cli 2>/dev/null >/dev/null); then
	PHP="php-cli"
elif $(which php 2>/dev/null >/dev/null); then
	PHP="php"
else
	echo "Cannot find the php binary please be sure to install it"
	exit 1
fi

# OK, what we are doing here is using PHP to do the parsing of the DSN for us (much less error prone :)
matrix_318_php_code="<?php
require_once '${SYSTEM_ROOT}/data/private/conf/db.inc';
function splitdsn(\$input_dsn, \$prefix='DB_')
{
        \$start_pos = strpos(\$input_dsn['DSN'], ':') + 1;
        \$dsn = preg_split('/[\s;]/', substr(\$input_dsn['DSN'], \$start_pos));
        foreach(\$dsn as \$dsn_v) {
                list(\$k, \$v) = explode('=', \$dsn_v);
                echo 'export ' . \$prefix .strtoupper(\$k).'=\"'.addslashes(\$v).'\";';
        }

        echo 'export ' . \$prefix . 'USERNAME=\"'.\$input_dsn['user'].'\";';
        echo 'export ' . \$prefix . 'PASSWORD=\"'.\$input_dsn['password'].'\";';
}

echo 'export DB_TYPE=\"'.\$db_conf['db']['type'].'\";';

if (\$db_conf['db']['type'] === 'pgsql') {
	splitdsn(\$db_conf['db']);

	if (\$db_conf['dbcache'] !== null) {
		splitdsn(\$db_conf['dbcache'], 'CACHE_DB_');
	}

	if (\$db_conf['dbsearch'] !== null) {
		splitdsn(\$db_conf['dbsearch'], 'SEARCH_DB_');
	}

} else {
	echo 'export DB_HOST=\"'.\$db_conf['db']['DSN'].'\";';
	echo 'export DB_USERNAME=\"'.\$db_conf['db']['user'].'\";';
	echo 'export DB_PASSWORD=\"'.\$db_conf['db']['password'].'\";';

	if (\$db_conf['dbcache'] !== null) {
		echo 'export CACHE_DB_HOST=\"'.\$db_conf['dbcache']['DSN'].'\";';
		echo 'export CACHE_DB_USERNAME=\"'.\$db_conf['dbcache']['user'].'\";';
		echo 'export CACHE_DB_PASSWORD=\"'.\$db_conf['dbcache']['password'].'\";';
	}

	if (\$db_conf['dbsearch'] !== null) {
		echo 'export SEARCH_DB_HOST=\"'.\$db_conf['dbsearch']['DSN'].'\";';
		echo 'export SEARCH_DB_USERNAME=\"'.\$db_conf['dbsearch']['user'].'\";';
		echo 'export SEARCH_DB_PASSWORD=\"'.\$db_conf['dbsearch']['password'].'\";';
	}
}
?>
"

matrix_316_php_code="<?php
define('SQ_SYSTEM_ROOT', '${SYSTEM_ROOT}');
define('SQ_LOG_PATH',    SQ_SYSTEM_ROOT.'/data/private/logs');
require_once '${SYSTEM_ROOT}/data/private/conf/main.inc';
require_once 'DB.php';

function parsedsn(\$input_dsn, \$prefix='DB_')
{
	\$dsn = DB::parseDSN(\$input_dsn);

	if (\$dsn['phptype'] == 'oci8') {
		\$dsn['phptype'] = 'oci';
	}

	echo \$prefix . 'TYPE=\"'.\$dsn['phptype'].'\";';
	echo \$prefix . 'USERNAME=\"'.\$dsn['username'].'\";';
	echo \$prefix . 'PASSWORD=\"'.\$dsn['password'].'\";';
	echo \$prefix . 'HOST=\"'.\$dsn['hostspec'].'\";';
	echo \$prefix . 'PORT=\"'.\$dsn['port'].'\";';
	echo \$prefix . 'DBNAME=\"'.\$dsn['database'].'\";';
}

parsedsn(SQ_CONF_DB_DSN);
if (SQ_CONF_DB_DSN !== SQ_CONF_DBCACHE_DSN) {
	parsedsn(SQ_CONF_DBCACHE_DSN, 'CACHE_DB_');
}
?>
";

if [ -f ${SYSTEM_ROOT}/data/private/conf/db.inc ]; then
	print_verbose "Found a 3.18/3.20 system"
	eval $(echo "${matrix_318_php_code}" | $PHP)
else
	print_verbose "Found a 3.16 system"
	eval $(echo "${matrix_316_php_code}" | $PHP)
fi

# Usage:
# oracle_dbdump $remote_user $user $pass $hostspec
# the hostspec is
# //localhost|ip.addr/dbname
# which is broken up inside the fn to do the right thing.
oracle_dbdump()
{
	remote_user=$1
	username=$2
	pass=$3
	hostspec=$4

	schema_only=0
	if [ -n $5 ]; then
		schema_only=1
	fi

	# The oracle dsn is in the format of:
	# //localhost|ip.addr/dbname
	# Split it up so we just get the dbname, then set the oracle_sid to the right thing.
	db=$(echo $hostspec | awk -F'/' '{ print $NF }')
	old_oracle_sid=$(echo $ORACLE_SID)
	if [ -z ${remote_user} ]; then
		export ORACLE_SID=$db
	fi

	# Also need the hostname to see if we need to do a remote dump
	dbhost=$(echo $hostspec | awk -F'/' '{ print $1 }')
	print_verbose ""
	print_verbose "Found a dbhost of ${dbhost}"
	print_verbose ""
	if [ "$dbhost" != "localhost" ]; then
		if [ -z $remote_user ]; then
			print_verbose ""
			echo "To do remote oracle backups, please supply '--remotedb=username@hostname'"
			echo "The database has not been included in the backup."
			print_verbose ""
			return
		fi
	fi

	if [ -z $remote_user ]; then
		oracle_exp=$(which exp)
	else
		oracle_exp=$(ssh "${remote_user}" 'which exp')
	fi

	if [ $? -gt 0 ]; then
		print_verbose ""
		echo "Unable to create oracle dump."
		echo "Make sure 'exp' is in your path."
		echo "The database has not been included in the backup."
		print_verbose ""
		return
	fi

	if [ -z $remote_user ]; then
		home=$(echo $ORACLE_HOME)
	else
		home=$(ssh "${remote_user}" 'echo $ORACLE_HOME')
	fi
	if [ $? -gt 0 ] || [ "${home}" = "" ]; then
		print_verbose ""
		echo "Unable to create oracle dump."
		echo "Make sure the 'ORACLE_HOME' environment variable is set"
		echo "The database has not been included in the backup."
		print_verbose ""
		return
	fi

	print_verbose "Creating oracle db dump .. "

	dump_args="${username}/${pass}@localhost/${db}"

	dumpfileprefix=${db}
	if [ ${schema_only} -eq 1 ]; then
		dumpfileprefix="${dumpfileprefix}-schema"
	fi

	if [ -z ${remote_user} ]; then
		dumpfilepath=${SYSTEM_ROOT}
	else
		dumpfilepath='.'
	fi

	dumpfile=${dumpfilepath}/${dumpfileprefix}-`date +%Y-%m-%d_%H-%M`.dump

	echo "${dumpfile}" >> "${SYSTEM_ROOT}/.extra_backup_files"

	print_verbose "Dumping database out to ${dumpfile} .. "

	oracle_args="consistent=y"

	outputfile="${SYSTEM_ROOT}/oracleoutput"
	if [ -z $remote_user ]; then
		$(${oracle_exp} ${dump_args} ${oracle_args} File=${dumpfile} 2> ${outputfile})
	else
		outputfile='./oracleoutput'
		ssh "${remote_user}" "${oracle_exp} ${dump_args} ${oracle_args} File=${dumpfile} 2> ${outputfile}"
	fi

	if [ $? -gt 0 ]; then
		echo "The oracle dump may have contained errors. Please check it's ok"
		if [ -z $remote_user ]; then
			output=$(cat ${outputfile})
		else
			output=$(ssh "${remote_user}" cat ${outputfile})
		fi
		echo $output
	fi

	if [ -z $remote_user ]; then
		rm -f ${outputfile}
	else
		scp -q "${remote_user}:${dumpfile}" "${SYSTEM_ROOT}/${dumpfile}"
		if [ $? -gt 0 ]; then
			echo "Unable to copy the oracle dump file back."
			echo "Tried to run"
			echo "scp ${remote_user}:${dumpfile} ${SYSTEM_ROOT}/${dumpfile}"
			echo "The database has not been included in the backup."
			return
		fi
		ssh "${remote_user}" 'rm -f ${outputfile} ${dumpfile}'
	fi

	if [ -z ${remote_user} ]; then
		export ORACLE_SID=$old_oracle_sid
	fi

	print_verbose "Finished dumping database."
}

case "${DB_TYPE}" in
	"pgsql")
		pg_dbdump "${DB_DBNAME}" "${DB_USERNAME}" "${DB_PASSWORD}" "${DB_HOST}" "${DB_PORT}"
		# If the cache db variable is set,
		# do a schema only dump of the cache db.
		if [ "${CACHE_DB_DBNAME}" ]; then
			pg_dbdump "${CACHE_DB_DBNAME}" "${CACHE_DB_USERNAME}" "${CACHE_DB_PASSWORD}" "${CACHE_DB_HOST}" "${CACHE_DB_PORT}" 1
		fi
	;;

	"oci")
		oracle_dbdump "${REMOTE_USER}" "${DB_USERNAME}" "${DB_PASSWORD}" "${DB_HOST}"

		# If the cache db variable is set,
		# do a schema only dump of the cache db.
		if [ "${CACHE_DB_DBNAME}" ]; then
			oracle_dbdump "${REMOTE_USER}" "${DB_USERNAME}" "${DB_PASSWORD}" "${DB_HOST}" 1
		fi
	;;

	*)
		echo "ERROR: DATABASE TYPE '${DB_TYPE}' NOT KNOWN" >&2;
		exit 6
esac

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

sysroot_base=$(basename "${SYSTEM_ROOT}")
sysroot_dir=$(dirname ${SYSTEM_ROOT})

# So we get the right relative paths for the exclude file,
# so solaris tar excludes them properly:
#
# go to the dir under the SYSTEM_ROOT
# then do a find
# so the paths end up as:
# folder/filename
#

mydir=$(pwd)
cd "${sysroot_dir}/"

print_verbose "Creating an exclude file .. "
echo "${sysroot_base}/${backupfilename}" > ${mydir}/tar_exclude_list
echo "${sysroot_base}/.extra_backup_files" >> ${mydir}/tar_exclude_list
echo "${backupdir}/${backupfilename}" >> ${mydir}/tar_exclude_list

for file in $(find "${sysroot_base}" \( -path "${sysroot_base}/cache" -o -path "${sysroot_base}/data" \) -prune -o -type f -name '*-backup.tar*' -print); do
	echo "${file}" >> ${mydir}/tar_exclude_list
done

echo "${sysroot_base}/cache" >> ${mydir}/tar_exclude_list
print_verbose "Done"
print_verbose ""

cd "${mydir}"

print_verbose "Tar'ing up the ${SYSTEM_ROOT} folder to ${backupdir}/${backupfilename} .. "

# Of course the tar syntax is slightly different for different os's
os=$(uname)
case "${os}" in
	"SunOS")
		tar -cfX "${backupdir}/${backupfilename}" ${mydir}/tar_exclude_list -C $(dirname ${SYSTEM_ROOT}) "${sysroot_base}"
	;;

	*)
		tar -cf "${backupdir}/${backupfilename}" -X ${mydir}/tar_exclude_list -C $(dirname ${SYSTEM_ROOT}) "${sysroot_base}"
esac

print_verbose "Finished Tar'ing up the ${SYSTEM_ROOT} folder to ${backupdir}/${backupfilename}."

print_verbose ""
print_verbose "Removing tar exclude list .. "
rm -f ${mydir}/tar_exclude_list
print_verbose "Done"
print_verbose ""

print_verbose "Gzipping ${backupdir}/${backupfilename} .. "

gzip -f ${backupdir}/${backupfilename}
if [ $? -gt 0 ]; then
	print_verbose ""
	echo "*** Unable to gzip tarball ${backupdir}/${backupfilename}."
	print_verbose ""
fi

print_verbose "Finished gzipping up ${backupdir}/${backupfilename}."
print_verbose "Cleaning up .. "

files=$(cat ${SYSTEM_ROOT}/.extra_backup_files)
for file in $files; do
	rm -f "${file}"
	if [ $? -gt 0 ]; then
		print_verbose ""
		echo "Unable to clean up file ${file}."
		print_verbose ""
	fi
done

file="${SYSTEM_ROOT}/.extra_backup_files"
rm -f "${file}"
if [ $? -gt 0 ]; then
	print_verbose ""
	echo "Unable to clean up file ${file}."
	print_verbose ""
fi

print_verbose "Finishing cleaning up."

print_info ""
print_info "Your system is backed up to ${backupdir}/${backupfilename}.gz"
print_info ""

exit 0

