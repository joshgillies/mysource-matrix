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
* $Id: remap_manager.inc,v 1.60 2013/10/18 03:57:25 ewang Exp $
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
* @version $Revision: 1.60 $
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
	protected function _getName($short_name=FALSE, $contextid=NULL)
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
	public function addRemapURL($old_url, $new_url, $expires=NULL, $never_delete=NULL, $auto=TRUE)
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

		if ($never_delete === NULL){
			$never_delete = ($this->attr('never_delete_remap_default') == FALSE) ? 0 : 1;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// check if the new URL is already remapping to something else, and delete it
		$sql = 'DELETE FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_remap';
		$where = ' WHERE url = :url';

		try {
			$query = MatrixDAL::preparePdoQuery($sql.$where);
			MatrixDAL::bindValueToPdo($query, 'url', $new_url);
			MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception('Unable to delete old remap URLs for URL: '.$new_url.' due to database error: '.$e->getMessage());
		}

		try {
			$bind_vars	= Array (
							'url'		=> $old_url,
							'remap_url'	=> $new_url,
							'expires'	=> (is_null($expires)) ? $expires : ts_iso8601($expires),
							'never_delete' => $never_delete,
							'auto_remap' => ($auto === TRUE) ? '1' : (($auto === FALSE) ? '0' : NULL),
						  );
			$type_codes = MatrixDAL::executeQuery('core', 'insertRemap', $bind_vars);
		} catch (Exception $e) {
			trigger_error('Unable to insert new remap from: '.$old_url.' to: '.$new_url.' due to database error: '. $e->getMessage(), E_USER_ERROR);
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
			$sql = 'SELECT url, remap_url, expires, never_delete, auto_remap
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

		$protocols = Array ( 'https' => FALSE, 'http' => FALSE );
		$never_delete = ($this->attr('never_delete_remap_default') == FALSE) ? 0 : 1;

		foreach ($urls as $url_data) {
			if ($url_data['http']) {
				$retval = @$this->addRemapURL('http://'.$url_data['url'], $new_url, $expires, $never_delete, FALSE);
				if ($retval === 0) {
					trigger_localised_error('CORE0039', E_USER_WARNING, $assetid, $new_url);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
				$protocols['http'] = TRUE;
			}
			if ($url_data['https']) {
				$retval = @$this->addRemapURL('https://'.$url_data['url'], $new_url, $expires, $never_delete, FALSE);
				if ($retval === 0) {
					trigger_localised_error('CORE0039', E_USER_WARNING, $assetid, $new_url);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
				$protocols['https'] = TRUE;
			}
		}

		//Feature #4759: Asset ID to URL remaps.
		foreach ($protocols as $protocol => $enabled){
			if ($enabled){
				//try deleting first as we want to be able to change remap entries for the assetid.
				try {
					$bind_vars = Array('urls' => Array($protocol.'://?a='.$assetid));
					MatrixDAL::executeQuery('remap_manager', 'deleteRemapsByURL', $bind_vars);
				} catch (DALException $e) {
					throw new Exception('Unable to delete assetid remaps due to database error: '.$e->getMessage());
				}
				$retval = @$this->addRemapURL($protocol.'://?a='.$assetid, $new_url, $expires, $never_delete, FALSE);
				if ($retval !== 1) {
					trigger_localised_error('CORE0040', E_USER_WARNING, $assetid, $new_url);
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
	* Attempts to redirect user from current URL
	*
	*
	*
	* @return boolean
	* @access public
	*/
	public function loadRemapFromCurrentURL()
	{
		$protocol = current_protocol();

		// URL without parameters
		$url      = strip_url(current_url(FALSE, TRUE));

		// This URL is verbatim
		$host = array_get_index($_SERVER, 'HTTP_HOST', '');
		$url_with_params = strip_url($host.$_SERVER['REQUEST_URI']);

		// Try verbatim URL first
		if($this->loadRemapFromURL($protocol, $url_with_params))
		     return TRUE;

		if(!$this->attr("preserve_url_parameters")) {
			// If the urls are the same with and without params, there's no
			// need to check anything else.
			if ($url === $url_with_params) {
				return FALSE;
			}

			// Try URL without paramters
			return($this->loadRemapFromURL($protocol, $url));
		}

		// Try URL, and preserve all parameters when redirecting
		$parameters = '';
		$pos = strpos($url_with_params, '?');
		if($pos !== FALSE) {
			$parameters = substr($url_with_params, $pos + 1);
		}

		// Try URL and preserve all paramters
		if (!empty($parameters)) {
			if($this->loadRemapFromURL($protocol, $url, $parameters)) {
				return TRUE;
			}
		}

		return FALSE;
	}

	/**
	* Attempts to redirect a url to its remapped url
	* If the remap manager is unsuccessful in doing so, it will return FALSE
	*
	* @param string	$protocol	the protocol that the url is using
	* @param string	$url		the url to remap from
	* @param string	$parameters	paramters string to preserve
	*
	* @return boolean
	* @access public
	*/
	public function loadRemapFromURL($protocol=NULL, $url=NULL, $parameters='')
	{
		if (is_null($protocol)) $protocol = current_protocol();
		if (is_null($url)) {
			$url = strip_url(current_url(FALSE, TRUE));
		}

		$url = $protocol.'://'.$url;
		$time      = ts_iso8601(time());

		// there is URL parameters to pass
		if(!empty($parameters)) {
		    // Try URL + ?, if match is found, this URL is marked as an exception that no need of  preserving parameters
		    try {

			    $bind_vars = Array(
							    'url'  => $url,
							    'url_with_trailing_question_mark'  => $url.'?',
							    'time' => $time,
						    );
			    $results = MatrixDAL::executeAssoc('remap_manager', 'getCurrentRemapURLTrailingQuestionMark', $bind_vars);
		    } catch (DALException $e) {
			    throw new Exception('Unable to get current remap URL for "'.$url.'" due to database error: '.$e->getMessage());
		    }

		    if(empty($results)) return FALSE;

		    $remap_url = $results[0]['remap_url'];

		    // if this query returns us a plain url (not with trailing '?'), we should pass URL parameters to remapped url
		    if($results[0]['url'] === $url) {
			// preserve parameters to remapped url
			if(strpos($remap_url, '?') !== FALSE) {
			     $remap_url = $remap_url.'&'.$parameters;
			}
			else {
			    $remap_url = $remap_url.'?'.$parameters;
			}
		    }
		}
		else {
		    // no URL parameters to pass
		    try {

			    $bind_vars = Array(
							    'url'  => $url,
							    'time' => $time,
						    );
			    $remap_url = MatrixDAL::executeOne('remap_manager', 'getCurrentRemapURL', $bind_vars);
		    } catch (DALException $e) {
			    throw new Exception('Unable to get current remap URL for "'.$url.'" due to database error: '.$e->getMessage());
		    }
		}

		// if we have a remap url, display it
		if (!empty($remap_url)) {

			header('HTTP/1.1 301 Moved Permanently');
			$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
			// Send Cacheable Header based on cache manager default setting
			if (SQ_CONF_SEND_CACHEABLE_HEADER && $cm->cacheableHeadersEnabledForCurrentProtocol()) {
				header('Pragma: cache');

				$browser_cache_expiry = $cm->attr('browser_cache_expiry');
				if (empty($browser_cache_expiry)) {
					$browser_cache_expiry = $cm->attr('expiry');
				}
				$level = $cm->cacheControlLevel();
				header('Cache-Control: max-age='.$browser_cache_expiry.', '.$level);
				header('Expires: '.gmdate('D, d M Y H:i:s', time() + $browser_cache_expiry).' GMT');
			}
			header('Location: '.$remap_url);

			return TRUE;
		}

		return FALSE;

	}//end loadRemapFromURL()

	/**
	* Attempts to get remap from url
	*
	* @param string	$protocol	the protocol that the url is using
	* @param string	$url		the url to remap from
	*
	* @return string|FALSE
	* @access public
	*/
	public function getRemapFromURL($protocol=NULL, $url=NULL)
	{
		if (is_null($protocol)) $protocol = current_protocol();
		if (is_null($url)) {
			$url = strip_url(current_url(FALSE, TRUE));
		}

		$url = $protocol.'://'.$url;

		try {
			$time      = ts_iso8601(time());
			$bind_vars = Array(
							'url'  => $url,
							'time' => $time,
						 );
			$remap_url = MatrixDAL::executeOne('remap_manager', 'getCurrentRemapURL', $bind_vars);
		} catch (DALException $e) {
			throw new Exception('Unable to get current remap URL for "'.$url.'" due to database error: '.$e->getMessage());
		}

		// if remap is found, keeping finding its remap until the final remap
		if (!empty($remap_url)) {
			$protocol = parse_url($remap_url, PHP_URL_SCHEME);
			$search = '#'.$protocol.'\\://#';
			$url = preg_replace($search, '', $remap_url);

			// find the final remap url recursively
			$final_url = $this->getRemapFromURL($protocol, $url);
			if ($final_url !== FALSE) $remap_url = $final_url;
			return $remap_url;
		}
		return FALSE;

	}//end getRemapFromURL()

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

		// Send Cacheable Header based on cache manager default setting
		$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		if (SQ_CONF_SEND_CACHEABLE_HEADER && SQ_CONF_SEND_404_CACHEABLE_HEADER && $cm->cacheableHeadersEnabledForCurrentProtocol()) {
			header('Pragma: cache');

			$browser_cache_expiry = $cm->attr('browser_cache_expiry');
			$acc_cache_expiry = $cm->attr('accelerator_cache_expiry');
			$acc_cache_string = empty($acc_cache_expiry) ? '' : "s-maxage=$acc_cache_expiry, ";

			if (empty($browser_cache_expiry)) {
				$browser_cache_expiry = $cm->getExpiry($asset->type(), $asset->id);
			}

			header('Cache-Control: max-age='.$browser_cache_expiry.', '.$acc_cache_string.$cm->cacheControlLevel());
			header('Expires: '.gmdate('D, d M Y H:i:s', time() + $browser_cache_expiry).' GMT');
		}

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
		ob_start();
			$asset->printFrontend();
			$contents =  ob_get_contents();
		ob_end_clean();

		$GLOBALS['SQ_SYSTEM']->replaceKeyword($contents);

		echo $contents;

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
