<?php

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';

/**
* Site
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
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
	}


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
	* Returns a list of urls that point to this asset
	*
	* @param boolean	$all_info	whether to return all the URL information (ie ports & protocols) or not
	*
	* @return Array()
	* @access public
	*/
	function getSiteURLs()
	{
		if (!isset($this->_tmp['urls_info'])) {
			$sql = 'SELECT urlid, url, http, https
					FROM sq_asset_url
					WHERE assetid = '.$GLOBALS['SQ_SYSTEM']->db->quote($this->id);

			$result = $GLOBALS['SQ_SYSTEM']->db->getAssoc($sql, true, array(), DB_FETCHMODE_ASSOC); // need all this because of DB API
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
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$updated = false;
		$processed_urlids = Array();
		$current_urls = $this->getSiteURLs();
		for(reset($save_urls); null !== ($save_urlid = key($save_urls)); next($save_urls)) {

			$curr_urlid = $save_urlid;

			// strip any extra slashes
			$save_urls[$save_urlid]['url'] = strtolower(strip_url($save_urls[$save_urlid]['url'], true));

			//// Firstly let's try and see if this URL is already in use by something ////

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
					trigger_error('URL "'.$save_urls[$save_urlid]['url'].'" is already in use by "'.$asset->name.'"', E_USER_WARNING);
					continue;
				}
			}
			// are we an existing or an new one
			if ($save_urlid) {
				if ($current_urls[$save_urlid]['url']   != $save_urls[$save_urlid]['url'] ||
					$current_urls[$save_urlid]['http']  != $save_urls[$save_urlid]['http'] ||
					$current_urls[$save_urlid]['https'] != $save_urls[$save_urlid]['https']) {
					$sql = 'UPDATE sq_asset_url
							SET url   = '.$db->quote($save_urls[$save_urlid]['url']).',
								http  = '.$db->quote($save_urls[$save_urlid]['http']).',
								https = '.$db->quote($save_urls[$save_urlid]['https']).'
							WHERE urlid   = '.$db->quote($save_urlid).'
							  AND assetid = '.$db->quote($this->id);
					$result = $db->query($sql);
					if (DB::isError($result)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
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

				$sql = 'INSERT INTO sq_asset_url
						(urlid, assetid, url, http, https)
						VALUES
						('.$db->quote($curr_urlid).', '.$db->quote($this->id).',
						'.$db->quote($save_urls[$save_urlid]['url']).',
						'.$db->quote($save_urls[$save_urlid]['http']).',
						'.$db->quote($save_urls[$save_urlid]['https']).')';
				$result = $db->query($sql);
				if (DB::isError($result)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
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
			$sql = 'DELETE FROM sq_asset_url
					WHERE urlid = '.$db->quote($urlid);
			$result = $db->query($sql);
			if (DB::isError($result)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
				return false;
			}
			$updated = true;
		}

		unset($this->_tmp['urls_info']);

		if ($this->_updateLookups()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return $updated;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

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
	* Fn that does the real processing of updateLookups(), separated to allow for the DB transaction stuff
	* Will be called by parent assets recursively on their kids so not 'really' private
	* and by the saveWebURLs() so that their transaction calls can wrap around it
	*
	* @return boolean
	* @access private
	* @see Asset::_updateLookups(), Asset::updateLookups(), Asset::saveWebPaths()
	*/
	function _updateLookups()
	{
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		unset($this->_tmp['lookups']);
		unset($this->_tmp['url']);
		unset($this->_tmp['href']);

		// Remove all the old URLs

		$sql = 'DELETE FROM sq_asset_lookup
				WHERE assetid = '.$db->quote($this->id);
		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			return false;
		}

		$design_link = $this->getLink(SQ_LINK_TYPE_3, 'design', false, 'design');
		$designid = (is_null($design_link)) ? 0 : $design_link['minorid'];
		$login_design_link = $this->getLink(SQ_LINK_TYPE_3, 'design', false, 'login_design');
		$login_designid = (is_null($login_design_link)) ? 0 : $login_design_link['minorid'];

		$urls = $this->getSiteURLs();
		// if we have urls then do some url inserting
		for(reset($urls); null !== ($urlid = key($urls)); next($urls)) {
			$sql = 'INSERT INTO sq_asset_lookup
					(url, assetid, root_urlid, designid, login_designid)
					VALUES
					('.$db->quote($urls[$urlid]['url']).',
					'.$db->quote($this->id).',
					'.$db->quote($urlid).',
					'.$db->quote($designid).',
					'.$db->quote($login_designid).')';
			$result = $db->query($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
				return false;
			}
		}// end for urls

		// recusively call the kids update
		$kids = $this->getLinks(SQ_LINK_FRONTEND_NAV);
		for($i = 0; $i < count($kids); $i++) {
			$kid = &$GLOBALS['SQ_SYSTEM']->am->getAsset($kids[$i]['minorid'], $kids[$i]['type_code']);
			if (is_null($kid)) continue;
			if (!$kid->_updateLookups()) return false;
		}// end for kids

		return true;

	}// end _updateLookups()


	/**
	* Processes a backend submission from this asset, returns true if all OK
	*
	* @param object Backend_Outputter	$o
	* @param array(string)				&$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	function processBackend(&$o, &$link)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		if (parent::processBackend($o, $link)) {
			if (isset($this->_tmp['update_lookups']) && $this->_tmp['update_lookups']) {
				if (!$this->_updateLookups()) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;

		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}// end if

	}// end processBackend()


	/**
	* Prints out the Frontend for this asset
	* Extra checking here to make sure the user has read access to the index page
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		$index_page = &$this->getSpecialPage('index');
		if (is_null($index_page)) return;

		if (!$index_page->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to access <i>'.$this->name.'</i>');
			return;
		}

		parent::printFrontend();

	}//end printFrontend()


	/**
	* Called by the design to print the body of this asset
	* it's up to the kids to override
	*
	* @access public
	*/
	function printBody()
	{
		$index_page = &$this->getSpecialPage('index');
		if (is_null($index_page)) return;
		$index_page->printBody();

	}// end printBody()


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
				return $this->getLink(SQ_LINK_TYPE_3, '', true, $page);
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
		if (empty($link)) return null;

		return $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['type_code']);

	}//end getSpecialPage()


}//end class
?>
