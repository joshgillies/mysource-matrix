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
 */

/**
* Script to replace the string the webpaths system wide.
* 
* The script will update webpath/urls in following tables:
* 	sq_ast_path
*	sq_ast_lookup
*	sq_ast_lookup_value
*	sq_ast_lookup_remap (either insert new remaps or update existing one)
*	sq_ast_url (with '--include-site-urls' option)
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix
*/

error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = getCLIArg('system');
if (!$SYSTEM_ROOT) {
	echo "ERROR: You need to supply the path to the System Root.\n";
	print_usage();
	exit(1);
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	print_usage();
	exit(1);
}


$replace_from = getCLIArg('search');
if (!$replace_from) {
	if ($replace_from !== FALSE) {
		$replace_from = preg_replace('/[^a-zA-Z0-9\-$_@.!*~(),]/', '',  $replace_from);
	}
	echo  "\nERROR: ".($replace_from === FALSE ? "Enter the string to search" : "The string to search must be non-empty").".\n\n";
	print_usage();
	exit(1);
}

$replace_to = getCLIArg('replace');
if ($replace_to === FALSE) {
	echo  "\nERROR: Enter the string to replace.\n\n";
	print_usage();
	exit(1);
}
$replace_to = preg_replace('/[^a-zA-Z0-9\-$_@.!*~(),]/', '',  $replace_to);

if ($replace_from == $replace_to) {
	echo "Search and replace string are same: '".$replace_from."'. Nothing to update.\n\n";
	exit();
}


// Whether to replace site/root URLs
$include_site_urls = getCLIArg('include-site-urls');

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');
require_once $SYSTEM_ROOT.'/core/include/init.inc';

echo "The search string '".$replace_from."' will be replaced by '".$replace_to."' in the webpaths system wide.\n\n";

if (empty($replace_to)) {
	echo "WARNING: The 'replace' string is empty. This means all the 'search' string will be removed in the webpaths, which can result into webpath conflits.\n";
	echo "For example with 'search' => '0' and 'replace' => '' can result into following:\n";
	echo "'www.test.com/1001' to www.test.com/11'\n";
	echo "'www.test.com/1100' to www.test.com/11'\n\n";
}
echo "It highly recommended to backup the system before running this script.\n";
echo "Are you sure you want to proceed (Y/n)? \n";
$yes_no = rtrim(fgets(STDIN, 4094));
if ($yes_no != 'Y') {
	echo "\nScript aborted. \n";
	exit;
}

// Get the root urls
$ROOT_URLS = Array();
foreach(explode("\n", SQ_CONF_SYSTEM_ROOT_URLS) as $root_url) {
	$ROOT_URLS[] = trim($root_url, '/ ');
}//end foreach

// Sort the root URLs by length
uasort($ROOT_URLS, function ($a, $b) {
					return strlen($a) < strlen($b);
		}
	);

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

$progress_count = 0;
$ERRORS = Array();
$summary = Array(
				'ast_url' => 0,
				'ast_path' => 0,
				'ast_lookup' => 0,
				'ast_lookup_value' => 0,
				'ast_lookup_remap' => 0
		);

// Get all the site URLs
$site_urls = Array();
$sql = 'SELECT urlid, url FROM sq_ast_url';
// This script will not touch static URLs
if (SQ_CONF_STATIC_ROOT_URL) {
	$sql = ' WHERE url <> '.MatrixDAL::quote(SQ_CONF_STATIC_ROOT_URL).' OR SUBSTR(url, \'0\', '.MatrixDAL::quote(strlen(SQ_CONF_STATIC_ROOT_URL)+1).') <> '.MatrixDAL::quote(SQ_CONF_STATIC_ROOT_URL.'/');
}

$result = MatrixDAL::executeSqlAssoc($sql);
foreach($result as $row) {
	$old_url = $new_url = $row['url'];

	if ($include_site_urls) {
		// Get the closest root URL to this url
		$root_url = _getRootUrl($old_url);
		if (!$root_url) {
			// Url is useless if it does not belongs to any root url
			continue;
		}

		$new_url = $root_url.str_replace($replace_from, $replace_to, substr($old_url, strlen($root_url)));
		if (_invalidReplacement($new_url, $replace_to)) {
			// This means replacement completely removed a 'webpath level'
			$new_url = $old_url;
		}
	}
	$site_urls[$row['urlid']] = Array(
									'old' => $old_url,
									'new' => $new_url,
								);

	if ($include_site_urls && $new_url != $old_url) {
		$sql = 'UPDATE sq_ast_url SET url = :new_url WHERE urlid = :urlid';
		$records_count = _executeSql($sql, Array('new_url' => $new_url, 'urlid' =>$row['urlid']));
		$summary['ast_url'] += ($records_count ? $records_count : 0);
		if ($records_count === FALSE) {
			break;
		}
	}

	$progress_count++;
	if ($progress_count % 10 == 0) {
		echo '.';
	}
}//end foreach

