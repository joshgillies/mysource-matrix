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
* Add or Remove a url from a site/asset. This script will go and update the
* sq_ast_url, sq_ast_lookup, sq_ast_lookup_value table. It assume that the site being edited is already have
* a URL applied to it.
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix
*/


class URL_Manager
{

	//static $db = $GLOBALS['SQ_SYSTEM']->db;
	function URL_Manager()
	{

	}//end constructor


	/*
	* Interface to add URL
	*
	*/
	public static function cliInterfaceAddUrl()
	{
		global $SYSTEM_ROOT;
		pre_echo("To use this script, you would need the following information\n
Site Assetid
Protocol to be used
New URL
An existing URL of the site\n
Make sure you have all you information you need before Proceeding\n");

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

			$asset = self::checkAssetTypes($assetid);
			if (!$asset) {
				echo "Asset must be either a site, design folder, media folder, or user folder\n";
				$assetid = NULL;
			}//end if

		}//end while


		// Now we have to check whether we should add this URL to this site or not.
		require_once $SYSTEM_ROOT.'/data/private/conf/main.inc';

		$update_file_public_live_assets = FALSE;
		$new_url = NULL;
		while (is_null($new_url)) {
			$new_url = get_line('Enter the New URL: ');

			if (!(self::checkNewUrlExists($new_url))) {
				echo "The new URL : ".$new_url." is already exist.\n";
				$new_url = NULL;
				continue;
			}//end if

			if (!(self::checkNewUrlInSystemRootUrls($SYSTEM_ROOT, $new_url, $update_file_public_live_assets))) {
				echo "The provided URL is not based upon an existing System Root URL\n";
				echo "Existing System Root URLs are : \n".SQ_CONF_SYSTEM_ROOT_URLS."\n";
				$new_url = NULL;
				continue;
			}//end if

		}//end while

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
		}//end if


		$existing_urlid = self::checkExistingUrlExists($existing_url);
		if (!$existing_urlid) {
			echo "The existing URL : ".$existing_url." does not exist\n";
			exit(0);
		}//end if


