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
#* $Id: backup.sh,v 1.27.2.3 2009/10/21 03:02:19 csmith Exp $
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

If you want to, you can also name your backup files a particular way.
If you do this, you *must* specify where to put the file (even if it's the current directory)
otherwise the backup filename will be mistaken for the backup directory.
You must also specify the extension(s), eg 'filename.tar.gz' or 'filename.tgz'

$0 /path/to/matrix /path/to/backup/folder filename.tar.gz

You can view progress of the script by using --verbose:
$0 /path/to/matrix [/path/to/backup/folder] [filename] --verbose

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
	if [ "${CRON_RUN}" -eq 0 ]; then
		echo "$1"
	fi
}

print_verbose()
{
	if [ "${VERBOSE}" -eq 1 ]; then
		echo "$1"
	fi
}

# print a message to stderr
print_error()
{
	echo "$1" >&2
}

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

	schema_only=1
	if [ "x$6" = "x" ]; then
		schema_only=0
	fi

	# set a timeout for pg_dump
	# just in case it's on a remote server and it's down
	# or otherwise unavailable
	PGCONNECT_TIMEOUT=10
	export PGCONNECT_TIMEOUT

	if [ "x${host}" = "x" ]; then
		host='localhost'
	fi

	if [ "x${port}" = "x" ]; then
		port=5432
	fi

	if [ "x${username}" = "x" ]; then
		print_verbose ""
		print_error "You can't create a backup of database ${db} without a database username."
		print_error "The database has not been included in the backup."
		print_verbose ""
		return
	fi

	file_exists "pg_dump"
	if [ $? -gt 0 ]; then
		print_verbose ""
		print_error "Unable to create postgres dump."
		print_error "Make sure 'pg_dump' is in your path."
		print_error "The database has not been included in the backup."
		print_verbose ""
		exit
		return
	fi

	# We know the file exists so grab the path.
	pgdump=`which pg_dump`

	pgpass_filename="${SYSTEM_ROOT}/.pgpass_${db}"

	print_verbose "Creating pgpass file (${pgpass_filename})"

	if [ -f "${pgpass_filename}" ]; then
		print_info "pgpass file (${pgpass_filename}) already exists. Removing"
		rm -f ${pgpass_filename}
	fi

	pgpass_string="${host}:${port}:${db}:${username}"
	if [ "x${pass}" != "x" ]; then
		pgpass_string="${pgpass_string}:${pass}"
	fi

	print_verbose "Finished creating pgpass file."

	args="-i "
	if [ "x${host}" != "x" ]; then
 		if [ "x${host}" != "xlocalhost" ]; then
			args="${args} -h ${host} "
		fi
	fi
	args="${args} -n public -p ${port} -U ${username}"

	dumpfileprefix=${db}

	if [ "${schema_only}" -eq 1 ]; then
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
	oldpassfile=`echo $PGPASSFILE`
	PGPASSFILE=${pgpass_filename}
	export PGPASSFILE

	print_verbose "Dumping database out to ${dumpfile} .. "

	outputfile="${SYSTEM_ROOT}/.pgdumpoutput"

	${pgdump} ${args} ${db} -f ${dumpfile} 2> ${outputfile}

	if [ $? -gt 0 ]; then
		print_verbose ""
		print_error "*** Unable to create dumpfile ${dumpfile}."
		print_error ""
		output=`cat ${outputfile}`
		print_error "${output}"
		print_error ""
		print_verbose ""
	else
		print_verbose "Finished dumping database."
	fi

	rm -f ${outputfile}

	print_verbose "Cleaning up temp pgpass file .. "

	PGPASSFILE=${oldpassfile}
	export PGPASSFILE

	rm -f ${pgpass_filename}

	print_verbose "Finished cleaning up temp pgpass file."
}

if [ "x$1" = "x" ]; then
	print_error "The directory you supplied is not a matrix system."
	print_error ""
	print_usage
	exit 1
else
	SYSTEM_ROOT="$1"
	shift 1
fi

if [ ! -f "${SYSTEM_ROOT}/data/private/conf/main.inc" ]; then
	print_error "The directory you supplied is not a matrix system."
	print_error ""
	print_usage
	exit 1
fi

backupdir=""
REMOTE_USER=""
VERBOSE=0

if [ "$SYSTEM_ROOT" = "." ]; then
	SYSTEM_ROOT=`pwd`
fi

backupfilename_prefix=`basename $SYSTEM_ROOT`
backupfilename="${backupfilename_prefix}-`date +%Y-%m-%d_%H-%M`-backup.tar.gz"

