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
* $Id: site.inc,v 1.51.2.3 2004/03/02 18:35:51 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';

/**
* Site
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
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
	* @return Array()
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
	* @see Asset::updateLookups()
	*/
	function getWebPaths($url=false)
	{
		// because we never have paths, we can only return the urls
		return Array();

	}// end getWebPaths()


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
		return false;

	}// end addWebPath()


	/**
	* Attempts to delete a path to this asset
	*
	* @param string	$path	the path to add to this asset's list
	*
	* @return boolean
	* @access public
	* @see Asset::updateLookups()
	*/
	function deleteWebPath($url)
	{
		return false;

	}// end deleteWebPath()


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
	function delete($release_lock=true)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		
		if (!$this->saveWebURLs(Array())) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		if (!parent::delete($release_lock)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end delete()


	/**
	* Returns a list of urls that point to this asset
	*
	* @return Array()
	* @access public
	*/
	function getSiteURLs()
	{
		if (!isset($this->_tmp['urls_info'])) {
			$sql = 'SELECT urlid, url, http, https
					FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_url
					';
			$where = 'assetid = '.$GLOBALS['SQ_SYSTEM']->db->quote($this->id);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

			$result = $GLOBALS['SQ_SYSTEM']->db->getAssoc($sql.$where, true, array(), DB_FETCHMODE_ASSOC); // need all this because of DB API
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				$this->_tmp['urls_info'] = Array();
			} else {
				$this->_tmp['urls_info'] = $result;

			}// end if

		}// end if

		return $this->_tmp['urls_info'];

	}// end getSiteURLs()


	/**
	* Attempts to add a url to this site, site's don't have paths, only URLs
	*
	* @param Array $save_urls	an Array($urlid => Array(url => '', http => boolean, https => boolean)
	*
	* @return boolean
	* @access public
	*/
	function saveWebURLs($save_urls)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$updated = false;
		$processed_urlids = Array();
		$current_urls = $this->getSiteURLs();
		$system_root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
		for(reset($save_urls); null !== ($save_urlid = key($save_urls)); next($save_urls)) {

			$curr_urlid = $save_urlid;

			// strip any extra slashes
			$save_urls[$save_urlid]['url'] = strtolower(strip_url($save_urls[$save_urlid]['url'], true));

			//// Firstly let's make sure all the paths after the domain are valid ////
			$paths = explode('/', $save_urls[$save_urlid]['url']);
			// remove the domain
			array_shift($paths);
			foreach($paths as $path) {
				$clean_paths = make_valid_web_paths(Array($path));
				$clean_path = (empty($clean_paths)) ? '' : $clean_paths[0];
				if ($path != $clean_path) {
					trigger_error('URL "'.$save_urls[$save_urlid]['url'].'" contains a path "'.$path.'" which was deemed to be not valid. '.(($clean_path != '') ? 'A valid version is "'.$clean_path.'"' : ''), E_USER_WARNING);
					return false;
				}

			}// end foreach

			//// Next let's see if this url based upon a root url ////

			$found_root_url = false;
			foreach($system_root_urls as $root_url) {
				if (substr($save_urls[$save_urlid]['url'], 0, strlen($root_url)) == $root_url) {
					$found_root_url = true;
					break;
				}
			}

			if (!$found_root_url) {
				trigger_error('URL "'.$save_urls[$save_urlid]['url'].'" is not based upon an existing System Root URL', E_USER_WARNING);
				return false;
			}

			//// Next let's try and see if this URL is already in use by something ////

			$found = false;
			for(reset($current_urls); null !== ($i = key($current_urls)); next($current_urls)) {
				if ($save_urls[$save_urlid]['url'] == $current_urls[$i]['url']) {
					$found = true;
					break;
				}
			}// end for

			// We don't have it ? does anyone else
			if (!$found) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAssetFromURL(null, $save_urls[$save_urlid]['url'], true, true);
				if (!is_null($asset)) {
					trigger_error('URL "'.$save_urls[$save_urlid]['url'].'" is already in use by "'.$asset->name.'" [#'.$asset->id.']', E_USER_WARNING);
					continue;
				}
			}
			// are we an existing or an new one
			if ($save_urlid) {
				if ($current_urls[$save_urlid]['url']   != $save_urls[$save_urlid]['url'] ||
					$current_urls[$save_urlid]['http']  != $save_urls[$save_urlid]['http'] ||
					$current_urls[$save_urlid]['https'] != $save_urls[$save_urlid]['https']) {

					$where = 'urlid       = '.$db->quote($save_urlid).'
							  AND assetid = '.$db->quote($this->id);
					$values = Array('url'   => $db->quote($save_urls[$save_urlid]['url']),
									'http'  => $db->quote($save_urls[$save_urlid]['http']),
									'https' => $db->quote($save_urls[$save_urlid]['https'])
									);
					if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_url', $values, $where)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return false;
					}
					$updated = true;
				}

			} else {

				$curr_urlid = $db->nextId('sq_sequence_asset_url');
				if (DB::isError($curr_urlid)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					trigger_error($curr_urlid->getMessage().'<br/>'.$new_urlid->getUserInfo(), E_USER_ERROR);
				}

				$values = Array('urlid'   => $curr_urlid,
								'assetid' => $this->id,
								'url'     => $save_urls[$save_urlid]['url'],
								'http'    => $save_urls[$save_urlid]['http'],
								'https'   => $save_urls[$save_urlid]['https']
								);
				if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_url', $values)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}

				$updated = true;

			}// end if urlid

			$current_urls[$curr_urlid] = $save_urls[$save_urlid];
			$processed_urlids[] = $curr_urlid;

		}// end for

		$delete_urlids = array_diff(array_keys($current_urls), $processed_urlids);

		foreach($delete_urlids as $urlid) {
			// Remove all the old URLs
			$where_cond = 'urlid = '.$db->quote($urlid);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_url', $where_cond)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
			$updated = true;
		}

		unset($this->_tmp['urls_info']);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}// end saveWebURLs()


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
			if (!$this->saveWebURLs(Array())) return false;
		}
		return true;

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
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		unset($this->_tmp['lookups']);
		unset($this->_tmp['url']);
		unset($this->_tmp['href']);

		// Remove all the old URLs
		$subs = Array( 'SELECT url
						FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_lookup l
						'.$GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause('assetid = '.$db->quote($this->id), 'l'));

		$where = 'url IN (~SQ0~)';
		$where = db_extras_subquery($db, $where, $subs);
		if (DB::isError($where)) {
			trigger_error($where->getMessage().'<br/>'.$where->getUserInfo(), E_USER_ERROR);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_lookup_design', $where)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$where_cond = 'assetid = '.$db->quote($this->id);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_lookup', $where_cond)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$our_design_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_3, 'design', false);
		$our_designs = Array();
		foreach($our_design_links as $link) {
			if (preg_match('/^(system_design|user_design)::(.*)$/', $link['value'], $matches)) {
				$our_designs[$link['value']] = $link['minorid'];
			}// end if
		}// end foreach


		$urls = $this->getSiteURLs();
		// if we have urls then do some url inserting
		for(reset($urls); null !== ($urlid = key($urls)); next($urls)) {
			$values = Array(
							'url'               => $urls[$urlid]['url'],
							'assetid'           => $this->id,
							'http'              => ($this->force_secure) ? '0' : $urls[$urlid]['http'],
							'https'             => $urls[$urlid]['https'],
							'root_urlid'        => $urlid,
						);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_lookup', $values)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}



			foreach($our_designs as $name => $designid) {
				$values = Array(
								'url'      => $urls[$urlid]['url'],
								'name'     => $name,
								'designid' => $designid,
							);
				if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_lookup_design', $values)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			}// endforeach


		}// end for urls

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}// end updateLookups()


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
				return 'The index page for the site';
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
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to access <i>'.$this->name.'</i>');
			return;
		}

		$index_page = &$this->getSpecialPage('index');
		if (is_null($index_page)) {
			trigger_error('No Index Page Set for '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').' "'.$this->name.'" (Id #'.$this->id.')', E_USER_NOTICE);
			return;
		}

		// check that the user has read access to the index page
		if (!$index_page->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to access <i>'.$this->name.'</i>');
			return;
		}

		if($index_page->charset)   header("Content-type: text/html; charset=$this->charset");
		if($index_page->languages) header("Content-language: $this->languages");

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$url = strip_url(current_url(false, true));

		$sql = 'SELECT ld.designid, a.type_code
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_lookup l
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_lookup_design ld ON l.url = ld.url
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset a ON ld.designid = a.assetid
				';
		$where ='l.assetid  = '.$db->quote($index_page->id).'
				  AND ld.url  LIKE '.$db->quote($url.'%').'
				  AND ld.name    = '.$db->quote('system_design::frontend');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ld');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$sql .= $where.' LIMIT 1';
		$result = $db->getRow($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
		}

		// we have found the design to use
		if ($result) {
			$design = &$GLOBALS['SQ_SYSTEM']->am->getAsset($result['designid'], $result['type_code']);
			if (SQ_IN_LIMBO) $design->paint($this);
			else $design->paint($index_page);
		// shit we can't find a design, oh well let's just print out our body
		} else {
			$index_page->printBody();
		}

	}//end printFrontend()


	/**
	* Called by the design to print the body of this asset when in limbo
	*
	* Site puts its index page up for editing instead of itself
	*
	* @return void
	* @access public
	*/
	function printLimbo()
	{
		$index_page = &$this->getSpecialPage('index');
		if (is_null($index_page)) {
			trigger_error('No Index Page Set for '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').' "'.$this->name.'" (Id #'.$this->id.')', E_USER_NOTICE);
			return;
		}

		$GLOBALS['SQ_SYSTEM']->backend->out->addFormActionGetVar('assetid', $index_page->id);
		$GLOBALS['SQ_SYSTEM']->backend->out->addHiddenField('backend_assetid', $index_page->id);
		$index_page->printLimbo();

	}//end printLimbo()


	/**
	* Returns the link to the special page (index, not_found, forbidden) passed in
	*
	* @param string	$page	the page that the user wants
	*
	* @return Array
	* @access public
	*/
	function getSpecialPageLink($page)
	{
		switch($page) {
			case 'index' :
			case 'not_found' :
			case 'forbidden' :
				return $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_3, '', true, $page);
				break;

			default :
				trigger_error('Special Page "'.$page.'" unknown');
				return Array();

		}// end switch

	}//end getSpecialPageLink()


	/**
	* Returns the special page asset (index, not_found, forbidden)
	*
	* @param string	$page	the page that the user wants
	*
	* @return object Asset
	* @access public
	*/
	function &getSpecialPage($page)
	{
		$link = $this->getSpecialPageLink($page);
		if (empty($link)) {
			$null = null;
			return $null;
		}

		return $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);

	}//end getSpecialPage()


}//end class

?>