		$inputs	= Array (
					'http'								=> $http,
					'https'								=> $https,
					'new_url'							=> $new_url,
					'existing_url'						=> $existing_url,
					'siteid'							=> $assetid,
					'update_file_public_live_assets'	=> $update_file_public_live_assets,
					'existing_urlid'					=> $existing_urlid[0]['urlid'],
				  );
		return $inputs;

	}//end cliInterfaceAddUrl()


	/*
	* This function add URL for a site type asset
	*
	*
	*/
	public static function addUrl($http, $https, $new_url, $existing_url, $assetid, $update_file_public_live_assets, $existing_urlid, $SYSTEM_ROOT, $quiet=FALSE)
	{
		// strip trailing slash
		$new_url = strip_url($new_url);

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// Lets check if the system is using static URL, if they use static URL, there's no point updating FILE PUBLIC LIVE assets.
		$static_root_url = SQ_CONF_STATIC_ROOT_URL;
		if (!empty($static_root_url)) {
			$update_file_public_live_assets = FALSE;
		}

		// First we check that the new URL being added is not under any system root URLs, if it is, we don't need to update this FILE PUBLIC LIVE assets.
		if ($update_file_public_live_assets) {

			// Before we do any of the processing, lets grab all the FILE assets that are LIVE, and have PUBLIC READ ACCESS.
			$asset_types_list_info = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeHierarchy('file');

			$asset_types_list = Array();
			self::getTypeListRecursive($asset_types_list_info, $asset_types_list);

			$asset_types_list[] = 'file';
			$children = Array();
			$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($assetid, $asset_types_list, TRUE);
			
			
			// Remove shadow assets, they are not required  for this purpose
			foreach ($children as $id => $content) {
				if(strpos($id,':') !== FALSE){
					unset($children[$id]);	
				}
			}

			$children = array_keys($children);	// We just need the asset id
			$public_user_id = 7;
			$children_to_update = Array();
			foreach ($children as $child_id) {
				$child_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($child_id);
				if ($child_asset->usePublicPath()) {
						$children_to_update[] = MatrixDAL::quote((string) $child_id);
				}//end if
				// Else just ignore this asset
			}//end foreach
		}

		$new_urlid = MatrixDAL::executeOne('core', 'seqNextVal', Array('seqName' => 'sq_ast_url_seq'));

		$sql_update_sq_ast_url 				= 'INSERT INTO sq_ast_url (http, https, assetid, url, urlid) values (
									:http, :https, :assetid, :new_url, :new_urlid)';


		// Copy lookup value from existing url for new url. 
		// These urls have to be restricted to:
		// 1. both paint_layout and override paint layout which are asset based (not URL specified).
		// 2. or Design look up
		//
		// The distinct is required because we are doing a join on both the url and url with a /
		// This captures both the 'design::' and 'override::design::'.
		// They have the same url but different values (in lookup_value).
		//
		// This is equivalent to doing a union of
		// lookup_value v inner join lookup l on (l.url=v.url)
		// and
		// lookup_value v inner join lookup l on (l.url || '/' = v.url)
		//
		$sql_update_sq_ast_lookup_value 	=
		'INSERT INTO sq_ast_lookup_value (url, name, value, depth)
										SELECT DISTINCT replace(v.url, :existing_url, :new_url), v.name, v.value, v.depth
                                        FROM sq_ast_lookup_value v
                                        INNER JOIN sq_ast_lookup l
                                        ON (l.url = v.url OR l.url || \'/\' = v.url)
                                        INNER JOIN sq_ast_lnk k
                                        ON (v.value=k.minorid)
                                        WHERE 
                                        k.majorid=l.assetid
                                        AND
                                        (
                                            (
                                                k.value = \'paint_layout::system::frontend\'
                                                or k.value = \'override::paint_layout::system::frontend\'
                                            )
                                            OR 
                                            (
                                              (
                                                  v.name like \'design%\' AND (k.value=v.name OR k.value = \'override::\' || v.name)
                                              )
                                              or v.name like \'override::design%\'
                                            )
                                        )
                                        AND rtrim(v.url, \'/\') = l.url
                                        AND l.root_urlid = :existing_urlid
        ';
	

		


		$sql_update_sq_ast_lookup			= 'INSERT INTO sq_ast_lookup (http, https, assetid, url, root_urlid)
										SELECT :http, :https, assetid, replace(url, :existing_url, :new_url), :new_urlid
										FROM sq_ast_lookup WHERE url like :existing_url_wildcard AND root_urlid = :existing_urlid';

		$sql_update_sq_ast_lookup_public	= 'UPDATE sq_ast_lookup set root_urlid = 0 WHERE url like :new_url_wildcard AND url like :data_url AND root_urlid = :new_urlid';

		// Now we run the query
		// 1. Add entry to sq_ast_url
		$query = MatrixDAL::preparePdoQuery($sql_update_sq_ast_url);
		MatrixDAL::bindValueToPdo($query, 'http',      $http);
		MatrixDAL::bindValueToPdo($query, 'https',     $https);
		MatrixDAL::bindValueToPdo($query, 'assetid',   $assetid);
		MatrixDAL::bindValueToPdo($query, 'new_url',   $new_url);
		MatrixDAL::bindValueToPdo($query, 'new_urlid', $new_urlid);
		MatrixDAL::execPdoQuery($query);

		// 2. Add entries to sq_ast_lookup_value
		$query = MatrixDAL::preparePdoQuery($sql_update_sq_ast_lookup_value);
		MatrixDAL::bindValueToPdo($query, 'existing_url',   $existing_url);
		MatrixDAL::bindValueToPdo($query, 'new_url',        $new_url);
		MatrixDAL::bindValueToPdo($query, 'existing_urlid', $existing_urlid);
		MatrixDAL::execPdoQuery($query);

		// 3, Add entries to sq_ast_lookup
		$query = MatrixDAL::preparePdoQuery($sql_update_sq_ast_lookup);
		MatrixDAL::bindValueToPdo($query, 'http',      $http);
		MatrixDAL::bindValueToPdo($query, 'https',     $https);
		MatrixDAL::bindValueToPdo($query, 'existing_url',          $existing_url);
		MatrixDAL::bindValueToPdo($query, 'existing_url_wildcard', $existing_url.'%');
		MatrixDAL::bindValueToPdo($query, 'new_url',               $new_url);
		MatrixDAL::bindValueToPdo($query, 'new_urlid',             $new_urlid);
		MatrixDAL::bindValueToPdo($query, 'existing_urlid',        $existing_urlid);
		MatrixDAL::execPdoQuery($query);

		if (!$quiet) {
			pre_echo($sql_update_sq_ast_url);
			pre_echo($sql_update_sq_ast_lookup_value);
			pre_echo($sql_update_sq_ast_lookup);
		}//end if

		$query = MatrixDAL::preparePdoQuery($sql_update_sq_ast_lookup_public);
		MatrixDAL::bindValueToPdo($query, 'new_url_wildcard', $new_url.'%');
		MatrixDAL::bindValueToPdo($query, 'data_url',       '%/__data/%');
		MatrixDAL::bindValueToPdo($query, 'new_urlid',        $new_urlid);
		MatrixDAL::execPdoQuery($query);

		// We have done updating regular asset, now we will update the publically served file assets.

		if (!defined(SQ_CONF_SYSTEM_ROOT_URLS)) {
			require_once $SYSTEM_ROOT.'/data/private/conf/main.inc';
		}//end if
		$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
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

				$count = 1;
				$num_children_to_update = count($children_to_update);
				foreach ($in_clauses as $condition) {
					$sql_update_sq_ast_lookup_public_file = 'INSERT INTO sq_ast_lookup (http, https, assetid, url, root_urlid)
																SELECT :http, :https, assetid, replace(url, :existing_url, :new_url), 0
																FROM sq_ast_lookup WHERE root_urlid = 0 AND '. $condition. ' AND url like :existing_url_wildcard';
					$added_condition = ' AND replace(url, :existing_url_1, :new_url_1) NOT IN (
											SELECT url from sq_ast_lookup WHERE root_urlid = 0 AND '. $condition. ' AND url like replace(url,:existing_url_2, :new_url_2))';

					$sql_update_sq_ast_lookup_public_file .= $added_condition;

					// If the last char is not "/" we'l add a slash to make sure if we have two overlap url we still can handle it.
					// Eg: 	charlie.squiz.net/hnguyen_3_18_0
					//		charlie.squiz.net/hnguyen_3_18_0_sr2    . These 2 Urls overlap each other, so the replace() wont work unless we add "/"
					if ($existing_url_public[strlen($existing_url_public)-1] != '/') {
						$existing_url_public .= '/';
					}//end if
					// If we add slash "/" for existing url we need to add to the new url too
					if ($new_url_public[strlen($new_url_public)-1] != '/') {
						$new_url_public .= '/';
					}//end if

					if (!$quiet) pre_echo($sql_update_sq_ast_lookup_public_file);
					$query = MatrixDAL::preparePdoQuery($sql_update_sq_ast_lookup_public_file);
					MatrixDAL::bindValueToPdo($query, 'http',					$http);
					MatrixDAL::bindValueToPdo($query, 'https',					$https);
					MatrixDAL::bindValueToPdo($query, 'existing_url',			$existing_url_public);
					MatrixDAL::bindValueToPdo($query, 'existing_url_1',			$existing_url_public);
					MatrixDAL::bindValueToPdo($query, 'existing_url_2',			$existing_url_public);
					MatrixDAL::bindValueToPdo($query, 'existing_url_wildcard',	$existing_url_public.'%');
					MatrixDAL::bindValueToPdo($query, 'new_url',				$new_url_public);
					MatrixDAL::bindValueToPdo($query, 'new_url_1',				$new_url_public);
					MatrixDAL::bindValueToPdo($query, 'new_url_2',				$new_url_public);
					MatrixDAL::execPdoQuery($query);
					if (!$quiet) {
						echo "\n Finished Updating ".(($count*1000 > $num_children_to_update) ? $num_children_to_update : $count*1000) ." files out of ".$num_children_to_update."\n";
					}//end if
					$count++;
				}//end foreach

			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

	}//end addUrl()


	/**
	*
	*
	*/
	public static function cliInterfaceRemoveUrl()
	{
		global $SYSTEM_ROOT;
		// For Removing, need to take in
		// -removeurl
		pre_echo("!Important: Please make sure that you know how to use this script, and remember to back up the database before proceeding\n
(Press Ctrl+C to terminate the script)\n");

		$remove_url = NULL;
		while (is_null($remove_url)) {
			$remove_url = get_line('Enter the URL to be removed: ');
		}//end while

		$remove_url_info = self::checkExistingUrlExists($remove_url);
		if ($remove_url_info === FALSE) {
			echo 'The provided URL : '.$remove_url.' does not exist';
			exit(0);
		}//end if

		$remove_urlid   = $remove_url_info[0]['urlid'];
		$remove_assetid = $remove_url_info[0]['assetid'];

		return Array (
				'remove_urlid'		=> $remove_urlid,
				'remove_assetid'	=> $remove_assetid,
				'remove_url'		=> $remove_url,
			   );

	}//end cliInterfaceRemoveUrl()


	/*
	* Function removeURL
	*
	*
	*/
	public static function removeUrl($remove_urlid, $remove_assetid, $remove_url, $quiet=FALSE, $SYSTEM_ROOT)
	{
		global $db;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// Find all other root_urlid that look like $remove_url.'%', we don't want to remove those.
		$sql_get_sub_url_root_urlid = 'SELECT url FROM sq_ast_url WHERE
											url NOT LIKE :remove_url AND
											url LIKE :remove_url_wildcard';

		$query = MatrixDAL::preparePdoQuery($sql_get_sub_url_root_urlid);
		MatrixDAL::bindValueToPdo($query, 'remove_url',          $remove_url);
		MatrixDAL::bindValueToPdo($query, 'remove_url_wildcard', $remove_url.'%');
		$avoid_urls = MatrixDAL::executePdoAssoc($query);

		// Before we do any of the processing, lets grab all the FILE assets that are LIVE, and have PUBLIC READ ACCESS.
		$asset_types_list_info = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeHierarchy('file');

		$asset_types_list = Array();
		self::getTypeListRecursive($asset_types_list_info, $asset_types_list);

		$asset_types_list[] = 'file';
		$children = Array();
		$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($remove_assetid, $asset_types_list, TRUE);
		
		// Remove shadow assets, they are not required  for this purpose
		foreach ($children as $id => $content) {
			if(strpos($id,':') !== FALSE){
				unset($children[$id]);	
			}
		}
	 
		$children = array_keys($children);	// We just need the asset id
		$public_user_id = 7;
		$children_to_update = Array();
		
		foreach ($children as $child_id) {
			$child_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($child_id);
			if ($child_asset->usePublicPath()) {
					$get_count_links_query = 'SELECT COUNT(*) FROM sq_ast_lnk WHERE minorid = '.MatrixDAL::quote($child_id);
					$count = MatrixDAL::executeSqlOne($get_count_links_query);
					// We are only calling getParents if this asset is linked in multiple places
					if ($count > 1) {
						$site_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($child_id, 'site', FALSE);
						// If we have more than 1 site as parent, potential multiple URLs, so we are not going to delete the __data URL.
						if (!(count($site_parents) > 1)) {
							// We just have 1 site parent
							$children_to_update[] = MatrixDAL::quote($child_id);
						}//end if
					} else {
							$children_to_update[] = MatrixDAL::quote($child_id);
					}//end else
			}//end if
			// Else just ignore this asset
		}//end foreach


		if (!empty($children_to_update)) {
			$in_clauses = Array();
			foreach (array_chunk($children_to_update, 999) as $chunk) {
				$in_clauses[] = ' assetid IN ('.implode(', ', $chunk).')';
			}

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

			$count = 1;
			$num_children_to_update = count($children_to_update);
			foreach ($in_clauses as $condition) {

				$sql_update_sq_ast_lookup_public  = 'DELETE FROM sq_ast_lookup WHERE root_urlid = 0 AND url LIKE :remove_url_wildcard AND url LIKE :data_url';
				$sql_update_sq_ast_lookup_public .= ' AND '.$condition;

				 foreach ($avoid_urls as $index => $data) {
				 	$sql_update_sq_ast_lookup_public        .= ' AND url NOT LIKE ' . MatrixDAL::quote($data['url'].'%');
				 }//end foreach

				if (!$quiet) pre_echo($sql_update_sq_ast_lookup_public);
				$query = MatrixDAL::preparePdoQuery($sql_update_sq_ast_lookup_public);
				MatrixDAL::bindValueToPdo($query, 'remove_url_wildcard', $remove_url_public.'%');
				MatrixDAL::bindValueToPdo($query, 'data_url', '%/__data/%');
				MatrixDAL::execPdoQuery($query);

				if (!$quiet) {
					echo "\n Finished deleting ".(($count*1000 > $num_children_to_update) ? $num_children_to_update : $count*1000) ." files out of ".$num_children_to_update;
				}//end if
				$count++;
			}//end foreach

		}//end if

		$sql_update_sq_ast_lookup_value		= 'DELETE FROM sq_ast_lookup_value WHERE url like '.MatrixDAL::quote($remove_url.'%');
		foreach ($avoid_urls as $index => $data) {
			$sql_update_sq_ast_lookup_value .= ' AND url NOT LIKE ' . MatrixDAL::quote($data['url'].'%');
		}//end foreach

		$sql_update_sq_ast_lookup			= 'DELETE FROM sq_ast_lookup WHERE root_urlid = '.MatrixDAL::quote($remove_urlid).' AND url like '.MatrixDAL::quote($remove_url.'%').'';

		foreach ($avoid_urls as $index => $data) {
			$sql_update_sq_ast_lookup			.= ' AND url NOT LIKE ' . MatrixDAL::quote($data['url'].'%');
		}//end foreach

		$sql_update_sq_ast_url				= 'DELETE FROM sq_ast_url WHERE urlid = '.MatrixDAL::quote($remove_urlid).' AND url like '.MatrixDAL::quote($remove_url).'';

		// We run the query in different order
		// 1. Remove entries in sq_ast_lookup_value
		// 2. Remove entries in sq_ast_lookup
		// 3. Remove the entry in sq_ast_url
		MatrixDAL::executeSql($sql_update_sq_ast_lookup_value);
		MatrixDAL::executeSql($sql_update_sq_ast_lookup);
		MatrixDAL::executeSql($sql_update_sq_ast_url);

		if (!$quiet) {
			pre_echo($sql_update_sq_ast_lookup_value);
			pre_echo($sql_update_sq_ast_lookup);
			pre_echo($sql_update_sq_ast_url);
		}//end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

	}//end removeURL()


	/**
	* This function check whether the site to be applied URL on is of type site or not
	*
	*/
	private static function checkAssetTypes($assetid)
	{
		$allow_asset_types = array_keys($GLOBALS['SQ_SYSTEM']->am->getAssetTypeHierarchy('web_folder', 'system'));
		$allow_asset_types[] = 'site';
		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (!in_array($asset->type(), $allow_asset_types)) {
			return FALSE;
		}//end if
		return $asset;

	}//end checkAssetTypes


	/**
	* This function check whether the provided existing URL is really in the system
	*
	*/
	public static function checkExistingUrlExists($existing_url)
	{
		$sql_check_existing_url = 'SELECT urlid, assetid FROM sq_ast_url WHERE url LIKE :existing_url';

		$query = MatrixDAL::preparePdoQuery($sql_check_existing_url);
		MatrixDAL::bindValueToPdo($query, 'existing_url', $existing_url);
		$existing_urlid = MatrixDAL::executePdoAll($query);
		if (empty($existing_urlid)) {
			return FALSE;
		}//end if
		return $existing_urlid;

	}//end checkExistingUrlExists()


	/*
	* This function check whether the new Url being added is allowed, ie whether there it is belong to the system roots list.
	*
	*/
	private static function checkNewUrlInSystemRootUrls($system_root, $new_url, &$update_file_public_live_assets)
	{
		require_once $system_root.'/data/private/conf/main.inc';
		$update_file_public_live_assets = FALSE;
		$root_urls = Array();

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
			return FALSE;
		}//end if

		return TRUE;

	}//end checkNewUrl()


	/*
	* This function check if the new Url is already exist in the system
	*
	*/
	private static function checkNewUrlExists($new_url)
	{
		//Does any asset have same URL?
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL(NULL,  strip_url($new_url), TRUE, TRUE);
		if($asset != NULL) return FALSE;

		$sql_check_new_url = 'SELECT url FROM sq_ast_url WHERE url = :new_url';

		$query = MatrixDAL::preparePdoQuery($sql_check_new_url);
		MatrixDAL::bindValueToPdo($query, 'new_url', strip_url($new_url));
		$new_url_check = MatrixDAL::executePdoOne($query);
		if (!empty($new_url_check)) {
			return FALSE;
		}//end if

		return TRUE;

	}//end checkNewUrlExists()


	/**
	*	This function return all the asset types and theirs subtypes
	*/
	private static function getTypeListRecursive($asset_types_list_info, &$asset_types_list)
	{
		foreach ($asset_types_list_info as $asset_type => $type_info)
		{
				$asset_types_list[]   = $asset_type;
				if (isset($type_info['subs'])) {
					self::getTypeListRecursive($type_info['subs'], $asset_types_list);
				}//end if
		}//end foreach

	}//end getTypeListRecursive()


}//end class