// Update the the asset lookups
$sql = 'SELECT root_urlid, url, http, https, assetid FROM sq_ast_lookup WHERE REPLACE(url, '.MatrixDAL::quote($replace_from).', \'\') <> url';
if (SQ_CONF_STATIC_ROOT_URL) {
	// This script will not touch static URLs
	$sql = ' AND (url <> '.MatrixDAL::quote(SQ_CONF_STATIC_ROOT_URL).' OR SUBSTR(url, \'0\', '.MatrixDAL::quote(strlen(SQ_CONF_STATIC_ROOT_URL)+1).') <> '.MatrixDAL::quote(SQ_CONF_STATIC_ROOT_URL.'/');
}
$result = MatrixDAL::executeSqlGrouped($sql);

foreach($result as $urlid => $rows) {

	if (!$urlid || !isset($site_urls[$urlid])) {
		continue;
	}

	$base_url_old = rtrim($site_urls[$urlid]['old'], '/');
	$base_url_new = rtrim($site_urls[$urlid]['new'], '/');

	foreach($rows as $row) {
		$progress_count++;
		if ($progress_count % 100 == 0) {
			echo '.';
		}

		$url = $row[0];
		$http = $row[1];
		$https = $row[2];
		$assetid = $row[3];

		if ($data_url_pos = strpos($url, '/__data/')) {
			// We replace the root url bit only in __data file URL. Replacing the unrestricted file name is beyond the script's scope.
			if ($base_url_old != $base_url_new) {
				$updated_url = $base_url_new. substr($url, $data_url_pos);
			} else {
				continue;
			}
		} else if (SQ_CONF_STATIC_ROOT_URL && (SQ_CONF_STATIC_ROOT_URL == $url || strpos($url, SQ_CONF_STATIC_ROOT_URL.'/') === 0)) {
			// We dont touch static URL lookups
			continue;
		} else {
			// Regular URL
			$updated_url = $base_url_new.str_replace($replace_from, $replace_to, substr($url, strlen($base_url_old)));
		}

		if ($updated_url == $url || _invalidReplacement($updated_url, $replace_to)) {
			continue;
		}

		// Update ast_lookup table
		$sql = 'UPDATE sq_ast_lookup SET url = :updated_url WHERE url = :url';
		$records_count = _executeSql($sql, Array('updated_url' => $updated_url, 'url' => $url));
		$summary['ast_lookup'] += ($records_count ? $records_count : 0);
		if ($records_count === FALSE) {
			break 2;
		}

		// Update ast_path table
		$old_path = basename($url);
		$new_path = basename($updated_url);
		if ($old_path != $new_path) {
			$sql = 'UPDATE sq_ast_path SET path = :new_path WHERE path = :old_path AND assetid = :assetid';
			$records_count = _executeSql($sql, Array('new_path' => $new_path, 'old_path' => $old_path, 'assetid' => $assetid));
			$summary['ast_path'] += ($records_count ? $records_count : 0);
			if ($records_count === FALSE) {
				break 2;
			}
		}

		// Add the remap entry for the url change
		if (!$data_url_pos) {
			foreach(Array('http', 'https') as $protocol) {
				if (($protocol == 'http' && !$http) || ($protocol == 'https' && !$https)) {
					continue;
				}

				// If the URL is already remapped, update it
				$sql = 'SELECT url FROM sq_ast_lookup_remap WHERE url = '.MatrixDAL::quote($protocol.'://'.$url);
				if (MatrixDAL::executeSqlAssoc($sql, 'url')) {
					$sql = 'UPDATE sq_ast_lookup_remap SET remap_url = :remap_url, auto_remap = :auto_remap WHERE url = :url';
				} else {
					$sql = 'INSERT INTO sq_ast_lookup_remap(url, remap_url, auto_remap) VALUES (:url, :remap_url, :auto_remap)';
				}
				$bind_vars = Array(
								'url' => $protocol.'://'.$url,
								'remap_url' => $protocol.'://'.$updated_url,
								'auto_remap' => '1',
							);
				$records_count = _executeSql($sql, $bind_vars);
				$summary['ast_lookup_remap'] += ($records_count ? $records_count : 0);
				if ($records_count === FALSE) {
					break 2;
				}
			}//end foreach protocol
		}//end if non-unrestricted url

		// Update ast_lookup_value table
		$sql = 'UPDATE sq_ast_lookup_value SET url = :updated_url WHERE url = :url';
		$records_count = _executeSql($sql, Array('updated_url' => $updated_url, 'url' =>$url));
		$summary['ast_lookup_value'] += ($records_count ? $records_count : 0);
		if ($records_count === FALSE) {
			break 2;
		}

	}//end foreach
}//end foreach

