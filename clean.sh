#!/bin/sh
# Creates a clean system by removing data and cache directories 
# and clearing out the database and re-inserting the create script


SYSTEM_ROOT=`dirname "$0"`;


rm -rf "${SYSTEM_ROOT}/cache" \
		"${SYSTEM_ROOT}/data/file_repository" \
		"${SYSTEM_ROOT}/data/public/assets" \
		"${SYSTEM_ROOT}/data/public/asset_types" \
		"${SYSTEM_ROOT}/data/private/assets" \
		"${SYSTEM_ROOT}/data/private/db" \
		"${SYSTEM_ROOT}/data/private/asset_map"

cvs up -dP cache data/public data/private

DB_DSN=`grep "SQ_CONF_DB_DSN" "${SYSTEM_ROOT}/data/private/conf/main.inc" | sed "s/^define('SQ_CONF_DB_DSN', '\([^']\+\)');$/\1/"`
lines=`echo "${DB_DSN}" | wc -l`
if [ "${lines}" -lt "1" ]; then
	echo "ERROR: Database DSN Not Known" >&2;
	exit 1;
elif [ "${lines}" -gt "1" ]; then
	echo "ERROR: Multiple Database DSN Entries Found" >&2;
	exit 1;
fi
echo "DB DSN  : ${DB_DSN}";
DB_TYPE=`echo "${DB_DSN}" | sed "s/^\([^:]\+\):\/\/.*$/\1/"`;
echo "DB TYPE : ${DB_TYPE}";
DB_NAME=`echo "${DB_DSN}" | sed "s/^.*\/\([^/]\+\)$/\1/"`;
echo "DB NAME : ${DB_NAME}";

case "${DB_TYPE}" in 
	"mysql")
		mysql -u root -e "SHOW TABLES;" -s -N "${DB_NAME}" | sed 's/^.*$/DROP TABLE &;/' | mysql -u root "${DB_NAME}"
	;;

	"pgsql")
		psql -d "${DB_NAME}" -c "\d" -t -q -A -X | awk -F\| '{ print "DROP " $3 " " $2 ";" }' | psql -d "${DB_NAME}" -X -q
	;;

	*)
		echo "ERROR: DATABASE TYPE '${DB_TYPE}' NOT KNOWN" >&2;
		exit 1;
esac

# now just run step 2 again
php -d output_buffering=0 "${SYSTEM_ROOT}/install/step_02.php" "${SYSTEM_ROOT}"
if [ "$?" = "0" ]; then
	php -d output_buffering=0 "${SYSTEM_ROOT}/install/step_03.php" "${SYSTEM_ROOT}"
fi

chmod 775 cache
find data -type d -exec chmod 2775 {} \; 2> /dev/null

