#!/bin/sh
if [ $(id -u) -gt 0 ]; then
	echo "You must be root to run this script, exiting"
	exit 1
fi

if [ ! -d supervise ]; then
	echo "You must run this script from the directory it lives in"
	exit 1
fi

echo -n "What is the name of the user your webserver runs as [htdocs] ?"
read webuser
if [ -z $webuser ]; then
	webuser="htdocs"
fi

echo -n "Fixing supervise scripts for your environment "
sed < supervise/run > supervise/run.tmp \
-e "s%WEBUSER=\".*\"%WEBUSER=\"$WEBUSER\"%" && mv supervise/run.tmp supervise/run
echo -n "."

sed < supervise/log/run > supervise/log/run.tmp \
-e "s%WEBUSER=\".*\"%WEBUSER=\"$WEBUSER\"%" && mv supervise/log/run.tmp supervise/log/run
echo -n "."

chmod 755 supervise/{,log}/run
echo "done."

echo -n "Setting up log directories "
mkdir -p /var/log/hipo
echo -n "."
chown $webuser. -R /var/log/hipo
echo "done."

echo ""
echo "Now create your server.conf file (see the hipo documentation and the server.conf.example file)"
echo "then review your supervise scripts and when you are happy with them create a symlink"
echo "in your /service directory to the supervise directory here"
echo "e.g. ln -s /var/www/matrix/scripts/hipo_server/supervise /service/hipo"

