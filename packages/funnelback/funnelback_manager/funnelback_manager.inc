<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: funnelback_manager.inc,v 1.41 2012/08/30 00:58:43 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Funnelback_Manager
*
* Controls the OEM version of Funnelback. Also maintains a database of sitemap XML cache used so far exclusively for Funnelback.
*
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>
* @version $Revision: 1.41 $
* @package MySource_Matrix_Packages
* @subpackage funnelback
*/
class Funnelback_Manager extends Asset
{


	/**
	* Weather the asset is deprecated or not.
	* this will be used in asset_map.inc:_cloneAsset() to see if the asset type
	* can be created (cloned) on the system
	*/
	public $deprecated = TRUE;


	/**
	* Marking the asset deprecated since so that we can
	* use it when warning the users on backend
	*/
	public $deprecated_since = '5.1';


	/**
	* Marking the asset deprecated since so that we can
	* use it when warning the users on backend
	*/
	public $deprecated_message = '';


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);
		$this->deprecated_message = translate('The Funnelback Indexing User setting has been moved to System Configuration.');

	}//end constructor


	/**
	* Create this asset
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	function create(Array &$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) {
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = FALSE;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $linkid;

	}//end create()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE, $contextid=NULL)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');

	}//end _getName()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		return FALSE;

	}//end canDelete()


	/**
	* returns FALSE always because we don't want to be able to clone a funnelback manager
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return FALSE;

	}//end canClone()


//--        EVENT STUBS        --//


	/**
	* Event fired when an asset is updated
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								Component is set as $vars['component']
	*
	* @return boolean
	* @access public
	*/
	function onAssetUpdate(&$broadcaster, $vars=Array())
	{
		if (!$this->attr('indexing')) return FALSE;
		$vars['action'] = 'update';
		$this->updateAsset($broadcaster, $vars);

		return TRUE;

	}//end onAssetUpdate()


	/**
	* Event fired when an asset has its status changed
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								Component is set as $vars['component']
	*
	* @return boolean
	* @access public
	*/
	function onAssetStatusUpdate(&$broadcaster, $vars=Array())
	{
		if (!$this->attr('indexing')) return FALSE;
		$vars['action'] = 'status_update';
		$this->updateAsset($broadcaster, $vars);

		return TRUE;

	}//end onAssetStatusUpdate()


	/**
	* Event fired when an assets atrtributes have changed
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			a list of attributes that have changed
	*
	* @return boolean
	* @access public
	*/
	function onAttributeChange(&$broadcaster, $vars=Array())
	{
		if (!$this->attr('indexing')) return FALSE;
		$vars['action'] = 'update';
		$this->updateAsset($broadcaster, $vars);

		return TRUE;

	}//end onAttributeChange()


	/**
	* Event fired when an asset is created
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								Component is set as $vars['component']
	*
	* @return boolean
	* @access public
	*/
	function onContentsUpdated(&$broadcaster, $vars=Array())
	{
		if (!$this->attr('indexing')) return FALSE;
		$vars['action'] = 'update';
		$this->updateAsset($broadcaster, $vars);

		return TRUE;

	}//end onContentsUpdated()


	/**
	* Event fired when an asset gets deleted. We want to flush any key data in the index table
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								Component is set as $vars['component']
	*
	* @return boolean
	* @access public
	*/
	function onAssetDeleted(&$broadcaster, $vars=Array())
	{
		if (!$this->attr('indexing')) return FALSE;
		$vars['action'] = 'delete';
		$this->updateAsset($broadcaster, $vars);

		return TRUE;

	}//end onAssetDeleted()


	/**
	* Event fired when an asset's permission gets changed. We want to flush any key data in the index table
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								Component is set as $vars['component']
	*
	* @return boolean
	* @access public
	*/
	function onPermissionChange(&$broadcaster, $vars=Array())
	{
		if (!$this->attr('indexing')) return FALSE;
		$vars['action'] = 'permission_change';
		$this->updateAsset($broadcaster, $vars);

		return TRUE;

	}//end onPermissionChange()


	/**
	* Event fired when an asset's lookups are updated. We want to flush any key data in the index table
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								Component is set as $vars['component']
	*
	* @return boolean
	* @access public
	*/
	function onLookupsUpdated(&$broadcaster, $vars=Array())
	{
		if (!$this->attr('indexing')) return FALSE;
		$vars['action'] = 'lookups_updated';
		$this->updateAsset($broadcaster, $vars);

		return TRUE;

	}//end onLookupsUpdated()


	/**
	* Event fired when an asset metadata gets deleted. We want to flush any key data in the index table
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								Component is set as $vars['component']
	*
	* @return boolean
	* @access public
	*/
	function onMetadataDeleted(&$broadcaster, $vars=Array())
	{
		if (!$this->attr('indexing')) return FALSE;
		$vars['action'] = 'metadata_delete';
		$this->updateAsset($broadcaster, $vars);

		return TRUE;

	}//end onMetadataDeleted()


	/**
	* Event fired when an asset metadata gets updated.
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								Component is set as $vars['component']
	*
	* @return boolean
	* @access public
	*/
	function onMetadataUpdate(&$broadcaster, $vars=Array())
	{
		if (!$this->attr('indexing')) return FALSE;
		$vars['action'] = 'metadata_update';
		$this->updateAsset($broadcaster, $vars);

		return TRUE;

	}//end onMetadataUpdate()


