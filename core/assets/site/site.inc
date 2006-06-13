<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: site.inc,v 1.91 2006/06/13 06:45:09 sdanis Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';

/**
* Site
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.91 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Site extends Folder
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Site($assetid=0)
	{
		$this->Folder($assetid);

	}//end constructor


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return array
	* @access public
	*/
	function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['attr_links'] = ($lock_types['attributes'] | $lock_types['links']);
		return $lock_types;

	}//end lockTypes()


	/**
	* Returns the urls for this site
	*
	* @param boolean	$url	Url
	*
	* @return array
	* @access public
	* @see Asset::updateLookups()
	*/
	function getWebPaths($url=FALSE)
	{
		// because we never have paths, we can only return the urls
		return Array();

	}//end getWebPaths()


	/**
	* Attempts to add a url to this site, site's don't have paths, only URLs
	*
	* @param string	$url	the url to assign to this site
	*
	* @return boolean
	* @access public
	* @see Asset::updateLookups()
	*/
	function addWebPath($url)
	{
		return FALSE;

	}//end addWebPath()


	/**
	* Attempts to delete a path to this asset
	*
	* @param string	$url	the path to add to this asset's list
	*
	* @return boolean
	* @access public
	* @see Asset::updateLookups()
	*/
	function deleteWebPath($url)
	{
		return FALSE;

	}//end deleteWebPath()


	/**
	* Delete this asset from the trash
	*
	* Note that the asset <i>MUST</i> be in the trash to delete it <i>FROM</i> the trash.
	*
	* @param boolean	$release_lock	should we realease the lock after deleting
	*
	* @return boolean
	* @access public
	*/
	function delete($release_lock=TRUE)
	{
		$this->_tmp['deleting_site'] = TRUE;
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!$this->saveWebURLs(Array())) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		if (!parent::delete($release_lock)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		$this->_tmp['deleting_site'] = FALSE;

		return TRUE;

	}//end delete()


	/**
	* Returns a list of urls that point to this asset
	*
	* @return array
	* @access public
	*/
	function getSiteURLs()
	{
		if (!isset($this->_tmp['urls_info'])) {
			$sql = 'SELECT urlid, url, http, https
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_url
					';
			$where = 'assetid = '.$GLOBALS['SQ_SYSTEM']->db->quote($this->id);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			$sql .= $where.'ORDER BY urlid';

			$result = $GLOBALS['SQ_SYSTEM']->db->getAssoc($sql, TRUE, Array(), DB_FETCHMODE_ASSOC);
			assert_valid_db_result($result);
			$this->_tmp['urls_info'] = $result;

		}//end if not cached

		return $this->_tmp['urls_info'];

	}//end getSiteURLs()


	/**
	* Attempts to add a url to this site, site's don't have paths, only URLs
	*
	* @param array	$save_urls	an Array($urlid => Array(url => '', http => boolean, https => boolean)
	*
	* @return boolean
	* @access public
	*/
	function saveWebURLs($save_urls)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$updated = FALSE;
		$processed_urlids = Array();
		$current_urls = $this->getSiteURLs();
		$system_root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);

		for (reset($save_urls); NULL !== ($save_urlid = key($save_urls)); next($save_urls)) {

			$curr_urlid = $save_urlid;

			// strip any extra slashes
			$save_urls[$save_urlid]['url'] = strip_url($save_urls[$save_urlid]['url'], TRUE);

			//// Firstly let's make sure all the paths after the domain are valid ////
			$paths = explode('/', $save_urls[$save_urlid]['url']);
			// remove the domain
			array_shift($paths);

			foreach ($paths as $path) {
				$clean_paths = make_valid_web_paths(Array($path));
				$clean_path = (empty($clean_paths)) ? '' : $clean_paths[0];
				if ($path != $clean_path) {
					if ($clean_path != '') {
						trigger_localised_error('CORE0080', $save_urls[$save_urlid]['url'], $path, $clean_path, E_USER_WARNING);
					} else {
						trigger_localised_error('CORE0124', $save_urls[$save_urlid]['url'], $path, E_USER_WARNING);
					}

					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
			}

			//// Next let's see if this url based upon a root url ////

			$found_root_url = FALSE;
			foreach ($system_root_urls as $root_url) {
				if (substr($save_urls[$save_urlid]['url'], 0, strlen($root_url)) == $root_url) {
					$found_root_url = TRUE;
					break;
				}
			}

			if (!$found_root_url) {
				trigger_localised_error('CORE0082', E_USER_WARNING, $save_urls[$save_urlid]['url']);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			//// Next let's try and see if this URL is already in use by something ////

			$found = FALSE;
			for (reset($current_urls); NULL !== ($i = key($current_urls)); next($current_urls)) {
				if ($save_urls[$save_urlid]['url'] == $current_urls[$i]['url']) {
					$found = TRUE;
					break;
				}
			}

			// We don't have it ? does anyone else
			if (!$found) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL(NULL, $save_urls[$save_urlid]['url'], TRUE, TRUE);
				if (!is_null($asset)) {
					trigger_localised_error('CORE0081', E_USER_WARNING, $save_urls[$save_urlid]['url'], $asset->name, $asset->id);

					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
			}

			// are we an existing or an new one
			if (isset($current_urls[$save_urlid])) {
				if ($current_urls[$save_urlid]['url']   != $save_urls[$save_urlid]['url'] ||
					$current_urls[$save_urlid]['http']  != $save_urls[$save_urlid]['http'] ||
					$current_urls[$save_urlid]['https'] != $save_urls[$save_urlid]['https']) {

					$sql = 'UPDATE
								sq_ast_url
							SET
								url		= '.$db->quoteSmart($save_urls[$save_urlid]['url']).',
								http	= '.$db->quoteSmart($save_urls[$save_urlid]['http']).',
								https	= '.$db->quoteSmart($save_urls[$save_urlid]['https']).'
							WHERE
									urlid	= '.$db->quoteSmart($save_urlid).'
								AND	assetid	= '.$db->quoteSmart($this->id);

					$result = $db->query($sql);
					assert_valid_db_result($result);
					$updated = TRUE;
				}

			} else {

				$curr_urlid = $db->nextId('sq_ast_url');
				assert_valid_db_result($curr_urlid);

				$sql = 'INSERT INTO
							sq_ast_url
							(
								urlid,
								assetid,
								url,
								http,
								https
							)
							VALUES
							(
								'.$db->quoteSmart($curr_urlid).',
								'.$db->quoteSmart($this->id).',
								'.$db->quoteSmart($save_urls[$save_urlid]['url']).',
								'.$db->quoteSmart($save_urls[$save_urlid]['http']).',
								'.$db->quoteSmart($save_urls[$save_urlid]['https']).'
							)';

				$result = $db->query($sql);
				assert_valid_db_result($result);

				$updated = TRUE;

			}//end if save_urlid

			$current_urls[$curr_urlid] = $save_urls[$save_urlid];
			$processed_urlids[] = $curr_urlid;

		}//end for save_urls

		$delete_urlids = array_diff(array_keys($current_urls), $processed_urlids);

		foreach ($delete_urlids as $urlid) {
			// remove all the old URLs
			$sql = 'DELETE FROM
						sq_ast_url
					WHERE
						 urlid = '.$db->quote($urlid);

			$result = $db->query($sql);
			assert_valid_db_result($result);

			$updated = TRUE;
		}

		unset($this->_tmp['urls_info']);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end saveWebURLs()


	/**
	* Clean up any sloppy seconds that are left over before we cast this asset for good
	* In particular, remove out URLs and update the web paths of the kiddies
	*
	* @param string	$new_type_code	the new type_code that this asset is being cast to
	*
	* @return boolean
	* @access private
	*/
	function _morphCleanup($new_type_code)
	{
		$current_urls = $this->getSiteURLs();
		if (!empty($current_urls)) {
			if (!$this->saveWebURLs(Array())) return FALSE;
		}
		return TRUE;

	}//end _morphCleanup()


	/**
	* Called to force an update of this assets lookup information
	*
	* @return boolean
	* @access private
	* @see Asset::updateLookups(), Asset::saveWebPaths()
	*/
	function updateLookups()
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		unset($this->_tmp['lookups']);
		unset($this->_tmp['url']);
		unset($this->_tmp['href']);

		if (!isset($this->_tmp['deleting_site'])) {
			$this->_tmp['deleting_site'] = FALSE;
		}

		// Remove all the old URLs

		$sql = 'DELETE FROM
					sq_ast_lookup_value
				WHERE
					url IN
						(
							SELECT
								url
							FROM
								sq_ast_lookup l
							WHERE
								assetid = '.$db->quoteSmart($this->id).'
						) ';
					if ($this->_tmp['deleting_site'] == TRUE) {
						$sql .= 'AND (name LIKE '.$db->quoteSmart('design::%').' OR name LIKE '.$db->quoteSmart('paint_layout::%').')';
					} else {
						$sql .= 'AND	name LIKE '.$db->quoteSmart('design::%');
					}

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$sql = 'DELETE FROM
					sq_ast_lookup
				WHERE
					assetid = '.$db->quote($this->id);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$our_design_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, 'design', FALSE);
		$our_designs = Array();
		foreach ($our_design_links as $link) {
			if (preg_match('/^design::(system|user)::(.*)$/', $link['value'])) {
				$our_designs[$link['value']] = $link['minorid'];
			}
		}

		// look for overrides and overwrite the array in these places
		foreach ($our_design_links as $link) {
			if (preg_match('/^override::design::(system|user)::(.*)$/', $link['value'])) {
				$name = preg_replace('/^override::(.*)$/', '$1', $link['value']);
				$our_designs[$name] = $link['minorid'];
			}
		}

		$urls = $this->getSiteURLs();

		// if we have urls then do some url inserting
		for (reset($urls); NULL !== ($urlid = key($urls)); next($urls)) {

			$sql = 'INSERT INTO
						sq_ast_lookup
						(
							url,
							assetid,
							http,
							https,
							root_urlid
						)
						VALUES
						(
							'.$db->quoteSmart($urls[$urlid]['url']).',
							'.$db->quoteSmart($this->id).',
							'.$db->quoteSmart(($this->force_secure) ? '0' : $urls[$urlid]['http']).',
							'.$db->quoteSmart($urls[$urlid]['https']).',
							'.$db->quoteSmart($urlid).'
						)';

			$result = $db->query($sql);
			assert_valid_db_result($result);

			foreach ($our_designs as $name => $designid) {
				$sql = 'INSERT INTO
							sq_ast_lookup_value
							(
								url,
								name,
								value,
								inhd
							)
							VALUES
							(
								'.$db->quoteSmart($urls[$urlid]['url']).',
								'.$db->quoteSmart($name).',
								'.$db->quoteSmart($designid).',
								'.$db->quoteSmart('0').'
							)';

				$result = $db->query($sql);
				assert_valid_db_result($result);
			}
		}//end for urls

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end updateLookups()


	/**
	* Return a human readable description of the passed link
	*
	* @param int	$linkid	the link ID of the link to describe
	*
	* @return string
	* @access public
	*/
	function describeLink($linkid)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
		switch (strtolower($link['value'])) {
			case 'index' :
				return translate('core_site_index_page');
			break;
			case 'not_found' :
				return translate('core_site_not_found_page');
			break;
			default :
				return parent::describeLink($linkid);
			break;
		}

	}//end describeLink()


	/**
	* Prints out the Frontend for this asset
	* Ensures the design of the index page is printed
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		// check that the user has read access to the site
		if (!$this->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $this->name));
			return;
		}

		$index_page =& $this->getSpecialPage('index');
		if (is_null($index_page)) {
			if (SQ_IN_LIMBO) {
				// no index page is set & we are in LIMBO.
				$asset_url = strip_url($this->getURL(NULL, TRUE), TRUE);
				$design_name = $this->getCurrentDesignName();
				$result = $GLOBALS['SQ_SYSTEM']->am->getDesignFromURL($asset_url, $design_name);
				if (!empty($result)) {
					// if site has design then print LIMBO
					$this->paintAsset($this);
					return;
				}
			}
			trigger_localised_error('CORE0065', E_USER_NOTICE, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'), $this->name, $this->id);
			return;
		}

		$this->paintAsset($index_page);

	}//end printFrontend()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$index_page =& $this->getSpecialPage('index');
		if (!is_null($index_page)) $index_page->printBody();

		return;

	}//end printBody()


	/**
	* Paints an asset with the design that this site
	* has currently applied to it.
	*
	* If no design can be found fo
	* this site, the page will be displayed without the design
	*
	* @param object	&$asset	the asset to paint
	*
	* @return void
	* @access public
	*/
	function paintAsset(&$asset)
	{
		if (is_null($asset)) return;

		// check that the user has read access to the index page
		if (!$asset->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $this->name));
			return;
		}

		if ($asset->charset) {
			header("Content-type: text/html; charset=$this->charset");
		} else {
			header('Content-type: text/html; charset='.SQ_CONF_DEFAULT_CHARACTER_SET);
		}

		if ($asset->languages) {
			header("Content-language: $this->languages");
		}

		$url = strip_url($this->getURL(), TRUE);
		$asset_url = strip_url($asset->getURL(NULL, TRUE), TRUE);
		$design_name = $this->getCurrentDesignName();
		$result = $GLOBALS['SQ_SYSTEM']->am->getDesignFromURL($asset_url, $design_name);

		if (empty($result)) {
			$db =& $GLOBALS['SQ_SYSTEM']->db;
			// try and find the default design used by the asset we are painting (i.e. not our default design)
			$sql = 'SELECT lv.value as designid, a.type_code
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup l
							INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_value lv ON l.url = lv.url
							INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON lv.value = a.assetid
					';
			$where ='l.assetid  = '.$db->quote($asset->id).'
					  AND lv.url  LIKE '.$db->quote($url.'%').'
					  AND lv.name    = '.$db->quote('design::system::frontend');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lv');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
			$sql .= $where;
			$sql = $db->modifyLimitQuery($sql, 0, 1);

			$result = $db->getRow($sql);
			assert_valid_db_result($result);
		}

		global $ASSET_LINEAGE;
		$ASSET_LINEAGE = $GLOBALS['SQ_SYSTEM']->am->getLineageFromUrl(NULL, $asset_url);

		if ($result) {
			// we have found the design to use
			$design =& $GLOBALS['SQ_SYSTEM']->am->getAsset($result['designid'], $result['type_code']);
			if (SQ_IN_LIMBO) {
				$this->printFrontendAsset($this, $design);
			} else {
				// If there is an appopriate paint layout for the index page, tell the system to use it
				$layout_name = $this->getCurrentPaintLayoutName();
				$layout_id = $GLOBALS['SQ_SYSTEM']->am->getValueFromURL($asset_url, $layout_name);
				if ($layout_id) {
					$GLOBALS['SQ_SYSTEM']->setGlobalDefine('SQ_PAINT_LAYOUT_ID', $layout_id);
				}
				$this->printFrontendAsset($asset, $design);
			}
		} else {
			// we can't find a design, oh well let's just print out our body
			$asset->printBody();
		}

	}//end paintAsset()


	/**
	* Initialise the limbo (simple-edit) interface by painting to the limbo outputter
	*
	* Site puts its index page up for editing instead of itself
	*
	* @return void
	* @access public
	*/
	function initLimbo()
	{
		if (isset($_GET['limbo_assetid']) && $_GET['limbo_assetid'] == $this->id) {
			// print this site's limbo instead of the index page
			parent::initLimbo();
		} else {
			$index_page =& $this->getSpecialPage('index');
			if (is_null($index_page)) {
				parent::initLimbo();
				return;
			}

			$GLOBALS['SQ_SYSTEM']->backend->out->addFormActionGetVar('assetid', $index_page->id);
			$GLOBALS['SQ_SYSTEM']->backend->out->addHiddenField('backend_assetid', $index_page->id);
			$index_page->initLimbo();
		}

	}//end initLimbo()


	/**
	* Returns the link to the special page (index, not_found, forbidden) passed in
	*
	* @param string	$page	the page that the user wants
	*
	* @return array
	* @access public
	*/
	function getSpecialPageLink($page)
	{
		switch ($page) {
			case 'index'     :
			case 'not_found' :
			case 'forbidden' :
				return $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', TRUE, $page);
			break;

			default :
				trigger_localised_error('CORE0075', $page);
				return Array();
		}

	}//end getSpecialPageLink()


	/**
	* Returns the special page asset (index, not_found, forbidden)
	*
	* @param string	$page	the page that the user wants
	*
	* @return object
	* @access public
	*/
	function &getSpecialPage($page)
	{
		$link = $this->getSpecialPageLink($page);
		if (empty($link)) {
			$null = NULL;
			return $null;
		}

		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		return $asset;

	}//end getSpecialPage()


	/**
	* Returns the Site Network that this site exists in (if any)
	*
	* If this site is not in a Site Network, NULL will be returned.
	*
	* @return mixed object|NULL
	* @access public
	*/
	function &getSiteNetwork()
	{
		$null = NULL;

		$network_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_SIGNIFICANT, 'site_network', FALSE, 'minor');
		if (empty($network_links)) return $null;

		$site_network =& $GLOBALS['SQ_SYSTEM']->am->getAsset($network_links[0]['majorid'], $network_links[0]['major_type_code']);
		return $site_network;

	}//end getSiteNetwork()


	/**
	* Returns the primary URL of the Site Network in which this site exists (if any)
	*
	* If this site iss not in a Site Network, or the network has not been configured, an
	* empty string will be returned.
	*
	* @return string
	* @access public
	*/
	function getNetworkPrimaryURL()
	{
		$site_network =& $this->getSiteNetwork();
		if (is_null($site_network)) return '';
		return $site_network->getPrimaryURL();

	}//end getNetworkPrimaryURL()


}//end class

?>