while true; do
	case "$1" in
		--verbose)
			VERBOSE=1
		;;
		--remotedb=*)
			# /bin/sh doesnt expand *) and put it into $1
			# so do it ourselves with cut
			REMOTE_USER=`echo $1 | cut -d'=' -f2`
		;;
		*)
			if [ "x$backupdir" = "x" ]; then
				backupdir=$1
			else
				backupfilename=$1
			fi
		;;
	esac
	if [ "x$2" = "x" ]; then
		break;
	fi

	shift
done

if [ "x$backupdir" = "x" ]; then
	backupdir="."
fi

if [ ! -d "${backupdir}" ]; then
	mkdir -p "${backupdir}"
	if [ $? -gt 0 ]; then
		print_error "Unable to create backup dir (${backupdir})."
		print_error "Aborting"
		exit 1
	fi
fi

CRON_RUN=0
tty -s
if [ $? -gt 0 ]; then
	CRON_RUN=1
fi

if [ -f "${SYSTEM_ROOT}/.extra_backup_files" ]; then
	rm -f "${SYSTEM_ROOT}/.extra_backup_files"
fi

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
	print_error "Cannot find the php binary please be sure to install it"
	exit 1
fi

touch "${SYSTEM_ROOT}/.extra_backup_files"

# OK, what we are doing here is using PHP to do the parsing of the DSN for us (much less error prone :)
matrix_318_php_code="<?php
require_once '${SYSTEM_ROOT}/data/private/conf/db.inc';
function splitdsn(\$input_dsn, \$prefix='DB_')
{
        \$start_pos = strpos(\$input_dsn['DSN'], ':') + 1;
        \$dsn = preg_split('/[\s;]/', substr(\$input_dsn['DSN'], \$start_pos));
        foreach(\$dsn as \$dsn_v) {
                list(\$k, \$v) = explode('=', \$dsn_v);
		\$var = \$prefix . strtoupper(\$k);
		echo \$var .  '=\"' . addslashes(\$v) . '\";';
		echo 'export ' . \$var . ';';
        }

	\$var = \$prefix . 'USERNAME';
	echo \$var . '=\"' . addslashes(\$input_dsn['user']) . '\";';
	echo 'export ' . \$var.';';

	\$var = \$prefix . 'PASSWORD';
	echo \$var . '=\"' . addslashes(\$input_dsn['password']) . '\";';
	echo 'export ' . \$var.';';
}

\$var = 'DB_TYPE';
echo \$var . '=\"'.\$db_conf['db']['type'].'\";';
echo 'export ' . \$var.';';

if (\$db_conf['db']['type'] === 'pgsql') {
	splitdsn(\$db_conf['db']);

	if (\$db_conf['dbcache'] !== null) {
		splitdsn(\$db_conf['dbcache'], 'CACHE_DB_');
	}

	if (\$db_conf['dbsearch'] !== null) {
		splitdsn(\$db_conf['dbsearch'], 'SEARCH_DB_');
	}

} else {
	\$vars = array (
		'HOST' => 'DSN',
	   	'USERNAME' => 'user',
	   	'PASSWORD' => 'password',
	);

	foreach (\$vars as \$var => \$dsn_var) {
		echo 'DB_' . \$var . '=\"' . addslashes(\$db_conf['db'][\$dsn_var]) . '\";';
		echo 'export ' . \$var.';';
	}

	if (\$db_conf['dbcache'] !== null) {
		foreach (\$vars as \$var => \$dsn_var) {
			echo 'CACHE_DB_' . \$var . '=\"' . addslashes(\$db_conf['dbcache'][\$dsn_var]) . '\";';
			echo 'export ' . \$var.';';
		}
	}

	if (\$db_conf['dbsearch'] !== null) {
		foreach (\$vars as \$var => \$dsn_var) {
			echo 'SEARCH_DB_' . \$var . '=\"' . addslashes(\$db_conf['dbsearch'][\$dsn_var]) . '\";';
			echo 'export ' . \$var.';';
		}
	}
}
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

	echo \$prefix . 'TYPE=\"'.\$dsn['phptype'].'\";
	echo \$prefix . 'USERNAME=\"'.\$dsn['username'].'\";
	echo \$prefix . 'PASSWORD=\"'.\$dsn['password'].'\";
	echo \$prefix . 'HOST=\"'.\$dsn['hostspec'].'\";
	echo \$prefix . 'PORT=\"'.\$dsn['port'].'\";
	echo \$prefix . 'DBNAME=\"'.\$dsn['database'].'\";

	echo 'export ' . \$prefix . 'TYPE;';
	echo 'export ' . \$prefix . 'USERNAME;';
	echo 'export ' . \$prefix . 'PASSWORD;';
	echo 'export ' . \$prefix . 'HOST;';
	echo 'export ' . \$prefix . 'PORT;';
	echo 'export ' . \$prefix . 'DBNAME;';
}

