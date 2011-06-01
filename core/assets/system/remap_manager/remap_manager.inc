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
* $Id: remap_manager.inc,v 1.43 2009/01/12 02:26:59 akarelia Exp $
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
* @version $Revision: 1.43 $
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
	public function create(Array &$link)
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
	protected function _getName($short_name=FALSE)
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
	public function _getAllowedLinks()
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
	public function canDelete()
	{
		return FALSE;

	}//end canDelete()


	/**
	* returns FALSE always because we don't want to be able to clone a remap manager
	*
	* @return boolean
	* @access public
	*/
	public function canClone()
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
	public function describeLink($linkid)
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
	public function addRemapURL($old_url, $new_url, $expires=NULL, $never_delete='1')
	{
		// old url and new url must be strings, makes not sense otherwise
		if (!is_string($old_url) || !is_string($new_url)) {
			trigger_localised_error('CORE0093', E_USER_WARNING);
			return 0;
		}

		$old_url = strip_url($old_url);
		$new_url = strip_url($new_url);

		// there is no use remapping a URL to itself
		if (strcmp($old_url, $new_url) == 0) return 1;

		// can't remap to a blank URL
		if ($new_url === '') {
			trigger_localised_error('CORE0271', E_USER_WARNING);
			return 0;
		}

		$db = MatrixDAL::getDb();

		$sql = 'SELECT remap_url
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_remap';
		$where = 'url = :url';
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		try {
			$query = MatrixDAL::preparePdoQuery($sql.$where);
			MatrixDAL::bindValueToPdo($query, 'url', $old_url);
			$remap_urls = MatrixDAL::executePdoAssoc($query, 0);
		} catch (Exception $e) {
			throw new Exception('Unable to get remap URLs for URL: '.$old_url.' due to database error: '.$e->getMessage());
		}

		if (in_array($new_url, $remap_urls)) return 1;

		if (count($remap_urls) > 0) {
			trigger_localised_error('CORE0024', E_USER_WARNING, $old_url);
			return -1;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// check if the new URL is already remapping to something else, and delete it
		try {
			$bind_vars	= Array (
							'url'		=> $old_url,
							'remap_url'	=> $new_url,
							'expires'	=> (is_null($expires)) ? $expires : ts_iso8601($expires),
							'never_delete' => $never_delete,
						  );
			$type_codes = MatrixDAL::executeQuery('core', 'insertRemap', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to insert new remap from: '.$old_url.' to: '.$new_url.' due to database error: '. $e->getMessage());
		}

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
	public function getRemapURLs(Asset $asset)
	{
		$null = NULL;
		if (is_null($asset)) return $null;

		$urls = $asset->getURLs();
		$remaps = Array();

		if (!empty($urls)) {

			$bind_vars = Array();

			// This sub-query gets all URLs for this asset that have HTTP enabled
			// and adds http protocol to them to create a usable URL - then same
			// or similar for HTTPS enabled.
			$sql = 'SELECT url, expires
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_remap';
			$union_select = '(SELECT
								(\'http://\' || url)
							FROM sq_ast_lookup
							'.$GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause('http = \'1\'
								AND assetid = :assetid').')
							UNION
							(SELECT
								(\'https://\' || url)
							FROM sq_ast_lookup
							'.$GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause('https = \'1\'
								AND assetid = :assetid_1').')';

			$bind_vars['assetid'] = $asset->id;
			$bind_vars['assetid_1'] = $asset->id;
			$where = 'remap_url IN ('.$union_select.')';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

			try {
				$query  = MatrixDAL::preparePdoQuery($sql.$where);
				foreach ($bind_vars as $bind_var => $bind_value) {
					MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
				}
				$remaps = MatrixDAL::executePdoAssoc($query);
			} catch (Exception $e) {
				throw new Exception('Unable to get remap URLs for this asset: '.$asset->id.' due to database error: '.$e->getMessage());
			}

		}//end if

		return $remaps;


	}//end getRemapURLs()


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
	public function remapAllURLs($assetid, $new_url, $expires=NULL)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');

		try {
			$bind_vars = Array('assetid' => $assetid);
			$urls = MatrixDAL::executeAll('core', 'getLookupUrlsByAssetId', $bind_vars);
		} catch (DALException $e) {
			throw new Exception('Unable to remap all URLs; could not get existing lookups for asset ID #'.$assetid.' due to database error: '.$e->getMessage);
		}

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
	public function loadRemapFromURL($protocol=NULL, $url=NULL)
	{
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		if (is_null($protocol)) $protocol = current_protocol();
		if (is_null($url)) {
			$url = strip_url(current_url(FALSE, TRUE));
		}

		$url = $protocol.'://'.$url;

		try {
			$bind_vars = Array(
								'date_value' => ts_iso8601(time()),
							  );
			$time = MatrixDAL::executeOne('core', 'toDate', $bind_vars);
			$bind_vars = Array(
							'url'  => $url,
							'time' => $time,
						 );
			$remap_url = MatrixDAL::executeOne('remap_manager', 'getCurrentRemapURL', $bind_vars);
		} catch (DALException $e) {
			throw new Exception('Unable to get current remap URL for "'.$url.'" due to database error: '.$e->getMessage());
		}

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
	public function getPageNotFoundAsset()
	{
		// searching for the root asset
		$link_to_404 = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', FALSE, '404');

		if (empty($link_to_404)) {
			$asset = NULL;
		} else {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_to_404['minorid']);
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
	public function paintPageNotFoundRemap()
	{
		$asset = $this->getPageNotFoundAsset();
		if (is_null($asset)) return FALSE;

		// send a 404 header
		header('HTTP/1.0 404 Not Found');

		$url = strip_url($this->getURL(), TRUE);
		$asset_url = strip_url($asset->getURL(NULL, TRUE), TRUE);

		global $ASSET_LINEAGE;
		$ASSET_LINEAGE = $GLOBALS['SQ_SYSTEM']->am->getLineageFromUrl(NULL, $asset_url);

		$design = $this->getDesign($asset, $url);
		if (!is_null($design)) {
			$design->paint($asset);
			return TRUE;
		}
		// get the first design for this asset
		$design = $this->getDesign($asset);
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
	* @param Asset	$asset	the asset to get the design for
	* @param string	$url	the url of the asset
	*
	* @return object
	* @access public
	*/
	public function getDesign(Asset $asset, $url=NULL)
	{
		$null = NULL;
		if (is_null($asset)) return $null;

		$bind_vars = Array();

		// get original design or inherited design if no override design
		$sql = 'SELECT lv.value as designid, a.type_code
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup l
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_value lv ON ((l.url = lv.url) OR (l.url || \'/\' = lv.url))
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON lv.value = a.assetid
				';
		$where = 'l.assetid  = :assetid AND lv.name = :lookup_name';

		$bind_vars['assetid'] = $asset->id;
		$bind_vars['lookup_name'] = 'design::system::frontend';

		if (!is_null($url)) {
			$where .= ' AND lv.url  LIKE :url';
			$bind_vars['url'] = $url.'%';
		}

		$order_by = ' ORDER BY depth DESC';
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lv');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$sql .= $where.$order_by;
		$sql = db_extras_modify_limit_clause($sql, MatrixDAL::getDbType(), 1);

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			foreach ($bind_vars as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
			}
			$result = MatrixDAL::executePdoAll($query);
		} catch (DALException $e) {
			throw new Exception('Unable to select design of remapped asset "'.$asset->name.'" (#'.$asset->id.') due to database error: '.$e->getMessage());
		}

		if ($result) {
			return $GLOBALS['SQ_SYSTEM']->am->getAsset($result[0]['designid'], $result[0]['type_code']);
		}
		return $null;

	}//end getDesign()


}//end class

?>
