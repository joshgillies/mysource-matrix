#!/bin/sh
#
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
#* $Id: clean.sh,v 1.19 2009/10/13 05:41:04 csmith Exp $
#*/

# Creates a clean system by removing data and cache directories
# and clearing out the database and re-inserting the create script


SYSTEM_ROOT=`pwd`;

rm -rf "${SYSTEM_ROOT}/cache" \
		"${SYSTEM_ROOT}/data/file_repository" \
		"${SYSTEM_ROOT}/data/public/assets" \
		"${SYSTEM_ROOT}/data/public/asset_types" \
		"${SYSTEM_ROOT}/data/private/logs" \
		"${SYSTEM_ROOT}/data/private/assets" \
		"${SYSTEM_ROOT}/data/private/db" \
		"${SYSTEM_ROOT}/data/private/events" \
		"${SYSTEM_ROOT}/data/private/asset_map" \
		"${SYSTEM_ROOT}/data/private/maps" \
		"${SYSTEM_ROOT}/data/private/conf/system_assets.inc"

# Build the directory structure
dir_map="cache \
		data \
		data/private \
		data/public \
		data/temp \
		data/private/asset_map \
		data/private/asset_types \
		data/private/assets \
		data/private/conf \
		data/private/db \
		data/private/events \
		data/private/logs \
		data/private/packages \
		data/private/system \
		data/public/asset_types \
		data/public/assets \
		data/public/system \
		data/public/temp"

for dir in $dir_map; do
	if [ -d .git ]; then
		git checkout $dir
	else
		mkdir $dir;
	fi
done

# OK, what we are doing here is using PHP to do the parsing of the DSN for us (much less error prone :)
php_code="<?php
\$db_conf = require_once('${SYSTEM_ROOT}/data/private/conf/db.inc');
\$dsn = \$db_conf['db2'];
foreach(\$dsn as \$k => \$v) {
	echo 'DB_'.strtoupper(\$k).'=\"'.addslashes(\$v).'\";';
}

if (isset(\$dsn['DSN'])) {
	\$dsn_parts = Array();
	list(\$db_type, \$dsn_split) = preg_split('/:/', \$dsn['DSN']);
	\$dsn_split = preg_split('/[\\s;]+/', \$dsn_split);
	foreach (\$dsn_split as \$dsn_part) {
		\$split = preg_split('/=/', \$dsn_part, 2);
		echo 'DB_DSN_'.strtoupper(\$split[0]).'=\"'.\$split[1].'\";';
	}
}
?>"

eval `echo "${php_code}" | php`

set | grep "^DB_"

case "${DB_TYPE}" in
	"pgsql")
		args="";
		if [ "${DB_USER}" != "" ]; then
			args="${args} -U ${DB_USER}";
		fi
		if [ "${DB_PASSWORD}" != "" ]; then
			args="${args} -W";
		fi
		if [ "${DB_DSN_HOST}" != "" ]; then
			args="${args} -h ${DB_DSN_HOST}";
		fi
		if [ "${DB_DSN_PORT}" != "" ]; then
			args="${args} -p ${DB_DSN_PORT}";
		fi
		output=`psql ${args} -d "${DB_DSN_DBNAME}" -c "\d" -t -q -A -X | awk -F\| '{ print "DROP " $3 " " $2 " CASCADE;" }' | psql ${args} -d "${DB_DSN_DBNAME}" -X -q 2>&1`
		if [ $? -gt 0 ]; then
			echo "Unable to drop some items."
			echo $output
			exit 1
		fi
	;;

	"oci")

		export ORACLE_HOME="/usr/local/instantclient"
		SQLPLUS="$ORACLE_HOME/sqlplus"

		# The oracle dsn is in the format of:
		# //localhost|ip.addr/dbname
		# Split it up so we just get the dbname, then set the oracle_sid to the right thing.
		DB_NAME=`echo $DB_DSN | awk -F'/' '{ print $NF }'`
		old_oracle_sid=`echo $ORACLE_SID`
		export ORACLE_SID=$DB_NAME
		args="${DB_USER}/${DB_PASSWORD}@${DB_DSN}";
		$SQLPLUS -S "${args}" "@${SYSTEM_ROOT}/scripts/dev/oracle_drop.sql" "${DB_USER}";
		export ORACLE_SID=$old_oracle_sid
	;;

	*)
		echo "ERROR: DATABASE TYPE '${DB_TYPE}' NOT KNOWN" >&2;
		exit 1;
esac

echo ""
echo "Running step_02.php"
# now just run step 2 again
php -d output_buffering=0 "${SYSTEM_ROOT}/install/step_02.php" "${SYSTEM_ROOT}"
if [ "$?" != "0" ]; then
	exit 1
fi

echo ""
echo "Running compile_locale.php"
php -d output_buffering=0 "${SYSTEM_ROOT}/install/compile_locale.php" "${SYSTEM_ROOT}" "--locale=en"
if [ "$?" != "0" ]; then
	exit 1
fi

echo ""
echo "Running step_03.php"
php -d output_buffering=0 "${SYSTEM_ROOT}/install/step_03.php" "${SYSTEM_ROOT}"
# if step-3 failed then stop it here
if [ "$?" != "0" ]; then
	exit 1
fi

# again to ensure that all type descendants are able to be found
echo ""
echo "Running step_03.php again"
php -d output_buffering=0 "${SYSTEM_ROOT}/install/step_03.php" "${SYSTEM_ROOT}"
# if step-3 failed then stop it here
if [ "$?" != "0" ]; then
	exit 1
fi

echo ""
echo "Running compile_locale.php"
php -d output_buffering=0 "${SYSTEM_ROOT}/install/compile_locale.php" "${SYSTEM_ROOT}" "--locale=en"
if [ "$?" != "0" ]; then
	exit 1
fi

chmod 775 cache
find data -type d -exec chmod 2775 {} \; 2> /dev/null
find data -type f -exec chmod 664 {} \; 2> /dev/null