parsedsn(SQ_CONF_DB_DSN);
if (SQ_CONF_DB_DSN !== SQ_CONF_DBCACHE_DSN) {
	parsedsn(SQ_CONF_DBCACHE_DSN, 'CACHE_DB_');
}
";

if [ -f ${SYSTEM_ROOT}/data/private/conf/db.inc ]; then
	print_verbose "Found a 3.18/3.20 system"
	eval `echo "${matrix_318_php_code}" | $PHP`
else
	print_verbose "Found a 3.16 system"
	eval `echo "${matrix_316_php_code}" | $PHP`
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

	schema_only=1
	if [ "x$5" = "x" ]; then
		schema_only=0
	fi


	# The oracle dsn is in the format of:
	# //localhost|ip.addr/dbname
	# Split it up so we just get the dbname, then set the oracle_sid to the right thing.
	db=`echo $hostspec | awk -F'/' '{ print $NF }'`
	old_oracle_sid=`echo $ORACLE_SID`
	if [ "x${remote_user}" = "x" ]; then
		export ORACLE_SID=$db
	fi

	# Also need the hostname to see if we need to do a remote dump
	# since we're not using ' ' as the separator,
	# the field we want is actually #3.
	dbhost=`echo $hostspec | awk -F'/' '{ print $3 }'`
	print_verbose ""
	print_verbose "Found a dbhost of ${dbhost}"
	print_verbose ""
	if [ "$dbhost" != "localhost" ]; then
		if [ "x$remote_user" = "x" ]; then
			print_verbose ""
			print_error "To do remote oracle backups, please supply '--remotedb=username@hostname'"
			print_error "The database has not been included in the backup."
			print_verbose ""
			return
		fi
	fi

	if [ "x$remote_user" = "x" ]; then
		file_exists "exp"
		rc=$?
		if [ $rc -eq 0 ]; then
			oracle_exp=`which exp`
		fi
	else
		oracle_exp=`ssh "${remote_user}" 'which exp'`
		rc=$?
	fi

	if [ $rc -gt 0 ]; then
		print_verbose ""
		print_error "Unable to create oracle dump."
		print_error "Make sure 'exp' is in your path."
		print_error "The database has not been included in the backup."
		print_verbose ""
		return
	fi

	if [ "x${remote_user}" = "x" ]; then
		home=`echo $ORACLE_HOME`
	else
		home=`ssh "${remote_user}" 'echo $ORACLE_HOME'`
	fi
	if [ $? -gt 0 ] || [ "${home}" == "" ]; then
		print_verbose ""
		print_error "Unable to create oracle dump."
		print_error "Make sure the 'ORACLE_HOME' environment variable is set"
		print_error "The database has not been included in the backup."
		print_verbose ""
		return
	fi

	print_verbose "Creating oracle db dump .. "

	dump_args="${username}/${pass}@localhost/${db}"

	dumpfileprefix=${db}
	if [ ${schema_only} -eq 1 ]; then
		dumpfileprefix="${dumpfileprefix}-schema"
	fi

	if [ "x${remote_user}" = "x" ]; then
		dumpfilepath=${SYSTEM_ROOT}
	else
		dumpfilepath='.'
	fi

	dumpfile=${dumpfilepath}/${dumpfileprefix}-`date +%Y-%m-%d_%H-%M`.dump

	echo "${dumpfile}" >> "${SYSTEM_ROOT}/.extra_backup_files"

	print_verbose "Dumping database out to ${dumpfile} .. "

	oracle_args="consistent=y"

	outputfile="${SYSTEM_ROOT}/oracleoutput"
	if [ "x${remote_user}" = "x" ]; then
		`${oracle_exp} ${dump_args} ${oracle_args} File=${dumpfile} 2> ${outputfile}`
	else
		outputfile='./oracleoutput'
		ssh "${remote_user}" "${oracle_exp} ${dump_args} ${oracle_args} File=${dumpfile} 2> ${outputfile}"
	fi

	if [ $? -gt 0 ]; then
		print_error "The oracle dump may have contained errors. Please check it's ok"
		if [ ! -e $remote_user ]; then
			output=`cat ${outputfile}`
		else
			output=`ssh "${remote_user}" cat ${outputfile}`
		fi
		print_error $output
	fi

	if [ "x${remote_user}" = "x" ]; then
		rm -f ${outputfile}
	else
		scp -q "${remote_user}:${dumpfile}" "${SYSTEM_ROOT}/${dumpfile}"
		if [ $? -gt 0 ]; then
			print_error "Unable to copy the oracle dump file back."
			print_error "Tried to run"
			print_error "scp ${remote_user}:${dumpfile} ${SYSTEM_ROOT}/${dumpfile}"
			print_error "The database has not been included in the backup."
			return
		fi
		ssh "${remote_user}" 'rm -f ${outputfile} ${dumpfile}'
	fi

	if [ "x${remote_user}" = "x" ]; then
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
		print_error "ERROR: DATABASE TYPE '${DB_TYPE}' NOT KNOWN"
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

