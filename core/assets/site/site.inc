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
* @package Resolve::packages::__core__
*/
class Site extends Folder
{

	/**
	* Constructor
	*
	* @param int    $assetid      the asset id to be loaded
	*
	*/
	function Site($assetid=0) 
	{
		$this->Folder($assetid);
	}

	/*
	* returns the urls for this site
	*
	* @see Asset::updateWebURLs()
	*/
	function getWebPaths($url=false) 
	{
		// because we never have paths, we can only return the urls
		return Array();
	}// end getWebPaths()

	/*
	* Attempts to add a url to this site, site's don't have paths, only URLs
	*
	* @param string	$url	the url to assign to this site
	*
	* @return boolean
	* @access public
	* @see Asset::updateWebURLs()
	*/
	function addWebPath($url) 
	{
		return false;
	}// end addWebPath()

	/*
	* Attempts to delete a path to this asset
	*
	* @param string	$path	the path to add to this asset's list
	*
	* @return boolean
	* @access public
	* @see Asset::updateWebURLs()
	*/
	function deleteWebPath($url) 
	{
		return false;
	}// end deleteWebPath()

	/**
	* returns a list of urls that point to this asset
	*
	* @param boolean	$inc_urlid	return the root urlid with the url
	*
	* @return Array()
	* @access public
	*/
	function getWebURLs($inc_urlid=false) 
	{
		$urls = $this->getSiteURLs();
		$ret_val = Array();
		if ($inc_urlid) {
			for(reset($urls); null !== ($urlid = key($urls)); next($urls)) {
				$ret_val[] = Array('url' => $urls[$urlid]['url'],
									'urlid' => $urlid
									);
			}
		} else {
			for(reset($urls); null !== ($urlid = key($urls)); next($urls)) {
				$ret_val[] = $urls[$urlid]['url'];
			}
		}

		return $ret_val;

	}// end getWebURLs()

	/**
	* returns a list of urls that point to this asset
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
					WHERE assetid = '.$GLOBALS['SQ_RESOLVE']->db->quote($this->id);

			$result = $GLOBALS['SQ_RESOLVE']->db->getAssoc($sql, true, array(), DB_FETCHMODE_ASSOC); // need all this because of DB API
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				$this->_tmp['urls_info'] = Array();
			} else {
				$this->_tmp['urls_info'] = $result;

			}// end if

		}// end if

		return $this->_tmp['urls_info'];

	}// end getSiteURLs()

	/*
	* Attempts to add a url to this site, site's don't have paths, only URLs
	*
	* @param Array $save_urls		an Array($urlid => Array(url => '', http => boolean, https => boolean)
	*
	* @return boolean
	* @access public
	*/
	function saveWebURLs($save_urls) 
	{

		$db = &$GLOBALS['SQ_RESOLVE']->db;
		$db->query('BEGIN');

		$processed_urlids = Array();
		$current_urls = $this->getSiteURLs();
		for(reset($save_urls); null !== ($save_urlid = key($save_urls)); next($save_urls)) {

			$curr_urlid = $save_urlid;

			// remove any trailing slashes
			$save_urls[$save_urlid]['url'] = strtolower(preg_replace('/\/+$/', '', $save_urls[$save_urlid]['url']));

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
				$asset = &$GLOBALS['SQ_RESOLVE']->am->getAssetFromURL($save_urls[$save_urlid]['url'], '', true, true);
				if (!is_null($asset)) {
					trigger_error('URL "'.$save_urls[$save_urlid]['url'].'" is already in use by "'.$asset->name.'"', E_USER_WARNING);
					continue;
				}
			}
			// are we an existing or an new one
			if ($save_urlid) {
				$sql = 'UPDATE sq_asset_url
						SET url   = '.$db->quote($save_urls[$save_urlid]['url']).',
							http  = '.$db->quote($save_urls[$save_urlid]['http']).',
							https = '.$db->quote($save_urls[$save_urlid]['https']).'
						WHERE urlid   = '.$db->quote($save_urlid).'
						  AND assetid = '.$db->quote($this->id);
				$result = $db->query($sql);
				if (DB::isError($result)) {
					$db->query('ROLLBACK');
					trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
					return false;
				}// end if

			} else {

				$curr_urlid = $db->nextId('sq_sequence_asset_url');
				if (DB::isError($curr_urlid)) {
					$db->query('ROLLBACK');
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
					$db->query('ROLLBACK');
					trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
					return false;
				}
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
				$db->query('ROLLBACK');
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
				return false;
			}

		}

		unset($this->_tmp['urls_info']);

		if ($this->_updateWebURLs()) {
			$db->query('COMMIT');
			return true;
		} else {
			$db->query('ROLLBACK');
			return false;
		}

	}// end saveWebURLs()
	
	/**
	* Fn that does the real processing of updateWebURLs(), separated to allow for the DB transaction stuff
	* Will be called by parent assets recursively on their kids so not 'really' private
	* and by the addWebURLs() and deleteWebURLs() so that their transaction calls can wrap around it
	*
	* @return boolean
	* @access private
	* @see Asset::_updateWebURLs();
	*/
	function _updateWebURLs() 
	{
		$urls = $this->getSiteURLs();
		$db = &$GLOBALS['SQ_RESOLVE']->db;

		// Remove all the old URLs
		$sql = 'DELETE FROM sq_asset_url_lookup
				WHERE assetid = '.$db->quote($this->id);
		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			return false;
		}

		// if we have paths then do some url inserting
		for(reset($urls); null !== ($urlid = key($urls)); next($urls)) {
			$sql = 'INSERT INTO sq_asset_url_lookup
					(url, assetid, root_urlid)
					VALUES
					('.$db->quote($urls[$urlid]['url']).', '.$db->quote($this->id).', '.$db->quote($urlid).')';
			$result = $db->query($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
				return false;
			}
		}// end for urls

		// recusively call the kids update
		$kids = $this->getLinks(SQ_LINK_NAV, '', 'S', 'major');
		for($i = 0; $i < count($kids); $i++) {
			$kid = &$GLOBALS['SQ_RESOLVE']->am->getAsset($kids[$i]['minorid'], $kids[$i]['type_code']);
			if (is_null($kid)) continue;
			if (!$kid->_updateWebURLs()) return false;
		}// end for kids

		return true;

	}// end _updateWebURLs()

}//end class
?>
