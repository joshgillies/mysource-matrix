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
		return parent::getWebPaths(true);
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
		// remove any trailing slashes
		$url = preg_replace('/\/+$/', '', $url);

		$urls = $this->getWebPaths(true);
		// does it exist already ?
		if (in_array($url, $urls)) return true;

		$db = &$GLOBALS['SQ_RESOLVE']->db;

		$sql = 'SELECT COUNT(*)
				FROM sq_asset_urls
				WHERE url = '.$db->quote($url);

		$result = $db->getOne($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_ERROR);
			return false;
		} 

		if ($result) {
			trigger_error('URL "'.$url.'" already in use', E_USER_WARNING);
			return false;
		}// end if

		// Remove all the old URLs
		$sql = 'INSERT INTO sq_asset_urls
				(url, assetid)
				VALUES
				('.$db->quote($url).', '.$db->quote($this->id).')';
		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_ERROR);
			return false;
		}

		return true;

	}// end addWebPath()

	/*
	* Attempts to add a path to this asset
	*
	* @param string	$path	the path to add to this asset's list
	*
	* @return boolean
	* @access public
	* @see Asset::updateWebURLs()
	*/
	function deleteWebPath($url) 
	{
		$urls = $this->getWebPaths(true);
		// doesn't it exist already ? dickhead
		if (!in_array($url, $urls)) return true;

		$db = &$GLOBALS['SQ_RESOLVE']->db;

		// Remove all the old URLs
		$sql = 'DELETE FROM sq_asset_urls
				WHERE url     = '.$db->quote($url).'
				  AND assetid = '.$db->quote($this->id);
		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_ERROR);
			return false;
		}

		return true;

	}// end deleteWebPath()
	
	/*
	* Called to force an update of this assets URLs
	*
	* @return Array()
	* @access public
	* @see Asset::updateWebURLs()
	*/
	function updateWebURLs() 
	{
		// because we all our urls are updated in addWebPath() and deleteWebPath() fns we don't
		// have to do anything for them here
		return true;

	}// end updateWebURLs()

}//end class
?>