sysroot_base=`basename "${SYSTEM_ROOT}"`
sysroot_dir=`dirname ${SYSTEM_ROOT}`

# So we get the right relative paths for the exclude file,
# so solaris tar excludes them properly:
#
# go to the dir under the SYSTEM_ROOT
# then do a find
# so the paths end up as:
# folder/filename
#

mydir=`pwd`
cd "${sysroot_dir}/"

print_verbose "Creating an exclude file .. "
exclude_file="${mydir}/tar_exclude_file"
echo "${exclude_file}" > "${exclude_file}"
echo "${sysroot_base}/tar_exclude_file" >> "${exclude_file}"
echo "${sysroot_base}/${backupfilename}" >> "${exclude_file}"
echo "${sysroot_base}/.extra_backup_files" >> "${exclude_file}"
echo "${backupdir}/${backupfilename}" >> "${exclude_file}"

for file in `find "${sysroot_base}" -name cache -o -name data -prune -o -type f -name '*-backup.tar*' -print`; do
	echo "${file}" >> "${exclude_file}"
done

echo "${sysroot_base}/cache/*" >> "${exclude_file}"
print_verbose "Done"
print_verbose ""

cd "${mydir}"

# Of course the tar syntax is slightly different for different os's
os=`uname`

# if tar supports gzip itself, we will do everything in one step
# if it doesn't (solaris tar for example does not)
# it will be done in two steps
# - 1) tar the system
# - 2) gzip the tarball.

tar_gzip=0
case "${os}" in
	"SunOS")
		file_exists "gtar"
		if [ $? -eq 0 ]; then
			tar_gzip=1
			tar_command=`which gtar`
		fi
	;;

	*)
		tar_gzip=1
		tar_command=`which tar`
esac

if [ "${tar_gzip}" -eq 0 ]; then
	# if gtar is not present, use the solaris tar & then gzip the tarball.
	# solaris tar doesn't support gzipping in the same process.

	# get rid of the .gz extension
	backupfilename=`echo ${backupfilename} | sed -e 's/\.gz$//'`

	print_verbose "Tar'ing up the ${SYSTEM_ROOT} folder to ${backupdir}/${backupfilename} .. "

	tar -cfX "${backupdir}/${backupfilename}" "${exclude_file}" -C `dirname ${SYSTEM_ROOT}` "${sysroot_base}"

	print_verbose "Gzipping ${backupdir}/${backupfilename} .. "

	gzip -f ${backupdir}/${backupfilename}

	# gzip *always* adds a .gz extension. You can't stop it.
	backupfilename="${backupfilename}.gz"

	if [ $? -gt 0 ]; then
		print_verbose ""
		print_error "*** Unable to gzip tarball ${backupdir}/${backupfilename}."
		print_verbose ""
	else
		print_verbose "Finished gzipping up ${backupdir}/${backupfilename}."
	fi
else
	print_verbose "Tar'ing & gzipping up the ${SYSTEM_ROOT} folder to ${backupdir}/${backupfilename} .. "
	# dereference is in case there is a symlink either to matrix or inside the matrix folder
	"${tar_command}" --dereference -czf "${backupdir}/${backupfilename}" -X "${exclude_file}" -C `dirname ${SYSTEM_ROOT}` "${sysroot_base}"
	print_verbose "Finished Tar'ing & gzipping up the ${SYSTEM_ROOT} folder to ${backupdir}/${backupfilename}."
fi

print_verbose ""
print_verbose "Removing tar exclude list .. "
rm -f "${exclude_file}"
print_verbose "Done"
print_verbose ""

print_verbose "Cleaning up .. "

files=`cat ${SYSTEM_ROOT}/.extra_backup_files`
for file in $files; do
	rm -f "${file}"
	if [ $? -gt 0 ]; then
		print_verbose ""
		print_error "Unable to clean up file ${file}."
		print_verbose ""
	fi
done

file="${SYSTEM_ROOT}/.extra_backup_files"
rm -f "${file}"
if [ $? -gt 0 ]; then
	print_verbose ""
	print_error "Unable to clean up file ${file}."
	print_verbose ""
fi

print_verbose "Finishing cleaning up."

print_info ""
print_info "Your system is backed up to ${backupdir}/${backupfilename}"
print_info ""

exit 0