// --- ASSET UPDATE --- //


	/**
	* Asset has been changed in some way, update the funnelback table to suit
	*
	* @param object	&$asset	the asset to update
	* @param array	$vars	the components to update
	*
	* @return boolean
	* @access public
	*/
	public function updateAsset(&$asset, $vars=Array())
	{
		// if funnelback manager is updated then there is infinite recursion!!!
		if ($asset->id == $this->id) return FALSE;

		// We don't need to update the xml while we are creating a new asset.
		// This will stop the xml getting regenerated while
		// permissions, metadata, workflow are being applied.
		if (isset($asset->_tmp['__creating__']) && $asset->_tmp['__creating__'] === TRUE) return FALSE;

		// We need the URL, if the asset has no url, bail early.
		$urls = $asset->getURLs();
		if (empty($urls)) {
			return $this->_cleanupEntriesByAssetid($asset->id);
		}

		$action = array_get_index($vars, 'action', '');

		// If the action is delete, just delete and do not worry about the rest
		if ($action == 'delete') {
			return $this->_cleanupEntriesByAssetid($asset->id);
		}//end if

		// Exclusions
		$exclusions = $this->attr('global_exclude');
		$excluded_assets = array_get_index($exclusions, 'assetid', Array());
		$excluded_types = array_get_index($exclusions, 'type', Array());
		$excluded_nodes = array_get_index($exclusions, 'tree', Array());
		$asset_type = $asset->type();

		// Exclude By Assetid
		if (in_array($asset->id, $excluded_assets)) {
			return $this->_cleanupEntriesByAssetid($asset->id);
		}//end if

		// Exclude By Type
		if (in_array($asset_type, $excluded_types)) {
			return $this->_cleanupEntriesByAssetid($asset->id);
		}//end if

		// Exclude By Root Node
		if (!empty($excluded_nodes)) {
			if ($GLOBALS['SQ_SYSTEM']->am->isUnderRootNodes($asset->id, $excluded_nodes)) {
				return $this->_cleanupEntriesByAssetid($asset->id);
			}
		}//end if

		// Geeez, if the action is a status change, we *need* to hack the status, otherwise it doesn't register
		// This is because the status at this point, hasn't gone through a status change yet.
		$new_status = array_get_index($vars, 'new_status', 0);
		$old_status = array_get_index($vars, 'old_status', 0);
		if ($action == 'status_update' && $new_status != $asset->status && $new_status != $old_status) {
			$asset_status = $new_status;
		} else {
			$asset_status = $asset->status;
		}//end if

		$public_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');
		$live = (!($asset_status & SQ_SC_STATUS_NOT_LIVE) && $asset->readAccess(Array($public_user)));
		$lookups = $asset->getLookups();

		// Delete first, ask questions later...
        if ($this->_cleanupEntriesByAssetid($asset->id)) {
            // get a placeholder for the url.
            // it'll be filled in down the line.
            $url_template = $this->_generateXML('%%sq_temp_fb_url%%', $asset);

			// Create a list of urls per root url id.
			// We do this once to save having to loop over it below.
			// Should save some time if a system has a lot of urls applied
			// to a site.
			$root_urls = array();
			foreach ($lookups as $lookup) {
				$lookup_url = array_get_index($lookup, 'url', '');
				$root_urlid = array_get_index($lookup, 'root_urlid', FALSE);
				if ($root_urlid === FALSE) continue;
				if (empty($lookup_url)) continue;
				$root_urls[$lookup_url] = $root_urlid;
			}//end foreach

			// Now go through each url, adding the correct entries back in, with the correct xml
			foreach ($urls as $url_info) {
				// This should *never* happen
				if (empty($url_info['url'])) continue;

				// Find the root url
				if (array_key_exists($url_info['url'], $root_urls) === TRUE) {
					$root_urlid = $root_urls[$url_info['url']];
				} else {
					continue;
				}

				// Compile the URLs
				$http_url  = (($url_info['http']) ? 'http://'.$url_info['url'] : '');
				$https_url = (($url_info['https']) ? 'https://'.$url_info['url'] : '');

				$all_site_urls = $this->_getSiteURL($root_urlid, $asset, $url_info['url']);

				// HTTP Url found, add entry
                if (!empty($http_url)) {
                    $sitemap_url = str_replace('&#039;', '&apos;', htmlspecialchars($http_url, ENT_QUOTES, SQ_CONF_DEFAULT_CHARACTER_SET));
                    $sitemap_xml = str_replace('%%sq_temp_fb_url%%', $sitemap_url, $url_template);
					if (!empty($sitemap_xml)) {
						foreach ($all_site_urls as $site_url) {
							if(strpos($http_url,'__data') !== FALSE) {
								$original_urls = $asset->getOriginalURL($http_url, $site_url);
							} else {
								$original_urls = Array($http_url);
							}
							foreach($original_urls as $original_url) {
								if (strpos($site_url, 'http:') === 0) {
									$this->_addEntry($http_url, $asset->id, $site_url, $root_urlid, $live, $sitemap_xml, $original_url);
								}//end if
							}
						}//end foreach
					}//end if
				}//end if

				// HTTPS Url found, add entry
				if (!empty($https_url)) {
                    $sitemap_url = str_replace('&#039;', '&apos;', htmlspecialchars($https_url, ENT_QUOTES, SQ_CONF_DEFAULT_CHARACTER_SET));
                    $sitemap_xml = str_replace('%%sq_temp_fb_url%%', $sitemap_url, $url_template);
					if (!empty($sitemap_xml)) {
						foreach ($all_site_urls as $site_url) {
							if(strpos($https_url,'__data') !== FALSE) {
								$original_urls = $asset->getOriginalURL($https_url, $site_url);
							} else {
								$original_urls = Array($https_url);
							}
							foreach($original_urls as $original_url) {
								if (strpos($site_url, 'https:') === 0) {
									$this->_addEntry($https_url, $asset->id, $site_url, $root_urlid, $live, $sitemap_xml, $original_url);
								}//end if
							}
						}//end foreach
					}//end if
				}//end if
			}//end foreach
		} else {
			// Failed to delete
			return FALSE;
		}//end if

		return TRUE;

	}//end updateAsset()


