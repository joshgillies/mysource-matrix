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
*/

/**
* Add or Remove a url from a site/asset. This script will go and update the
* sq_ast_url, sq_ast_lookup, sq_ast_lookup_value table. It assume that the site being edited is already have
* a URL applied to it.
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/


error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
    trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

// check that the correct root password was entered
$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
    echo "ERROR: The root password entered was incorrect\n";
    exit();
}

echo "\n";

	$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');

	$db =& $GLOBALS['SQ_SYSTEM']->db;


$action = NULL;
while ($action != 'add' && $action != 'remove' && $action != 'update') {
	$action = get_line('Please specify whether you want to \'add\' or \'remove\' a URL: ');
}

// For ADDING
// Need to take in: -action=Add -http, -https, -assetid, -existing_url, -new_url


if ($action == 'add') {
		bam("To use this script, you would need the following information\n
Site Assetid
Protocol to be used
New URL
An existing URL of the site\n
Make sure you have all you information you need before Proceeding\n");
	
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$http = -1;
		while ($http != '0' && $http != '1') {
			$http 	= get_line('Please specify whether this URL use HTTP protocol or not (1 or 0): ');
		}

		$https = -1;
		while ($https != '0' && $https != '1') {
			$https	= get_line('Please specify whether this URL use HTTPS protocol or not (1 or 0): ');
		}

		if ($http != 1 && $https != 1) {
			echo "Please select either HTTP or HTTPS as protocol\n";
			exit(0);
		}

		$assetid = NULL;
		while (is_null($assetid)) {
			$assetid = get_line('Enter the Site Assetid to apply the URL to: ');
			assert_valid_assetid($assetid);
			$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if ($asset->type() != 'site') {
				echo "Asset must be a Site\n";
				$assetid = NULL;
			}
		}

		// Now we have to check whether we should add this URL to this site or not.
		require_once $SYSTEM_ROOT.'/data/private/conf/main.inc';

		$update_file_public_live_assets = FALSE;
		$new_url = NULL;
		$root_urls = Array();
		while (is_null($new_url)) {
			$new_url = get_line('Enter the New URL:');
			// Get the main.inc and then get the SQ_CONF_SYSTEM_ROOT_URLS
			// Check if the new URL is in the SQ_CONF_SYSTEM_ROOT_URLS
			$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
			$in_root_urls = FALSE;
			$root_urls_string = '';
			foreach ($root_urls as $root_url) {
				$root_urls_string .= $root_url."\n";
				if (strpos($new_url, $root_url) === 0) {
					$in_root_urls = TRUE;
					// cut trailing slash if there's any
					// only if new url is the same as one of the root url, or new url belongs to one of the root url.
					if (($new_url == $root_url) || (strpos($new_url, $root_url) === 0 && strlen($new_url) > strlen($root_url))){
						$update_file_public_live_assets = TRUE;
					}//end if
				}//end if

			}//end foreach
			if (!$in_root_urls) {
				echo "The provided URL is not based upon an existing System Root URL\n";
				echo "Existing System Root URLs are : \n".$root_urls_string;
				$new_url = NULL;
			}

			$sql_check_new_url = 'SELECT url FROM sq_ast_url WHERE url LIKE '.$db->quoteSmart($new_url).';';
			$new_url_check = $db->getOne($sql_check_new_url);
			assert_valid_db_result($new_url_check);
			if (!empty($new_url_check)) {
				echo "The new URL : ".$new_url." is already exist.\n";
				$new_url = NULL;
			}
		}

		$existing_url = NULL;
		while (is_null($existing_url)) {
			$existing_url = get_line('Enter the existing URL: ');
		}

		if ($new_url == $existing_url) {
			echo "Does not allow to alter existing url\n";
			exit(0);
		}

		$site_urls = Array();
		$urls = $asset->getURLs();
		foreach ($urls as $url_info) {
			$site_urls[] = $url_info['url'];
		}	
		if (!in_array($existing_url, $site_urls)) {
			echo "The existing URL does not belong to the site with id: $assetid \n";
			exit(0);
		}		
	
		$sql_check_existing_url = 'SELECT urlid FROM sq_ast_url WHERE url LIKE '.$db->quoteSmart($existing_url).';';
		$existing_urlid = $db->getOne($sql_check_existing_url);
		assert_valid_db_result($existing_urlid);
		if (empty($existing_urlid)) {
			echo "The existing URL : ".$existing_url." does not exist\n";
			exit(0);
		}


		// Lets check if the system is using static URL, if they use static URL, there's no point updating FILE PUBLIC LIVE assets.
		$static_root_url = SQ_CONF_STATIC_ROOT_URL;
		if (!empty($static_root_url)) {
			$update_file_public_live_assets = FALSE;
		}

		// First we check that the new URL being added is not under any system root URLs, if it is, we don't need to update this FILE PUBLIC LIVE assets.
		if ($update_file_public_live_assets) {

			// Before we do any of the processing, lets grab all the FILE assets that are LIVE, and have PUBLIC READ ACCESS.
			$asset_types_list = array_keys($GLOBALS['SQ_SYSTEM']->am->getAssetTypeHierarchy('file'));
			$asset_types_list[] = 'file';
			$children = Array();
			$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($assetid, $asset_types_list, TRUE);

			$children = array_keys($children);	// We just need the asset id
			$public_user_id = 7;
			$children_to_update = Array();
			foreach ($children as $child_id) {
				$child_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($child_id);
				if ($child_asset->usePublicPath()) {
						$children_to_update[] = $db->quoteSmart((string) $child_id);
				}//end if
				// Else just ignore this asset
			}//end foreach
		}

		$new_urlid = $db->nextId('sq_ast_url');
		assert_valid_db_result($new_urlid);

		$sql_update_sq_ast_url = 'INSERT INTO sq_ast_url (http, https, assetid, url, urlid) values ('
		.$db->quoteSmart($http).','
		.$db->quoteSmart($https).','
		.$db->quoteSmart($assetid).','
		.$db->quoteSmart($new_url).','
		.$db->quoteSmart($new_urlid)
		.');';

		$sql_update_sq_ast_lookup_value = 'INSERT INTO sq_ast_lookup_value (url, name, value, inhd)
										SELECT replace(url, '.$db->quoteSmart($existing_url).', '.$db->quoteSmart($new_url).'), name, value, inhd
											FROM sq_ast_lookup_value
											WHERE url in (SELECT url FROM sq_ast_lookup WHERE root_urlid = '.$db->quoteSmart($existing_urlid).');';

		$sql_update_sq_ast_lookup		= 'INSERT INTO sq_ast_lookup (http, https, assetid, url, root_urlid)
										SELECT http, https, assetid, replace(url,'.$db->quoteSmart($existing_url).','.$db->quoteSmart($new_url).'),'.$new_urlid.'
										FROM sq_ast_lookup WHERE url like '.$db->quoteSmart($existing_url.'%').' AND root_urlid ='.$db->quoteSmart($existing_urlid).';';

		$sql_update_sq_ast_lookup_public	= 'UPDATE sq_ast_lookup set root_urlid = 0 WHERE url like '.$db->quoteSmart($new_url.'%').' AND url like \'%/__data/%\' AND root_urlid = '.$db->quoteSmart($new_urlid);

		// Now we run the query
		// 1. Add entry to sq_ast_url
		$result_ast_url			= $db->query($sql_update_sq_ast_url);
		assert_valid_db_result($result_ast_url);

		// 2. Add entries to sq_ast_lookup_value
		$result_lookup_value 	= $db->query($sql_update_sq_ast_lookup_value);
		assert_valid_db_result($result_lookup_value);

		// 3, Add entries to sq_ast_lookup
		$result_lookup 			= $db->query($sql_update_sq_ast_lookup);
		assert_valid_db_result($result_lookup);
		$result_lookup_public	= $db->query($sql_update_sq_ast_lookup_public);
		assert_valid_db_result($result_lookup_public);

		// We have done updating regular asset, now we will update the publically served file assets.

		if ($update_file_public_live_assets) {
			// Now we have to chop out the system root Url from the "existing" Url. 
			$absolute_root = '';
			$relative_root = '';
			foreach ($root_urls	as $url) {
				if (strpos($existing_url, $url) !== FALSE) {
					$relative_root = $url;
				}
				if ($existing_url == $url) {
					$absolute_root = $url; 
					break;
				}
			}
			$existing_url_public = (empty($absolute_root)) ? $relative_root : $absolute_root;

			// Now we have to chop out the system root Url from the "new" Url.
			$absolute_new_root = '';
			$relative_new_root = '';
			foreach ($root_urls	as $url) {
				if (strpos($new_url, $url) !== FALSE) {
					$relative_new_root = $url;
				}
				if ($new_url == $url) {
					$absolute_new_root = $url; 
					break;
				}
			}	
			$new_url_public = (empty($absolute_new_root)) ? $relative_new_root : $absolute_new_root;

			// Do we have any file need to be updated?
			if (!empty($children_to_update)) {
				$in_clauses = Array();
				foreach (array_chunk($children_to_update, 999) as $chunk) {
					$in_clauses[] = ' assetid IN ('.implode(', ', $chunk).')';
				}
				$where = '('.implode(' OR ', $in_clauses).')';

				$sql_update_sq_ast_lookup_public_file = 'INSERT INTO sq_ast_lookup (http, https, assetid, url, root_urlid)
															SELECT http, https, assetid, replace(url, '.$db->quoteSmart($existing_url_public).','.$db->quoteSmart($new_url_public).'), 0
															FROM sq_ast_lookup WHERE root_urlid = 0 AND '. $where;
				bam($sql_update_sq_ast_lookup_public_file);
				$result_update_sq_ast_lookup_public_file = $db->query($sql_update_sq_ast_lookup_public_file);
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

} else if ($action == 'remove') {
		// For Removing, need to take in
		// -removeurl
		bam("!Important: Please make sure that you know how to use this script, and remember to back up the database before proceeding\n
(Press Ctrl+C to terminate the script)\n");

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$remove_url = NULL;
		while (is_null($remove_url)) {
			$remove_url = get_line('Enter the URL to be removed: ');
		}
		$sql_check_remove_url = 'SELECT urlid, assetid FROM sq_ast_url WHERE url LIKE '.$db->quoteSmart($remove_url).';';
		$remove_info = $db->getAssoc($sql_check_remove_url);
		$remove_urlid = key($remove_info);
		$remove_assetid = $remove_info[$remove_urlid];

		assert_valid_db_result($remove_urlid);
		if (empty($remove_urlid)) {
			echo 'The provided URL : '.$remove_url.' does not exist';
			exit(0);
		}
	
		// Before we do any of the processing, lets grab all the FILE assets that are LIVE, and have PUBLIC READ ACCESS.
		$asset_types_list = array_keys($GLOBALS['SQ_SYSTEM']->am->getAssetTypeHierarchy('file'));
		$asset_types_list[] = 'file';
		$children = Array();
		$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($remove_assetid, $asset_types_list, TRUE);

		$children = array_keys($children);	// We just need the asset id
		$public_user_id = 7;
		$children_to_update = Array();
		foreach ($children as $child_id) {
			$child_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($child_id);
			if ($child_asset->usePublicPath()) {
					$children_to_update[] = $db->quoteSmart((string) $child_id);
			}//end if
			// Else just ignore this asset
		}//end foreach
	

		if (!empty($children_to_update)) {
			$in_clauses = Array();
			foreach (array_chunk($children_to_update, 999) as $chunk) {
				$in_clauses[] = ' assetid IN ('.implode(', ', $chunk).')';
			}
			$where = '('.implode(' OR ', $in_clauses).')';

			require_once $SYSTEM_ROOT.'/data/private/conf/main.inc';
			$root_urls = Array();
			$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);

			// Now we have to chop out the system root Url from the "existing" Url. 
			$absolute_root_remove = '';
			$relative_root_remove = '';
			foreach ($root_urls	as $url) {
				if (strpos($remove_url, $url) !== FALSE) {
					$relative_root_remove = $url;
				}
				if ($remove_url == $url) {
					$absolute_root_remove = $url; 
					break;
				}
			}

			$remove_url_public = (empty($absolute_root_remove)) ? $relative_root_remove : $absolute_root_remove;

			$sql_update_sq_ast_lookup_public	= 'DELETE FROM sq_ast_lookup WHERE root_urlid = 0 AND url LIKE '.$db->quoteSmart($remove_url_public.'%').' AND url LIKE \'%/__data/%\'';
			$sql_update_sq_ast_lookup_public	.= ' AND '.$where;
		}


		// Find all other root_urlid that look like $remove_url.'%', we don't want to remove those.
		$sql_get_sub_url_root_urlid = 'SELECT url FROM sq_ast_url WHERE
											url NOT LIKE '.$db->quoteSmart($remove_url).' AND
											url LIKE '.$db->quoteSmart($remove_url.'%').';';

		$avoid_urls	= $db->getAll($sql_get_sub_url_root_urlid);
		assert_valid_db_result($avoid_urls);

		$sql_update_sq_ast_lookup_value		= 'DELETE FROM sq_ast_lookup_value WHERE url like '.$db->quoteSmart($remove_url.'%');
		foreach ($avoid_urls as $index => $data) {
			$sql_update_sq_ast_lookup_value .= ' AND url NOT LIKE ' . $db->quoteSmart($data['url'].'%');
		}

		$sql_update_sq_ast_lookup			= 'DELETE FROM sq_ast_lookup WHERE root_urlid = '.$db->quoteSmart($remove_urlid).' AND url like '.$db->quoteSmart($remove_url.'%').';';


		$sql_update_sq_ast_url				= 'DELETE FROM sq_ast_url WHERE urlid = '.$db->quoteSmart($remove_urlid).' AND url like '.$db->quoteSmart($remove_url).';';

		foreach ($avoid_urls as $index => $data) {
			$sql_update_sq_ast_lookup_public	.= ' AND url NOT LIKE ' . $db->quoteSmart($data['url'].'%');
			$sql_update_sq_ast_lookup			.= ' AND url NOT LIKE ' . $db->quoteSmart($data['url'].'%');
		}

		// We run the query in different order
		// 1. Remove entries in sq_ast_lookup_value
		$result_lookup_value	= $db->query($sql_update_sq_ast_lookup_value);
		assert_valid_db_result($result_lookup_value);

		// 2. Remove entries in sq_ast_lookup
		if (!empty($children_to_update)) {
			$result_lookup_public	= $db->query($sql_update_sq_ast_lookup_public);
			assert_valid_db_result($result_lookup_public);
		}
		$result_lookup			= $db->query($sql_update_sq_ast_lookup);
		assert_valid_db_result($result_lookup);

		// 3. Remove the entry in sq_ast_url
		$result_ast_url			= $db->query($sql_update_sq_ast_url);
		assert_valid_db_result($result_ast_url);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

}//end else if

		bam($sql_update_sq_ast_url);
		bam($sql_update_sq_ast_lookup_value);
		bam($sql_update_sq_ast_lookup);
		bam($sql_update_sq_ast_lookup_public);

	$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	exit(0);

/**
* Prints the specified prompt message and returns the line from stdin
*
* @param string $prompt the message to display to the user
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
