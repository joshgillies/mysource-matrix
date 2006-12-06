<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: update_lookups.php,v 1.3 2006/12/06 05:39:51 bcaldwell Exp $
*
*/

/**
* Remaps a url to another url. Use this script instead of updating lookups as it should be
* quicker
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

// check that the correct root password was entered
$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	echo "ERROR: The root password entered was incorrect\n";
	exit();
}

echo "\n";

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

$db = &$GLOBALS['SQ_SYSTEM']->db;

$sql = 'SELECT urlid, url FROM sq_ast_url';
$root_urls = $db->getAssoc($sql);
assert_valid_db_result($root_urls);

ksort($root_urls);

foreach ($root_urls as $urlid => $url) {
	echo $urlid.'. '.$root_urls[$urlid]."\n";
}

echo "\n";

$chosen_url = -1;

while (!in_array($chosen_url, array_keys($root_urls))) {
	$chosen_url = get_line('Please enter the urlid of the url to change: ');
	if (!in_array($chosen_url, array_keys($root_urls))) {
		echo 'Invalid urlid!'."\n";
	}
}

$from_url   = $root_urls[$chosen_url];
$from_urlid = $chosen_url;
$to_url     = get_line('Please enter the url to change to: ');

$confirm = null;
while ($confirm != 'y' && $confirm != 'n') {
	$confirm = strtolower(get_line('Change '.$from_url .' to '.$to_url.' (y/n)? : '));
	if ($confirm != 'y' && $confirm != 'n') {
		echo 'Please answer y or n'."\n";
	} else if ($confirm == 'n') {
		exit();
	}
}

// update the root url in the asset url table to the new url
$sql = 'UPDATE
			sq_ast_url
		SET
				url = '.$db->quoteSmart($to_url).'
		WHERE
				urlid = '.$db->quoteSmart($from_urlid);

$result = $db->query($sql);
assert_valid_db_result($result);

// update any urls that use this url in the lookup and lookup value tables
foreach (Array('sq_ast_lookup_value', 'sq_ast_lookup') as $tablename) {

	$sql = 'UPDATE
				'.$tablename.'
			SET
				url = '.$db->quoteSmart($to_url).' || SUBSTR(url, '.strlen($from_url).' + 1)
			WHERE
				url LIKE '.$db->quoteSmart($from_url.'%');

			if ($tablename == 'sq_ast_lookup') {
				$sql .= ' AND root_urlid = '.$db->quoteSmart($from_urlid);
			} else if ($tablename == 'sq_ast_lookup_value') {
				$sql .= 'AND url IN (
							SELECT
								l.url
							FROM
								sq_ast_lookup l
							INNER JOIN
								sq_ast_lookup_value v ON l.url = v.url
							WHERE
								l.root_urlid = '.$db->quoteSmart($from_urlid).'
						)';
			}

	$result = $db->query($sql);
	assert_valid_db_result($result);
}

bam('LOOKUPS CHANGED FROM '.$from_url.' TO '.$to_url);

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

/**
* Prints the specified prompt message and returns the line from stdin
*
* @param string	$prompt	the message to display to the user
*
* @return string
* @access public
*/
function get_line($prompt='')
{
	echo $prompt;
	// now get their entry and remove the trailing new line
	return rtrim(fgets(STDIN, 4096));

}//end get_line()

?>