// --- SITE MAP CACHING FUNCTIONS --- //


	/**
	 * Generate the Sitemap XML for this asset based on the info provided
	 *
	 * @param string	$url    The url of the broadcasting asset. This is just
	 *                          a placeholder url to make it easier to fill in later.
	 *                          It also saves calling the same function multiple times
	 *                          and only change the one attribute.
	 * @param object	$asset  The broadcasting asset
	 *
	 * @return string
	 * @access public
	 */
	public function _generateXML($url, $asset)
	{
		// No asset? Why even bother?
		if (is_null($asset)) {
			return '';
		}//end if

		// Some definitions
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		$xml  = '<url>';
		$xml .= '<loc>'.$url.'</loc>';
		$xml .= '<lastmod>'.date('Y-m-d', $asset->updated).'T'.date('H:i:s', $asset->updated).'</lastmod>';

		$access_locks = $this->getAccessLocks($asset);
		if (!empty($access_locks)) {
			$xml .= '<fbmeta name="access_locks" content="'.$access_locks.'" />';
		}
		// Add in the metadata
		// get schemas first using just getMetadata() but it wont get us the default value on the asset
		// which is not good enough for us so get the default values later again and merge it
		$metadata = $mm->getMetadata($asset->id);

		// now the default values
		$schemas_applied = $mm->getSchemas($asset->id, TRUE);
		$default_values = Array();
		foreach($schemas_applied as $schemaid) {
			$defaults = $mm->getSchemaDefaultValues($schemaid);
			// get the correct format of the array
			foreach ($defaults as $field_id => $field_info) {
				if(isset($metadata[$field_id])) continue;
				$metadata[$field_id][0] = $field_info;
			}
		}

		foreach ($metadata as $info) {
			$name = $info[0]['name'];

			// Fall back, if a metadata field is name "access_locks", it will be ignored.
			// The name "access_locks" are reserved for passing in the permissions
			if ($name == 'access_locks') continue;

			$value = $info[0]['value'];
			require_once SQ_FUDGE_PATH.'/general/text.inc';
			$keywords = retrieve_keywords_replacements($value);
			$replacements = Array();
			foreach ($keywords as $keyword) {
				$replacements[$keyword] = $asset->getKeywordReplacement($keyword);
			}//end foreach
			replace_keywords($value, $replacements);
			$name = htmlspecialchars($name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
			$value = htmlspecialchars($value, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
			$xml .= '<fbmeta name="'.$name.'" content="'.$value.'" />';
		}//end foreach
		$xml .= '<fbmeta name="SQUIZASSETID" content="'.$asset->id.'" />';
		$xml .= '<fbmeta name="SQUIZASSETTYPE" content="'.$asset->type().'" />';

		$xml .= '</url>';

		return $xml;

	}//end _generateXML()


	/**
	 * generates the comma seperated list for all the permission
	 * the passed asset has
	 *
	 * @param object	$asset  The broadcasting asset
	 *
	 * @return string
	 * @access public
	 */
	public function getAccessLocks($asset)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		/*
		 * Add in the permissions, with the following logic:
		 *	- No permissions, return nothing
		 *	- Public permission and live status, give a "public" lock
		 *  - Specific permission, add the lock
		 */
		if (!($asset->status & SQ_SC_STATUS_NOT_LIVE)) {
			$permissions = $am->getPermission($asset->id, SQ_PERMISSION_READ, TRUE);
		} else {
			$permissions = $am->getPermission($asset->id, SQ_PERMISSION_WRITE, TRUE);
		}
		$public_user_id			= $am->getSystemAssetId('public_user');
		$root_user_id			= $am->getSystemAssetId('root_user');
		$sys_admin_folder_id	= $am->getSystemAssetId('system_user_group');

		// Public user found AND page is live, place in the public lock
		if (in_array($public_user_id, $permissions) && !($asset->status & SQ_SC_STATUS_NOT_LIVE)) {
			$permissions[] = 'public';
		}//end if
		$all_permissions = Array();
		foreach ($permissions as $permission) {
			$all_permissions[] = "'".$this->encodeText(htmlspecialchars($permission, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET))."'";
		}//end foreach

		// Give the root user, "GOD" access
		if (!in_array("'".$root_user_id."'", $all_permissions)) {
			$all_permissions[] = "'".$this->encodeText(htmlspecialchars($root_user_id, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET))."'";
		}//end if

		// also give the system_admin_folder the access
		if (!in_array("'".$sys_admin_folder_id."'", $all_permissions)) {
			$all_permissions[] = "'".$this->encodeText(htmlspecialchars($sys_admin_folder_id, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET))."'";
		}//end if

		$xml = rtrim(implode(',', $all_permissions), ',');

		return $xml;

	}//end getAccessLocks()


	/**
	 * Add an entry for the url and assetid
	 *
	 * @param string	$url			The url of the broadcasting asset
	 * @param int		$assetid		The assetid of the broadcasting asset
	 * @param string	$root_url		The root url of the asset
	 * @param int		$root_urlid		The root url id of the asset
	 * @param boolean	$live			Is the broadcasting asset live?
	 * @param string	$xml			The xml ready for the sitemap
	 * @param string	$original_url	the original_url the __data url points to
	 *
	 * @return boolean
	 * @access private
	 */
	public function _addEntry($url, $assetid, $root_url, $root_urlid, $live, $xml, $original_url)
	{
		/**
		 * Make sure the url we're looking at starts with the right root_url
		 * If it doesn't, the site may have multiple urls and we're looking
		 * at the wrong one.
		 * In which case we can ignore this one.
		 *
		 * We check for the root_url first. If that's not found, strip a "sub-dir" off
		 * and try again.
		 * This is required if you have a matrix site like http://www.example.com/mysite
		 * and the url we're trying to add is http://www.example.com/__data/..../file.pdf
		 *
		 * Repeat that until base_url is back to a '.' (which of course means it won't match).
		 */
		$found_url = false;
		$base_url = $root_url;
		while ($base_url !== '.') {
			//if we have reached http:// or https:// break out coz that WILL match
			if($base_url == 'http:' || $base_url == 'https:') break;

			if (strpos($url, $base_url) === 0) {
				$found_url = true;
				break;
			}
			$base_url = dirname($base_url);
		}

		/**
		 * Well, we tried. They don't match. Don't add the entry.
		 */
		if (!$found_url) {
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		try {
			$check_sql = 'SELECT assetid FROM sq_fnb_idx WHERE url=:url AND assetid=:assetid AND root_url=:root_url';
			$check_query = MatrixDAL::preparePDOQuery($check_sql);
			MatrixDAL::bindValueToPdo($check_query, 'url', $url);
			MatrixDAL::bindValueToPdo($check_query, 'assetid', $assetid);
			MatrixDAL::bindValueToPdo($check_query, 'root_url', $root_url);
			$results = MatrixDAL::executePdoOne($check_query);
			if (empty($results)) {
				// Entry does not exist, adding a new entry
				$sql = 'INSERT INTO sq_fnb_idx (url, assetid, root_url, root_urlid, live, xml, original_url) VALUES (:url, :assetid, :root_url, :root_urlid, :live, :xml, :original_url)';
				$query = MatrixDAL::preparePDOQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'url', $url);
				MatrixDAL::bindValueToPdo($query, 'assetid', $assetid);
				MatrixDAL::bindValueToPdo($query, 'root_url', $root_url);
				MatrixDAL::bindValueToPdo($query, 'root_urlid', $root_urlid);
				MatrixDAL::bindValueToPdo($query, 'live', ($live) ? '1' : '0');
				MatrixDAL::bindValueToPdo($query, 'xml', $xml);
				MatrixDAL::bindValueToPdo($query, 'original_url', $original_url);
				MatrixDAL::execPdoQuery($query);
			}//end if
		} catch (PDOException $e) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			trigger_error('Unable to add entry to database: '.$e->getMessage());
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();


		return TRUE;

	}//end _addEntry()


	/**
	 * Cleanup old entries on the table by assetid ie. delete all entries for the broadcasting asset
	 *
	 * @param int	$assetid	The assetid of the broadcasting asset
	 *
	 * @return boolean
	 * @access private
	 */
	private function _cleanupEntriesByAssetid($assetid)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		try {
			$sql = 'DELETE FROM sq_fnb_idx WHERE assetid=:assetid';
			$query = MatrixDAL::preparePDOQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'assetid', $assetid);
			MatrixDAL::execPdoQuery($query);
		} catch (PDOException $e) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error('Unable to remove entries from database: '.$e->getMessage());
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end _cleanupEntriesByAssetid()


	/**
	 * Generate the XML Cache and saves to the file specified
	 *
	 * @param string	$url		The url of the Cache to generate
	 * @param string	$assetid	The assetid of the site this cache is
	 * @param boolean	$public		Return only public results
	 *
	 * @return string
	 * @access public
	 */
	public function generateXMLCache($url=NULL, $assetid=0, $public=TRUE)
	{
		$results = $this->getXMLCache($url, $assetid, $public);

		ob_start();
			echo '<'.'?xml version="1.0" encoding="utf-8"?'.'>';
			echo '<urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9">';
			foreach ($results as $result) {
				echo $result['xml']."\n";
			}//end foreach
			echo '</urlset>';
			$contents = ob_get_contents();
		ob_end_clean();

		return $contents;

	}//end generateXMLCache()


	/**
	 * Get the xml from the index table
	 *
	 * @param string	$url		The url of the Cache to generate
	 * @param string	$assetid	The assetid of the site this cache is
	 * @param boolean	$public		Return only public results
	 * @param array		$excludes	An array of URLs to be excluded (including their children)
	 *
	 * @return array
	 * @access public
	 */
	public function getXMLCache($url=Array(), $assetid=0, $public=TRUE, $excludes=Array())
	{
		if (empty($url) || empty($assetid)) {
			// No parent nor sitemap url found
			return Array();
		}//end if
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		$is_site_asset = !empty($asset) ? ($asset->type() == 'site') : TRUE;

		$where = Array();
		$bind_vars = Array();
		$include_collection_sql = '';
		$exclude_string = '';

		// Compile a list of assets to list based on URL
		foreach (array_values($url) as $index => $single_url) {
			if (!$is_site_asset) {
				$suburl = substr($single_url, strpos($single_url, '://') + 3);
				$root_url_info = $GLOBALS['SQ_SYSTEM']->am->getRootURL($suburl);
				$root_url = array_get_index($root_url_info, 'url', '');
				$assetid = array_get_index($root_url_info, 'assetid', $assetid);

				// If the collection is not site, include the collection asset as well
				$bind_vars['asset_url'.$index] = $single_url;
				$include_collection_sql = ' OR f.url LIKE :asset_url'.$index;
			} else {
				$root_url = $single_url;
			}

			$single_url = rtrim($single_url, '/');
			$root_url = rtrim($root_url, '/');
			$protocol = parse_url($single_url, PHP_URL_SCHEME);
			$where[] = '((f.url LIKE :url'.$index.$include_collection_sql.') AND f.root_urlid IN (SELECT urlid FROM sq_ast_url WHERE assetid=:assetid'.$index.')) OR ((f.root_url = :root_url'.$index.' AND f.root_urlid=\'0\') AND (f.original_url LIKE :original_url'.$index.'))';
			$bind_vars['url'.$index] = $single_url.'/%';
			$bind_vars['original_url'.$index] = $single_url.'/%';
			$bind_vars['assetid'.$index] = $assetid;
			if (strpos($root_url, 'http:') === FALSE && strpos($root_url, 'https:') === FALSE) {
				$bind_vars['root_url'.$index] = $protocol.'://'.$root_url;
			} else {
				$bind_vars['root_url'.$index] = $root_url;
			}
		}//end foreach

		// Compile a list of assets to be excluded based on URL
		foreach (array_values($excludes) as $index => $exclude) {
			$excluded[] = 'f.url||\'/\' NOT LIKE :exclude'.$index.' OR f.url <> :exclude_asset'.$index;
			$bind_vars['exclude'.$index] = rtrim($exclude, '/').'/%';
			$bind_vars['exclude_asset'.$index] = rtrim($exclude, '/');
		}//end foreach
		if (!empty($excluded)) {
			$exclude_string = ' AND (('.implode(') AND (', $excluded).'))';
		}//end if

		if (isset($root_url_info)) unset($root_url_info);

		$db = MatrixDAL::getDb();
		try {
			if (MatrixDAL::getDbType() == 'pgsql') {
				$sql = 'SELECT DISTINCT f.url, f.xml
						 FROM sq_fnb_idx f
						  INNER JOIN sq_ast_lnk l ON l.minorid=f.assetid
							WHERE l.is_dependant=:dependant';
			} else {
				// Oracle fun n games
				$sql = 'SELECT f.url, f.assetid, f.root_url, f.xml
						 FROM sq_fnb_idx f
						  INNER JOIN sq_ast_lnk l ON l.minorid=f.assetid
							WHERE l.is_dependant=:dependant';
			}//end if

			if ($public) $sql .= ' AND f.live=:live';
			$sql .= ' AND (';
			$sql .= rtrim(implode(' OR ', $where), 'OR');
			$sql .= ')';
			$sql .= $exclude_string;
			$sql .= ' ORDER BY f.url';

			$query = MatrixDAL::preparePDOQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'dependant', '0');
			if ($public) MatrixDAL::bindValueToPdo($query, 'live', '1');
			foreach ($bind_vars as $var_name => $var_value) {
				MatrixDAL::bindValueToPdo($query, $var_name, $var_value);
			}//end foreach
			$results = MatrixDAL::executePdoAssoc($query);

		} catch (Exception $e) {
			trigger_error('DB Error '.$e->getMessage());
			return Array();
		}

		// If Oracle, we're not quite done yet
		if (MatrixDAL::getDbType() == 'oci' && !empty($results)) {
			$unique_results = Array();
			foreach($results as $key => $result) {
				// get rid of all the duplicate entires
				if (isset($unique_results[$result['url'].'|'.$result['assetid'].'|'.$result['root_url']])) {
					unset($results[$key]);
				} else {
					$unique_results[$result['url'].'|'.$result['assetid'].'|'.$result['root_url']] = 1;
				}
			}
			unset($unique_results);
		}

		return $results;

	}//end getXMLCache()


	/**
	 *  Get the site URL based on the root_urlid
	 *
	 * @param string	$root_urlid	The root urlid
	 * @param object	&$asset		The current asset, only used when the $root_urlid=0
	 *
	 * @return string
	 * @access protected
	 */
	protected function _getSiteURL($root_urlid='0', &$asset, $asset_url)
	{
		$site_url = Array();

		if (empty($root_urlid)) {
			// Empty root_urlid, then most likely file is public, so just grab the site urls
			$sites = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($asset->id, 'site', TRUE));
			$urls = Array();
			foreach ($sites as $site) {
				$url_info = $GLOBALS['SQ_SYSTEM']->am->getUrls($site);
				foreach ($url_info as $info) {
					$urls[] = $info;
				}//end foreach
			}//end foreach

			if (!empty($urls)) {
				foreach ($urls as $details) {
					$http  = array_get_index($details, 'http', 0);
					$https = array_get_index($details, 'https', 0);
					$url   = array_get_index($details, 'url', '');

					// *Sigh* ignore empty urls
					if (empty($url)) continue;

					// Make sure we only accept site urls which has the same root domain as __data file url
					$system_root_url = sq_root_url($asset_url, FALSE);
					if(strpos($url, $system_root_url) !== 0) continue;

					// Create the site url
					if ($http) {
						$site_url[] = 'http://'.$url;
					}//end if
					if ($https) {
						$site_url[] = 'https://'.$url;
					}//end if
				}//end foreach
			}//end if
		} else {
			// We have a root_urlid, find out the site url
			$db = MatrixDAL::getDb();
			try {
				$sql = 'SELECT http, https, url FROM sq_ast_url WHERE urlid=:urlid';
				$query = MatrixDAL::preparePDOQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'urlid', $root_urlid);
				$results = MatrixDAL::executePdoAssoc($query);
			} catch (Exception $e) {
				trigger_error('DB Error '.$e->getMessage());
				return $site_url;
			}//end try-catch

			if (!empty($results)) {
				// Only care about the first result (it should be the only result)
				$result = $results[0];
				$http	= array_get_index($result, 'http', 0);
				$https  = array_get_index($result, 'https', 0);
				$url    = array_get_index($result, 'url', '');

				if (($http) && !empty($url)) {
					$site_url[] = 'http://'.$url;
				}//end if

				if (($https) && !empty($url)) {
					$site_url[] = 'https://'.$url;
				}//end if
			}//end if
		}//end if

		return $site_url;

	}//end _getSiteURL()