// We will update the db only if there are no errors
if ($ERRORS) {
	$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');

	echo "\nFollowing errors occured when trying to update the lookups:\n";
	echo implode("\n", $ERRORS)."\n\n";
	echo "Please resolve these errors before running this script. DB was not updated.\n";
} else {
	$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

	echo "\nSummary of records updated\n";
	echo "Table sq_ast_url:          ".$summary['ast_url']." records updated\n";
	echo "Table sq_ast_path:         ".$summary['ast_path']." records updated\n";
	echo "Table sq_ast_lookup:       ".$summary['ast_lookup']." records updated\n";
	echo "Table sq_ast_lookup_value: ".$summary['ast_lookup_value']." records updated\n";
	echo "Table sq_ast_lookup_remap: ".$summary['ast_lookup_remap']." records updated\n";
}

$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

// End of the main progam //////


/**
* Execute the write db query
*
* @param string $sql
*
* @return boolean|int
*/
function _executeSql($sql, $bind_vars)
{
	try {
		$query = MatrixDAL::preparePdoQuery($sql);
		foreach($bind_vars as $bind_var => $bind_value) {
			MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
		}
		$count = MatrixDAL::execPdoQuery($query);
	} catch (Exception $e) {
		global $ERRORS;
		$error = 'DB Exception '.$e->getMessage()."\n"."SQL: ".$sql;
		foreach($bind_vars as $key => $val) {
			$error = str_replace(':'.$key, MatrixDAL::quote($val), $error);
		}
		$ERRORS[] = $error;

		return FALSE;
	}

	return $count;

}//end _executeSql()


/**
* Returns TRUE the empty replacement completely removes a "webpath level"
*
* @param string $url
* @param string $replace_to
*
* @return boolean
*/
function _invalidReplacement($url, $replace_to)
{
	$url = trim($url);

	return empty($replace_to) && (substr($url, '-1') == '/' || strpos($url, '//') !== FALSE);

}//end _invalidReplacement()


/**
* Check the closest matching system root URL for given site URL
*
* @params string $url
*
* @return string|boolean
*/
function _getRootUrl($url)
{
	// $ROOT_URLS has order root URLs by the URL length, so longest root URL is matched first
	global $ROOT_URLS;
	foreach($ROOT_URLS as $root_url) {
		if ($root_url == $url || strpos($url, $root_url.'/') !== FALSE) {
			return $root_url;
		}
	}

	return FALSE;

}//end _getRootUrl()


/**
* Get CLI Argument
* Check to see if the argument is set, if it has a value, return the value
* otherwise return true if set, or false if not
*
* @params $arg string argument
*
* @return string/boolean
*/
function getCLIArg($arg)
{
	return (count($match = array_values(preg_grep("/--" . $arg . "(\=(.*)|)/i",$_SERVER['argv']))) > 0 === TRUE) ? ((preg_match('/--(.*)=(.*)/',$match[0],$reg)) ? $reg[2] : TRUE) : FALSE;

}//end getCLIArg()


/**
 * Print the usage of this script
 *
 * @return void
 */
function print_usage()
{
	echo "\nThis search and replaces the given string in the webpath system wide.";

	echo "Usage: php ".basename(__FILE__)." --system=<SYSTEM_ROOT> --search=<SEARCH_STRING> --replace=<REPLACE_STRING> [--include-site-urls]\n\n";
	echo "\t<SYSTEM_ROOT>        : The root directory of Matrix system.\n";
	echo "\t<SEARCH_STRING>      : String to search in webpath. Allowed chars [a-zA-Z0-9\-$_@.!*~(),].\n";
	echo "\t<REPLACE_STRING>     : String to replace in webpath. Allowed chars [a-zA-Z0-9\-$_@.!*~(),], including empty string..\n";
	echo "\t[--include-site-urls]: Whether to include site URLs\n";
	echo "\n";

}//end print_usage()

?>
