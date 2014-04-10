<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: replace_url.php,v 1.10 2013/04/10 07:31:54 cupreti Exp $
*
*/

/**
* Remaps a url to another url. Use this script instead of updating lookups as it should be
* quicker
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed logging in as root user\n", E_USER_ERROR);
}

echo "\n";

$db_type = MatrixDAL::getDbType();

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

$sql = 'SELECT urlid, url, assetid FROM sq_ast_url';
$root_urls = MatrixDAL::executeSqlGroupedAssoc($sql);
ksort($root_urls);

foreach ($root_urls as $urlid => $url) {
	echo $urlid.'. '.$root_urls[$urlid][0]['url']."\n";
}

echo "\n";

$chosen_url = -1;

while (!in_array($chosen_url, array_keys($root_urls))) {
	$chosen_url = get_line('Please enter the urlid of the url to change: ');
	if (!in_array($chosen_url, array_keys($root_urls))) {
		echo 'Invalid urlid!'."\n";
	}
}

$from_url   = $root_urls[$chosen_url][0]['url'];
$from_urlid = $chosen_url;
$from_site_assetid = $root_urls[$chosen_url][0]['assetid'];
$to_url     = get_line('Please enter the url to change to: ');

$root_ok = FALSE;
while (!$root_ok) {
	$system_root_urls = explode("\n", trim(SQ_CONF_SYSTEM_ROOT_URLS));
	$matching_from_roots = Array();
	$matching_roots = Array();

	// We are matching system root URLs for two reasons:
	// - Finding URLs that __data paths need to be changed to, and
	// - Alerting the user that their new URL does not match a system root URL
	//   (which is a problem and should be stopped before the query runs)
	foreach ($system_root_urls as $root_url) {
		if (substr($to_url.'/', 0, strlen($root_url) + 1) == $root_url.'/') {
			$matching_roots[] = $root_url;
		}

		if (substr($from_url.'/', 0, strlen($root_url) + 1) == $root_url.'/') {
			$matching_from_roots[] = $root_url;			
		}
	}

	if (empty($matching_roots)) {
		echo 'The new URL entered, "'.$to_url.'", is not based upon an existing System Root URL.'."\n";
		$to_url = get_line('Please re-enter the new URL: ');
	} else {
		$root_ok = TRUE;
	}
}//end while root not OK

$confirm = null;
while ($confirm != 'y' && $confirm != 'n') {
	$confirm = strtolower(get_line('Change '.$from_url .' to '.$to_url.' (y/n)? : '));
	if ($confirm != 'y' && $confirm != 'n') {
		echo 'Please answer y or n'."\n";
	} else if ($confirm == 'n') {
		exit();
	}
}

// update any urls that use this url in the lookup and lookup value tables
foreach (Array('sq_ast_lookup_value', 'sq_ast_lookup') as $tablename) {

	$sql = 'UPDATE
				'.$tablename.'
			SET
				url = :to_url || SUBSTR(url, :from_url_length + 1)
			WHERE
				url LIKE :from_url_wildcard';

			if ($tablename == 'sq_ast_lookup') {
				$sql .= ' AND root_urlid = :from_urlid';
			} else if ($tablename == 'sq_ast_lookup_value') {
				$sql .= ' AND url IN (
							SELECT
								l.url
							FROM
								sq_ast_lookup l
							INNER JOIN
								sq_ast_lookup_value v ON ((l.url = v.url) OR (l.url || \'/\' = v.url))
							WHERE
								l.root_urlid = :from_urlid
						)';
			}

	$query = MatrixDAL::preparePdoQuery($sql);
	MatrixDAL::bindValueToPdo($query, 'to_url',            $to_url);
	MatrixDAL::bindValueToPdo($query, 'from_url_wildcard', $from_url.'%');
	MatrixDAL::bindValueToPdo($query, 'from_url_length',   strlen($from_url));
	MatrixDAL::bindValueToPdo($query, 'from_urlid',        $from_urlid);
	MatrixDAL::execPdoQuery($query);

}//end foreach table


// update the root url in the asset url table to the new url
$sql = 'UPDATE
			sq_ast_url
		SET
			url = :url
		WHERE
			urlid = :urlid';

$query = MatrixDAL::preparePdoQuery($sql);
MatrixDAL::bindValueToPdo($query, 'url',   $to_url);
MatrixDAL::bindValueToPdo($query, 'urlid', $from_urlid);
MatrixDAL::execPdoQuery($query);

// If there is a static root URL, there is nothing to change for __data URLs.
// Otherwise, try and update them to the closest root URL we can find
if (trim(SQ_CONF_STATIC_ROOT_URL) == '') {
	// First, work out the roots that have changed - if some have not changed
	// there is no point to be processing them
	$common_roots = array_intersect($matching_roots, $matching_from_roots);
	$matching_roots = array_diff($matching_roots, $common_roots);
	$matching_from_roots = array_diff($matching_from_roots, $common_roots);
	$file_children = $GLOBALS['SQ_SYSTEM']->am->getChildren($from_site_assetid, 'file', FALSE);
	
	$limit_clause = (strpos($db_type, 'oci') !== FALSE) ? 'AND ROWNUM = 1' : 'LIMIT 1';
	
	// Update the roots up to the number that we can update
	for ($x = 0; $x < min(count($matching_roots), count($matching_from_roots)); $x++) {
		// Change the lookup values first
		$sql = 'UPDATE sq_ast_lookup_value SET url = :to_url || SUBSTR(url, :from_url_length + 1) WHERE url IN (SELECT url FROM sq_ast_lookup WHERE assetid IN (SELECT minorid FROM sq_ast_lnk WHERE linkid IN (SELECT linkid FROM sq_ast_lnk_tree t1 WHERE treeid LIKE (SELECT treeid || \'_%\' FROM sq_ast_lnk_tree t2 WHERE linkid IN (SELECT linkid FROM sq_ast_lnk WHERE minorid = :site_assetid) '.$limit_clause.')))
					 AND url LIKE :from_url || \'/__data/%\')';
		
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'site_assetid', $from_site_assetid);
		MatrixDAL::bindValueToPdo($query, 'from_url', $matching_from_roots[$x]);
		MatrixDAL::bindValueToPdo($query, 'from_url_length', strlen($matching_from_roots[$x]));
		MatrixDAL::bindValueToPdo($query, 'to_url', $matching_roots[$x]);
		MatrixDAL::execPdoQuery($query);
			
		$sql = 'UPDATE sq_ast_lookup SET url = :to_url || SUBSTR(url, :from_url_length + 1) WHERE assetid IN (SELECT minorid FROM sq_ast_lnk WHERE linkid IN (SELECT linkid FROM sq_ast_lnk_tree t1 WHERE treeid LIKE (SELECT treeid || \'_%\' FROM sq_ast_lnk_tree t2 WHERE linkid IN (SELECT linkid FROM sq_ast_lnk WHERE minorid = :site_assetid) '.$limit_clause.')))
					AND url LIKE :from_url || \'/__data/%\'';
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'site_assetid', $from_site_assetid);
		MatrixDAL::bindValueToPdo($query, 'from_url', $matching_from_roots[$x]);
		MatrixDAL::bindValueToPdo($query, 'from_url_length', strlen($matching_from_roots[$x]));
		MatrixDAL::bindValueToPdo($query, 'to_url', $matching_roots[$x]);
		MatrixDAL::execPdoQuery($query);
	}

	// We have new URLs
	// Going to pass on adding new lookups to this situation, because lookups are usually meant
	// to be URL-based. How can we tell whether a lookup value is meant to be per-asset or per-URL?
	for (; $x < count($matching_roots); $x++) {
		$sql = 'INSERT INTO sq_ast_lookup (url, root_urlid, http, https, assetid) SELECT DISTINCT :to_url || SUBSTR(url, STRPOS(url, \'/__data/\')), 0, http, https, assetid FROM sq_ast_lookup WHERE url LIKE \'%/__data/%\' AND assetid IN (SELECT minorid FROM sq_ast_lnk WHERE linkid IN (SELECT linkid FROM sq_ast_lnk_tree t1 WHERE treeid LIKE (SELECT treeid || \'_%\' FROM sq_ast_lnk_tree t2 WHERE linkid IN (SELECT linkid FROM sq_ast_lnk WHERE minorid = :site_assetid) '.$limit_clause.')))';
		if (MatrixDAL::getDbType() == 'oci') {
			// String position function is called INSTR() in Oracle
			$sql = str_replace('STRPOS(', 'INSTR(', $sql);
		}
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'site_assetid', $from_site_assetid);
		MatrixDAL::bindValueToPdo($query, 'to_url', $matching_roots[$x]);
		MatrixDAL::execPdoQuery($query);
	}//end for - remaining to URLs
		
	// More URLs beforehand than what we have now = have to delete the rest
	for (; $x < count($matching_from_roots); $x++) {
		// Delete the lookup values first
		$sql = 'DELETE FROM sq_ast_lookup_value WHERE url IN (SELECT url FROM sq_ast_lookup WHERE assetid IN (SELECT minorid FROM sq_ast_lnk WHERE linkid IN (SELECT linkid FROM sq_ast_lnk_tree t1 WHERE treeid LIKE (SELECT treeid || \'_%\' FROM sq_ast_lnk_tree t2 WHERE linkid IN (SELECT linkid FROM sq_ast_lnk WHERE minorid = :site_assetid) '.$limit_clause.')))
					 AND url LIKE :from_url || \'/__data/%\')';
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'site_assetid', $from_site_assetid);
		MatrixDAL::bindValueToPdo($query, 'from_url', $matching_from_roots[$x]);
		MatrixDAL::execPdoQuery($query);
		
		$sql = 'DELETE FROM sq_ast_lookup WHERE assetid IN (SELECT minorid FROM sq_ast_lnk WHERE linkid IN (SELECT linkid FROM sq_ast_lnk_tree t1 WHERE treeid LIKE (SELECT treeid || \'_%\' FROM sq_ast_lnk_tree t2 WHERE linkid IN (SELECT linkid FROM sq_ast_lnk WHERE minorid = :site_assetid) '.$limit_clause.')))
					AND url LIKE :from_url || \'/__data/%\'';
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'site_assetid', $from_site_assetid);
		MatrixDAL::bindValueToPdo($query, 'from_url', $matching_from_roots[$x]);
		MatrixDAL::execPdoQuery($query);
	}//end for - remaining from URLs

}//end if - no static root set

pre_echo('LOOKUPS CHANGED FROM '.$from_url.' TO '.$to_url);

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