// --- FUNNELBACK COLLECTIONS FUNCTIONS --- //


	/**
	 * Is the passed collection id valid or not
	 *
	 * @param string	$collection_id	The collection id to verify
	 *
	 * @return boolean
	 * @access public
	 */
	public function isCollectionValid($collection_id)
	{
		$collections = $this->getCollections();

		if (isset($collections[$collection_id]['valid']) && $collections[$collection_id]['valid'] === TRUE) {
			return TRUE;
		}//end if

		return FALSE;

	}//end isCollectionValid()


	/**
	 * Get all collections on this install
	 *
	 * @return array
	 * @access public
	 */
	public function getCollections()
	{
		$collections = $this->attr('collections');

		return $collections;

	}//end getCollections()


	/**
	 * Get the collection name of the collection passed in
	 *
	 * @param string	$collection_id	The id of the collection to find the name for
	 *
	 * @return string
	 * @access public
	 */
	public function getCollectionName($collection_id)
	{
		$collections = $this->attr('collections');
		$collection  = array_get_index($collections, $collection_id, Array());
		return array_get_index($collection, 'name', '');

	}//end getCollectionName()


	/**
	* Get the data path of the collection
	*
	* @param string	$collection_id	The ID of the collection to get the data path for
	* @param string	$sub_dir		The name of a sub directory if needed
	*
	* @return string
	* @access public
	*/
	public function getCollectionDataPath($collection_id, $sub_dir='')
	{
		$path = '';
		$fb_data_path = $this->data_path;
		$collections = $this->getCollections();
		if (empty($collection_id)) return $path;
		if (!isset($collections[$collection_id])) return $path;

		$path = $fb_data_path.'/'.$collection_id;
		if (!empty($sub_dir)) $path .= '/'.$sub_dir;
		if (!is_dir($path) && !is_writable($path)) {
			// Path may not exist, try and create it
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			$status = create_directory($path);
			if ($status === FALSE) {
				// Error error error
				trigger_localised_error('FNB0013', translate('Unable to create the collection path'), E_USER_WARNING);
				$path = '';
			}//end if
		}//end if

		return $path;

	}//end getCollectionDataPath()


	/**
	* Get the config files of a collection
	*
	* @param string	$collection_id	The ID of the collection to get the data path for
	*
	* @return string
	* @access public
	*/
	function getCollectionConfigFiles($collection_id)
	{
		$config = Array();
		$path = $this->getCollectionDataPath($collection_id, 'conf');
		if (empty($path)) return $config;

		$config['executables'] = $path.'/executables.cfg';
		$config['collection'] = $path.'/collection.cfg';
		$config['fluster'] = $path.'/contextual_navigation.cfg';
		$config['metadata'] = $path.'/metamap.cfg';
		$config['xml'] = $path.'/xml.cfg';

		return $config;

	}//end getCollectionConfigFiles()


	/**
	* Create a collection config file, based on a default
	*
	* @param string	$file	The file to create
	* @param string	$config	The config file to create eg. fluster, metamap etc.
	*
	* @return void
	* @access public
	*/
	function createCollectionConfig($file, $config)
	{
		$config = (string) $config;
		switch ($config) {
			case 'collection':
				$default_file = $this->getFunnelbackPath().'/conf/collection.cfg';
				if (file_exists($default_file) && !file_exists($file)) {
					require_once SQ_FUDGE_PATH.'/general/file_system.inc';
					copy_file($default_file, $file);
				}//end if
			break;

			case 'executables':
				$default_file = $this->getFunnelbackPath().'/conf/executables.cfg';
				if (file_exists($default_file) && !file_exists($file)) {
					require_once SQ_FUDGE_PATH.'/general/file_system.inc';
					copy_file($default_file, $file);
				}//end if
			break;

			case 'fluster':
			case 'fluster.cfg':
			case 'contextual_navigation.cfg':
				$default_file = $this->getFunnelbackPath().'/conf/contextual_navigation.cfg';
				if (file_exists($default_file) && !file_exists($file)) {
					require_once SQ_FUDGE_PATH.'/general/file_system.inc';
					copy_file($default_file, $file);
					$lines = $this->readConfiguration($file, 'fluster');
					$this->writeConfiguration($file, $lines, 'fluster');
				}//end if
			break;

			case 'metadata':
			case 'metamap':
			case 'metamap.cfg':
				$default_file = $this->getFunnelbackPath().'/conf/metamap.cfg';
				if (file_exists($default_file) && !file_exists($file)) {
					require_once SQ_FUDGE_PATH.'/general/file_system.inc';
					copy_file($default_file, $file);
					$lines = $this->readConfiguration($file, 'metadata', ',');
					$this->writeConfiguration($file, $lines, 'metadata', ',');
				}//end if
			break;

			case 'xml':
				$default_file = $this->getFunnelbackPath().'/conf/xml.cfg';
				if (file_exists($default_file) && !file_exists($file)) {
					require_once SQ_FUDGE_PATH.'/general/file_system.inc';
					copy_file($default_file, $file);
				}//end if
			break;

			default:
				// Meh
			break;
		}//end switch

	}//end createCollectionConfig()


	/**
	* Returns the types of collections allowed
	*
	* @return array
	* @access private
	*/
	function _getCollectionTypes()
	{
		return Array(
					'web'	=> translate('Web Collection'),

					'meta'	=> translate('Meta Collection'),

			   );

	}//end _getCollectionTypes()


