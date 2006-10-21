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
* $Id: remap_manager.inc,v 1.27 2006/10/21 03:18:17 amiller Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Remap_Manager
*
* Manages the remapping of old URLs to new URLs
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.27 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Remap_Manager extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Remap_Manager($assetid=0)
	{
		$this->Asset($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* Returns either the linkid or FALSE on failure
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	function create(&$link)
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
	function _getName($short_name=FALSE)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');

	}//end _getName()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		// added for root node
		$page_links[SQ_LINK_NOTICE]['asset'] = Array('card' => 'M', 'exclusive' => FALSE);
		return $page_links;

	}//end _getAllowedLinks()


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
	* returns FALSE always because we don't want to be able to clone a remap manager
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return FALSE;

	}//end canClone()


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
			case '404' :
				return translate('404_remap_description');
			break;
			default :
				return parent::describeLink($linkid);
			break;
		}

	}//end describeLink()


	/**
	* Add a URL remap to the system
	*
	* Returns 0 on failed insert, 1 on success, -1 if URL already exists
	*
	* @param string	$old_url	the broken URL
	* @param string	$new_url	the URL to redirect to
	* @param int	$expires	timestamp for when remap expires (NULL for no expiry)
	*
	* @return int
	* @access public
	*/
	function addRemapURL($old_url, $new_url, $expires=NULL)
	{
		$old_url = strip_url($old_url);
		$new_url = strip_url($new_url);

		// old url and new url must be strings, makes not sense otherwise
		if (!is_string($old_url) || !is_string($new_url)) {
			trigger_localised_error('CORE0093', E_USER_WARNING);
			return 0;
		}

		// there is no use remapping a URL to itself
		if (strcmp($old_url, $new_url) == 0) return 1;

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT remap_url
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_remap';
		$where = 'url = '.$db->quote($old_url);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$remap_urls = $db->getCol($sql.$where);
		assert_valid_db_result($remap_urls);

		if (in_array($new_url, $remap_urls)) return 1;

		if (count($remap_urls) > 0) {
			trigger_localised_error('CORE0024', E_USER_WARNING, $old_url);
			return -1;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// check if the new URL is already remapping to something else, and delete it
		$sql = 'DELETE FROM
					sq_ast_lookup_remap
				WHERE
					url = '.$db->quoteSmart($new_url);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$sql = 'INSERT INTO
					sq_ast_lookup_remap
					(
						url,
						remap_url,
						expires
					)
					VALUES
					(
						'.$db->quoteSmart($old_url).',
						'.$db->quoteSmart($new_url).',
						'.$db->quoteSmart((is_null($expires)) ? $expires : ts_iso8601($expires)).'
					)';

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return 1;

	}//end addRemapURL()



	/**
	* Get all remap URLs for an asset's current URLs
	*
	* @param object	&$asset	the asset to get the remaps for
	*
	* @return array
	* @access public
	*/
	function getRemapURLs(&$asset)
	{
		$null = NULL;
		if (is_null($asset)) return $null;
		
		$urls = $asset->getURLs();
		$remaps = Array();
		
		if (!empty($urls)) {

			$db =& $GLOBALS['SQ_SYSTEM']->db;
			
			$sql = 'SELECT url, expires
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_remap';
			$where = 'remap_url IN (';
			
			foreach ($urls as $url) {
				if ($url['http'] == 1) {
					$where .= $db->quote('http://'.$url['url']).', ';
				}
				if ($url['https'] == 1) {
					$where .= $db->quote('https://'.$url['url']).', ';
				}
				
			}//end for

			$where = substr($where, 0, strlen($where) - 2);
			$where .= ')';
			
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			
			$remaps = $db->getAll($sql.$where);
			assert_valid_db_result($remaps);
		
		}//end if
		
		return $remaps;
		
	
	}//end getRemapURLs


	/**
	* Remap all the URLs of an asset to a new URL
	*
	* @param string	$assetid	the ID of the asset we are adding remaps for
	* @param string	$new_url	the URL to redirect to
	* @param int	$expires	timestamp for when remap expires (NULL for no expiry)
	*
	* @return boolean
	* @access public
	*/
	function remapAllURLs($assetid, $new_url, $expires=NULL)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT url, http, https
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup';
		$where = 'assetid = '.$db->quote($assetid);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$urls = $db->getAll($sql.$where);
		assert_valid_db_result($urls);

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		foreach ($urls as $url_data) {
			if ($url_data['http']) {
				$retval = @$this->addRemapURL('http://'.$url_data['url'], $new_url, $expires);
				if ($retval === 0) {
					trigger_localised_error('CORE0039', E_USER_WARNING, $assetid, $new_url);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
			}
			if ($url_data['https']) {
				$retval = @$this->addRemapURL('https://'.$url_data['url'], $new_url, $expires);
				if ($retval === 0) {
					trigger_localised_error('CORE0039', E_USER_WARNING, $assetid, $new_url);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end remapAllURLs()


	/**
	* Attempts to redirect a url to its remapped url
	* If the remap manager is unsuccessful in doing so, it will return FALSE
	*
	* @param string	$protocol	the protocol that the url is using
	* @param string	$url		the url to remap from
	*
	* @return boolean
	* @access public
	*/
	function loadRemapFromURL($protocol=NULL, $url=NULL)
	{
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		if (is_null($protocol)) $protocol = current_protocol();
		if (is_null($url)) {
			$url = strip_url(current_url(FALSE, TRUE));
		}

		$url = $protocol.'://'.$url;
		$db  =& $GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT
					remap_url
				FROM
					'.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_remap';

		$where = 'url = '.$db->quote($url).'
				  AND (expires IS NULL OR expires > '.db_extras_todate($db, ts_iso8601(time())).')';
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$remap_url = $db->getOne($sql.$where);
		assert_valid_db_result($remap_url);

		// if we have a remap url, display it
		if (!empty($remap_url)) {
			header('HTTP/1.1 301 Moved Permanently');
			header('Location: '.$remap_url);

			return TRUE;
		}

		return FALSE;

	}//end loadRemapFromURL()


	/**
	* Returns the asset that has been chosen to
	* delegate 404 Not found requests
	*
	* @return mixed object|NULL
	* @access public
	*/
	function &getPageNotFoundAsset()
	{
		// searching for the root asset
		$link_to_404 = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', FALSE, '404');

		if (empty($link_to_404)) {
			$asset = NULL;
		} else {
			$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link_to_404['minorid']);
		}

		return $asset;

	}//end getPageNotFoundAsset()


	/**
	* Paints the page not found asset
	*
	* If a design can be found for the asset, it will
	* be painted as well, otherwise justs the contents of
	* the asset are painted.
	*
	* @return boolean
	* @access public
	*/
	function paintPageNotFoundRemap()
	{
		$asset =& $this->getPageNotFoundAsset();
		if (is_null($asset)) return FALSE;

		// send a 404 header
		header('HTTP/1.0 404 Not Found');

		$url = strip_url($this->getURL(), TRUE);
		$asset_url = strip_url($asset->getURL(NULL, TRUE), TRUE);

		global $ASSET_LINEAGE;
		$ASSET_LINEAGE = $GLOBALS['SQ_SYSTEM']->am->getLineageFromUrl(NULL, $asset_url);

		$design =& $this->getDesign($asset, $url);
		if (!is_null($design)) {
			$design->paint($asset);
			return TRUE;
		}
		// get the first design for this asset
		$design =& $this->getDesign($asset);
		if (!is_null($design)) {
			$design->paint($asset);
			return TRUE;
		}
		// give up, and just paint the asset
		// without a design
		$asset->printFrontend();

		return TRUE;

	}//end paintPageNotFoundRemap()


	/**
	* Returns the design that this asset currently has applied to it.
	*
	* Pass in NULL as $url to return the first design asset associated
	* with the asset's id
	*
	* @param object	&$asset	the asset to get the design for
	* @param string	$url	the url of the asset
	*
	* @return object
	* @access public
	*/
	function &getDesign(&$asset, $url=NULL)
	{
		$null = NULL;
		if (is_null($asset)) return $null;

		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT lv.value as designid, a.type_code
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup l
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_value lv ON l.url = lv.url
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON lv.value = a.assetid
				';
		$where = 'l.assetid  = '.$db->quote($asset->id).'
					AND lv.name = '.$db->quote('design::system::frontend');
		if (!is_null($url)) {
			$where .= ' AND lv.url  LIKE '.$db->quote($url.'%');
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lv');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$sql .= $where;
		$sql = $db->modifyLimitQuery($sql, 0, 1);

		$result = $db->getRow($sql);
		assert_valid_db_result($result);

		if ($result) {
			return $GLOBALS['SQ_SYSTEM']->am->getAsset($result['designid'], $result['type_code']);
		}
		return $null;

	}//end getDesign()


}//end class

?>