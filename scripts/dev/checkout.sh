#!/bin/sh
USER="anonymous"
SERVER="public-cvs.squiz.net"
CVS_PUBLIC_PATH="/home/public"
CVS="/usr/bin/cvs"

print_usage()
{
	echo ""
	echo "This script will check out MySource Matrix based on a version number you supply."
	echo "It will check out into a directory called 'mysource_matrix'."
	echo "Version numbers are based on releases. They are named the same way:"
	echo "mysource_3-xx-y"
	echo "For example, if you want 3.24.2, the version number will be mysource_3-24-2"
	echo ""
	echo "So to use this script, it becomes:"
	echo "$0 mysource_3-24-2"
	echo ""
	echo "To install into a new directory, specify that as the second argument"
	echo "If this isn't specified, it will default to 'mysource_matrix'"
	echo "To do this, you must specify a version number to check out, eg:"
	echo "$0 mysource_3-24-2 mysource_matrix_3-24-2"
	exit 1
}

if [ "x$1" = "x" ]; then
	print_usage
	exit 1
fi

VERSION=$1

CHECKOUT_DIR="mysource_matrix"
if [ "x$2" != "x" ]; then
	CHECKOUT_DIR=$2
fi

if [ -d $CHECKOUT_DIR ]; then
	echo "Directory $CHECKOUT_DIR already exists, aborting."
	echo "Please specify a directory that doesn't exist."
	print_usage
	exit 1
fi

PACKAGES="bulkmail calendar cms data ecommerce filesystem funnelback google_maps import_tools ipb ldap news search sharepoint squid squiz_suite trim web_services"
FUDGE_PACKAGES="antivirus colour csv datetime_field db_extras dev file_versioning general image image_editor js_calendar ldap mollom rss_feeds standards_lists var_serialise wysiwyg"

echo "Checking out mysource matrix core .. "

$CVS -q -d :pserver:$USER:@$SERVER:$CVS_PUBLIC_PATH/core co -P -r $VERSION -d $CHECKOUT_DIR mysource_matrix > /dev/null

if [ $? -gt 0 ]; then
	echo "There was a problem checking out the matrix core"
	exit
fi

echo "Checking out mysource matrix packages .. "

cd $CHECKOUT_DIR/packages/

# these are done one by one because they are actually kept in separate repo's.
for package in $PACKAGES; do
	$CVS -q -d :pserver:$USER:@$SERVER:$CVS_PUBLIC_PATH/packages/$package co -P -r $VERSION $package > /dev/null
	if [ $? -gt 0 ]; then
		echo "There was a problem checking out the matrix package $package"
		echo "(Perhaps this package didn't exist with $VERSION)"
	fi
done

cd ../fudge/

# we can check everything out at once here because they are all just directories.
$CVS -q -d :pserver:$USER:@$SERVER:$CVS_PUBLIC_PATH/fudge co -P -r $VERSION $FUDGE_PACKAGES > /dev/null
if [ $? -gt 0 ]; then
	echo "There was a problem checking out the matrix 'fudge' $FUDGE_PACKAGES"
	echo "(Perhaps this package didn't exist with $VERSION)"
fi

cd ..

echo ""
echo "Everything has been checked out into the $CHECKOUT_DIR/ folder."
echo "Please visit http://matrix.squiz.net/resources/installation/ for installation instructions."
echo ""