// --- FUNNELBACK INDEXING FUNCTIONS --- //


	/**
	* Get the indexing options
	*
	* @return array
	* @access public
	*/
	function getIndexingOptions()
	{
		$options = Array();
		$available_opts = Array(
							'politeness'			=> '-politeness',
							// 'orgname'				=> '-O',
							'spelleroff'			=> '-spelleroff',
							'spelling_threshold'	=> '-spelling_threshold',
							'ibd'					=> '-ibd',
							'ixcom'					=> '-ixcom',
						  );

		// Add some defaults
		$options[] = '-annie';
		$options[] = '-cleanup';

		// Now add in the user-specified options
		foreach ($available_opts as $opt_name => $opt_switch) {
			$attribute = $this->attr($opt_name);
			if (in_array($opt_name, Array('politeness', 'spelling_threshold')) && $attribute == '0') {
				// Special case for politeness to work on 0, ie. we dont want to be polite ;-)
				$options[] = $opt_switch.'0';
			} else if ($opt_name == 'orgname') {
				// Special case for orgname to escape the option
				if (!empty($attribute)) {
					$options[] = '-O'.escapeshellarg($attribute);
				}//end if
			} else {
				if (!empty($attribute)) {
					if (in_array($opt_name, Array('politeness', 'spelling_threshold'))) {
						$options[] = $opt_switch.$attribute;
					} else {
						$options[] = $opt_switch;
					}//end if
				}//end if
			}//end if
		}//end foreach

		return $options;

	}//end getIndexingOptions()


// --- HELPER FUNCTIONS --- //


	/**
	* Read a configuration file
	*
	* @param string		$config		The file to read
	* @param string		$type		The type of configuration to read
	* @param string		$delimiter	The delimiter of each line
	* @param boolean	$system		Used currently for metamap, to pass in the attribute
	*
	* @return array
	* @access public
	*/
	function readConfiguration($config, $type='thesaurus', $delimiter='=', $system=FALSE)
	{
		$data = Array();
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$contents = file_to_string($config);
		$lines = explode("\n", $contents);
		switch ($type) {
			case 'feature':
				foreach ($lines as $line) {
					if (strpos($line, $delimiter) !== FALSE) {
						// Found a config line
						$line_contents = explode($delimiter, $line);
						// Need to figure out what is set
						$number_elements = count($line_contents);
						$trigger = NULL;
						$title = NULL;
						$desc = NULL;
						$url = NULL;
						$query = 'substring';
						if ($number_elements == 4) {
							// 4 Elements - trigger, title, desc, url
							$trigger = array_get_index($line_contents, 0, '');
							$title = array_get_index($line_contents, 1, '');
							$desc = array_get_index($line_contents, 2, '');
							$url = array_get_index($line_contents, 3, '');
						} else if ($number_elements == 3) {
							// 3 Elements - trigger, title, url
							$trigger = array_get_index($line_contents, 0, '');
							$title = array_get_index($line_contents, 1, '');
							$url = array_get_index($line_contents, 3, '');
						} else {
							// Guessing 2 elements - trigger, url
							$trigger = array_get_index($line_contents, 0, '');
							$url = array_get_index($line_contents, 3, '');
						}//end if

						// Needed to process *at least* a trigger and url
						if (!empty($trigger) && !empty($url)) {
							if (strpos($trigger, '~') === 0) {
								$trigger = substr($trigger, 1);
								$query = 'regex';
							} else if (strpos($trigger, '%') === 0) {
								$trigger = substr($trigger, 1);
								$query = 'term';
							} else if (strpos($trigger, '+') === 0) {
								$trigger = substr($trigger, 1);
								$query = 'exact';
							}//end if

							$data_line = Array(
											'query'		=> $query,
											'trigger'	=> $trigger,
											'url'		=> $url,
										 );
							if (!empty($title)) $data_line['title'] = $title;
							if (!empty($desc)) $data_line['desc'] = $desc;
							$data[] = $data_line;
						}//end if
					}//end if
				}//end foreach
			break;

			case 'fluster':
				foreach ($lines as $line) {
					$line = trim($line);
					if (!empty($line) && ((strpos($line, '#') !== 0) || (strpos($line, $delimiter) !== FALSE))) {
						$line_data = explode($delimiter, $line);
						if (isset($line_data[0]) && !empty($line_data[0])) {
							// These settings do not need to be saved as they are used exclusively by the search page
							$invalid = Array(
											'enable_fluster',
											'site.max_clusters',
											'type.max_clusters',
											'topic.max_clusters',
									   );
							if (!in_array($line_data[0], $invalid)) {
								$data[$line_data[0]] = $line_data[1];
							}//end if
						}//end if
					}//end if
				}//end foreach
			break;

			case 'metadata':
				foreach ($lines as $line) {
					$line = trim($line);
					if (!empty($line) && ((strpos($line, '#') !== 0) && (strpos($line, $delimiter) !== FALSE))) {
						$line_data = explode($delimiter, $line);
						if (!empty($line_data[0]) && isset($line_data[1]) && !empty($line_data[2])) {
							$alias = $line_data[0];
							$content = (($line_data[1] == '1') ? '1' : '0');
							$metatag = $line_data[2];
							$found_comment = strpos($metatag, '#');
							if ($found_comment !== FALSE) {
								$metatag = substr($metatag, 0, $found_comment);
							}//end if
							$data[] = Array(
										'alias'		=> $alias,
										'content'	=> $content,
										'metatag'	=> trim($metatag),
										'system'	=> TRUE,
									  );
						}//end if
					}//end if
				}//end foreach

				// This *has* to be in the entries for access_locks
				$data[] = Array(
							'alias'		=> 'S',
							'content'	=> '1',
							'metatag'	=> 'access_locks',
							'system'	=> TRUE,
						  );
			break;

			case 'thesaurus':
			default:
				foreach ($lines as $line) {
					if (strpos($line, $delimiter) !== FALSE) {
						// Found a config line
						$line_contents = explode($delimiter, $line);
						$query = array_get_index($line_contents, 0, '%');
						$term = array_get_index($line_contents, 1, '');
						$expansion = array_get_index($line_contents, 2, '');
						if ($query == '+') {
							$query_exp = 'whole';
						} else if ($query == '~') {
							$query_exp = 'regex';
						} else if ($query == '%') {
							$query_exp = 'term';
						}

						if (!empty($term) && !empty($expansion)) {
							// No point having a nothing there
							$data[] = Array(
										'query'		=> $query_exp,
										'term'		=> $term,
										'expansion'	=> $expansion,
									  );
						}//end if
					}//end if
				}//end foreach
			break;
		}//end switch

		return $data;

	}//end readConfiguration()


	/**
	* Write a configuration file
	*
	* @param string	$config		The file to write
	* @param array	$data		The data to write
	* @param string	$type		The type of configuration to write
	* @param string	$delimiter	The delimiter of each line
	*
	* @return array
	* @access public
	*/
	function writeConfiguration($config, $data=Array(), $type='thesaurus', $delimiter='=')
	{
		$status = FALSE;
		$contents = '';
		switch ($type) {
			case 'feature':
				foreach ($data as $line) {
					$query = array_get_index($line, 'query', 'substring');
					$trigger = array_get_index($line, 'trigger', '');
					$title = array_get_index($line, 'title', '');
					$desc = array_get_index($line, 'desc', '');
					$url = array_get_index($line, 'url', '');
					$pre_line = '';
					if ($query == 'exact') {
						$pre_line = '+';
					} else if ($query == 'term') {
						$pre_line = '%';
					} else if ($query == 'regex') {
						$pre_line = '~';
					}//end if

					if (!empty($trigger) && !empty($url)) {
						// At bare minimum, we need a trigger and url to write
						$contents .= $pre_line.$trigger.$delimiter;
						if (!empty($title)) $contents .= $title.$delimiter;
						if (!empty($desc)) $contents .= $desc.$delimiter;
						$contents .= $url."\n";
					}//end if
				}//end foreach
			break;

			case 'fluster':
				$contents .= "#\n";
				$contents .= "# Fluster Configuration\n";
				$contents .= "#\n";
				// These settings do not need to be saved as they are used exclusively by the search page
				$invalid = Array(
								'enable_fluster',
								'site.max_clusters',
								'type.max_clusters',
								'topic.max_clusters',
						   );
				ksort($data);
				foreach ($data as $switch => $parameter) {
					if (in_array($switch, $invalid)) continue;
					$contents .= $switch.$delimiter.$parameter."\n";
				}//end foreach
			break;

			case 'metadata':
				$contents .= "#\n";
				$contents .= "# Metadata Mapping Configuration\n";
				$contents .= "#\n";
				foreach ($data as $line) {
					$alias = array_get_index($line, 'alias', '');
					$field = array_get_index($line, 'content', 0);
					$metag = array_get_index($line, 'metatag', '');
					if (!empty($metag) || $alias != 'S') {
						$contents .= $alias.$delimiter.$field.$delimiter.$metag."\n";
					}//end if
				}//end foreach

				// Finally, add in the access locks
				$contents .= 'S,4,access_locks'."\n";
			break;

			case 'thesaurus':
			default:
				$contents = "PADRE Thesaurus Version: 2\n";
				foreach ($data as $line) {
					$query = array_get_index($line, 'query', '%');
					$term = array_get_index($line, 'term', '');
					$expansion = array_get_index($line, 'expansion', '');

					if (!empty($term) && !empty($expansion)) {
						$contents .= $query.$delimiter.$term.$delimiter.$expansion."\n";
					}//end if
				}//end foreach
			break;
		}//end switch

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$status = string_to_file($contents, $config);

		return $status;

	}//end writeConfiguration()


	/**
	* Run a command line program
	*
	* @param string	$command		The command to run
	* @param string	&$output		The output to return
	* @param string	$input			The input to pass into $command (NULL if none)
	* @param string	$log_file		Where to log
	* @param string	$cwd			The current working directory (NULL to use current)
	* @param array	$environment	Any environment settings to use at run time
	*
	* @return integer
	* @access public
	*/
	function runCommand($command, &$output, $input=NULL, $log_file=NULL, $cwd=NULL, $environment=Array())
	{
		// Default variables and environment settings
		$fm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('funnelback_manager');
		$search_home = $this->getFunnelbackPath();
		$environment['SEARCH_HOME'] = $search_home;
		if (!isset($environment['SITE_SEARCH_ROOT']) && !is_null($cwd)) {
			$environment['SITE_SEARCH_ROOT'] = $cwd;
		}//end if
		if (is_null($log_file)) {
			$log = Array(
						'pipe',
						'w',
				   );
		} else {
			$log = Array(
						'file',
						$log_file,
						'a',
				   );
		}//end if

		$status = FALSE;
		$spec = Array(
					0	=> Array(
								'pipe',
								'r',
						   ),					// stdin is a pipe that the child will read from
					1	=> $log,				// stdout is a pipe that the child will write to
					2	=> $log,				// stderr is a file to write to
				);

		$process = proc_open($command, $spec, $pipes, $cwd, $environment);

		if (is_resource($process)) {
			// Add the input if requested
			if (!is_null($input)) {
				fwrite($pipes[0], $input);
				fclose($pipes[0]);
			}

			if (is_null($log_file)) {
				// Get the output
				$output .= stream_get_contents($pipes[1]);
				fclose($pipes[1]);

				// Find the errors ... do NOT throw to output
				$errors = stream_get_contents($pipes[2]);
				if (!empty($errors)) {
					error_log("Funnelback Command failed, see funnelback.log for additional detail.");
					log_write($errors, 'funnelback');
				}//end if
				fclose($pipes[2]);
			}//end if

			// Finally, get the status of this command
			$status = proc_close($process);
		}//end if

		return $status;

	}//end runCommand()


	/**
	 * Encode a key or lock
	 * Funnelback strongly advise to stick to ASCII characters and not use control characters.
	 * More specifically, Word Smart Quotes are removed and ASCII single and double quote have no meaning except for escaping.
	 * Furthermore, single and double quote can be used but messy, so we are just going to escape them as HTML.
	 *
	 * @param string	$text	The text to encode
	 *
	 * @return string
	 * @access public
	 */
	public function encodeText($text)
	{
		// Strip disallowed characters (0-0x20 and 0x7F-0xA0)
		$pattern = Array();
		for ($i=0; $i<32; $i++) {
			$pattern[] = chr($i);
		}//end for
		for ($i=127; $i<161; $i++) {
			$pattern[] = chr($i);
		}//end for

		foreach ($pattern as $patt) {
			if (SQ_CONF_DEFAULT_CHARACTER_SET == 'utf-8' && function_exists('mb_substr') && function_exists('mb_strlen') && function_exists('mb_strpos')) {
				// Bug #4240 - when dealing with 2 byte utf8 characters, str_replace just ain't up to scratch
				$patt = utf8_encode($patt);
				$replace = '';
				$pos = mb_strpos($text, $patt);
				while ($pos !== FALSE) {
					$text = mb_substr($text, 0, $pos).$replace.mb_substr($text, $pos + mb_strlen($patt));
					$pos = mb_strpos($text, $patt, $pos+mb_strlen($replace));
				}
			} else {
				$text = str_replace($patt, '', $text);
			}//end if
		}

		// Remove Quote marks
		$text = str_replace('"', '', $text);
		$text = str_replace("'", '', $text);

		// Remove spaces (character 0x20)
		$text = str_replace(' ', '', $text);
		$text = str_replace('&nbsp;', '', $text);

		return $text;

	}//end encodeText()


	/**
	 * Decode a key or lock
	 * Reverse the effect of encoding
	 *
	 * @param string	$text	The text to decode
	 *
	 * @return string
	 * @access public
	 */
	public function decodeText($text)
	{
		// Reinstate Quote marks
		$text = str_replace('&ldquo;', '"', $text);
		$text = str_replace('&lsquo;', "'", $text);

		// Reinstate Space
		$text = str_replace('&nbsp;', ' ', $text);

		return $text;

	}//end decodeText()


	/**
	* Get the metadata mapping aliases
	*
	* @return array
	* @access public
	*/
	public function getMetadataAliases()
	{
		return Array(
					// ''=> 'Anywhere',						// DISABLED until tested properly
					'a'	=> sprintf(translate('Metadata Class %s'), 'a'),		// Author

					'b'	=> sprintf(translate('Metadata Class %s'), 'b'),		// Rights

					'c'	=> sprintf(translate('Metadata Class %s'), 'c'),		// Description

					'd'	=> sprintf(translate('Metadata Class %s'), 'd'),		// Date

					'e'	=> sprintf(translate('Metadata Class %s'), 'e'),		// Type

					'f'	=> sprintf(translate('Metadata Class %s'), 'f'),		// Format

					'g'	=> sprintf(translate('Metadata Class %s'), 'g'),		// Relation

					'h'	=> sprintf(translate('Metadata Class %s'), 'h'),		// Outgoing Links

					'i'	=> sprintf(translate('Metadata Class %s'), 'i'),		// Images

					'j'	=> sprintf(translate('Metadata Class %s'), 'j'),		// Availability

					'k'	=> sprintf(translate('Metadata Class %s'), 'k'),		// Anchor Text

					'l'	=> sprintf(translate('Metadata Class %s'), 'l'),		// Language

					'm'	=> sprintf(translate('Metadata Class %s'), 'm'),		// Mailto references

					'n'	=> sprintf(translate('Metadata Class %s'), 'n'),		// Source

					'o'	=> sprintf(translate('Metadata Class %s'), 'o'),		// Coverage

					'p'	=> sprintf(translate('Metadata Class %s'), 'p'),		// Publisher

					'q'	=> sprintf(translate('Metadata Class %s'), 'q'),		// Function

					'r'	=> sprintf(translate('Metadata Class %s'), 'r'),		// Recipients

					's'	=> sprintf(translate('Metadata Class %s'), 's'),		// Subject/Keywords

					't'	=> sprintf(translate('Metadata Class %s'), 't'),		// Title

					'u'	=> sprintf(translate('Metadata Class %s'), 'u'),		// Hostname part of URL

					'v'	=> sprintf(translate('Metadata Class %s'), 'v'),		// Filename part of URL

					'w'	=> sprintf(translate('Metadata Class %s'), 'w'),

					'x'	=> sprintf(translate('Metadata Class %s'), 'x'),

					'y'	=> sprintf(translate('Metadata Class %s'), 'y'),

					'z'	=> sprintf(translate('Metadata Class %s'), 'z'),

					'A'	=> sprintf(translate('Metadata Class %s'), 'A'),

					'B'	=> sprintf(translate('Metadata Class %s'), 'B'),

					'C'	=> sprintf(translate('Metadata Class %s'), 'C'),

					'D'	=> sprintf(translate('Metadata Class %s'), 'D'),

					'E'	=> sprintf(translate('Metadata Class %s'), 'E'),

					'F'	=> sprintf(translate('Metadata Class %s'), 'F'),

					'G'	=> sprintf(translate('Metadata Class %s'), 'G'),

					'H'	=> sprintf(translate('Metadata Class %s'), 'H'),

					'I'	=> sprintf(translate('Metadata Class %s'), 'I'),

					'J'	=> sprintf(translate('Metadata Class %s'), 'J'),

					'K'	=> sprintf(translate('Metadata Class %s'), 'K'),		// Click Data

					'L'	=> sprintf(translate('Metadata Class %s'), 'L'),

					'M'	=> sprintf(translate('Metadata Class %s'), 'M'),

					'N'	=> sprintf(translate('Metadata Class %s'), 'N'),

					'O'	=> sprintf(translate('Metadata Class %s'), 'O'),

					'P'	=> sprintf(translate('Metadata Class %s'), 'P'),

					'Q'	=> sprintf(translate('Metadata Class %s'), 'Q'),

					'R'	=> sprintf(translate('Metadata Class %s'), 'R'),

					'S'	=> sprintf(translate('Metadata Class %s'), 'S'),		// Squiz Access keys

					'T'	=> sprintf(translate('Metadata Class %s'), 'T'),

					'U'	=> sprintf(translate('Metadata Class %s'), 'U'),

					'V'	=> sprintf(translate('Metadata Class %s'), 'V'),

					'W'	=> sprintf(translate('Metadata Class %s'), 'W'),

					'X'	=> sprintf(translate('Metadata Class %s'), 'X'),

					'Y'	=> sprintf(translate('Metadata Class %s'), 'Y'),

					'Z'	=> sprintf(translate('Metadata Class %s'), 'Z'),

					'0'	=> sprintf(translate('Metadata Class %s'), '0'),

					'1'	=> sprintf(translate('Metadata Class %s'), '1'),

					'2'	=> sprintf(translate('Metadata Class %s'), '2'),

					'3'	=> sprintf(translate('Metadata Class %s'), '3'),

					'4'	=> sprintf(translate('Metadata Class %s'), '4'),

					'5'	=> sprintf(translate('Metadata Class %s'), '5'),

					'6'	=> sprintf(translate('Metadata Class %s'), '6'),

					'7'	=> sprintf(translate('Metadata Class %s'), '7'),

					'8'	=> sprintf(translate('Metadata Class %s'), '8'),

					'9'	=> sprintf(translate('Metadata Class %s'), '9'),

			   );

	}//end getMetadataAliases()


	/**
	* Check that funnelback is installed
	*
	* @return boolean
	* @access public
	*/
	public function getFunnelbackPath()
	{
		return '/opt/funnelback-oem';

	}//end getFunnelbackPath()


	/**
	 * Return the keyword to perform a NULL query search
	 * (Bascially, a long gobbly-gook word with a NOT operator)
	 *
	 * @return string
	 * @access public
	 */
	public function getNullQueryKeyword()
	{
		return '!NOdfadksjflTERMSdsjdskfSUBMITTED';

	}//end getNullQueryKeyword()


	/**
	* Check that funnelback is installed
	*
	* @return boolean
	* @access public
	*/
	public function checkInstalled()
	{
		$installed = TRUE;
		$search_dir = $this->getFunnelbackPath();
		foreach ($this->getRequiredFiles(TRUE) as $file) {
			if (!file_exists($search_dir.'/'.$file) && !is_readable($search_dir.'/'.$file)) {
				$installed = FALSE;
			}//end if
		}//end foreach

		return $installed;

	}//end checkInstalled()


	/**
	* Get the required files
	*
	* @param boolean	$path	Include the relative path in the results
	*
	* @return array
	* @access public
	*/
	public function getRequiredFiles($path=FALSE)
	{
		$indexer  = (($path) ? 'bin/' : '').'padre-iw';
		$search   = (($path) ? 'bin/' : '').'padre-sw';
		$fl       = (($path) ? 'bin/' : '').'padre-fl';
		$annie    = (($path) ? 'bin/' : '').'annie-a';
		$spelling = (($path) ? 'bin/' : '').'build_spelling_index';
		$click    = (($path) ? 'bin/' : '').'clix2anx';
		$config_xml     = (($path) ? 'conf/' : '').'xml.cfg';
		$config_metamap = (($path) ? 'conf/' : '').'metamap.cfg';
		$config_fluster = (($path) ? 'conf/' : '').'contextual_navigation.cfg';
		$licence        = (($path) ? 'VERSION/' : '').'squiz.lic';
		$files = Array(
					$indexer,
					$search,
					$fl,
					$annie,
					$spelling,
					$click,
					$config_xml,
					$config_metamap,
					$config_fluster,
					$licence,
				 );

		return $files;

	}//end getRequiredFiles()


}//end class

?>
