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
* $Id: site.inc,v 1.132 2013/03/21 05:01:20 cupreti Exp $
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
* @version $Revision: 1.132 $
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
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return array
	* @access public
	*/
	public function lockTypes()
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
	public function getWebPaths($url=FALSE)
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
	public function addWebPath($url)
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
	public function deleteWebPath($url)
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
	public function delete($release_lock=TRUE, $check_locked=TRUE)
	{
		$this->_tmp['deleting_site'] = TRUE;
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!$this->saveWebURLs(Array())) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		if (!parent::delete($release_lock, $check_locked)) {
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
	public function getSiteURLs()
	{
		$db = MatrixDAL::getDb();

		if (!isset($this->_tmp['urls_info'])) {
			$sql = 'SELECT urlid, url, http, https, base_contextid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_url
					';
			$where = 'assetid = :assetid';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			$sql .= $where.' ORDER BY urlid';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'assetid', $this->id);
				$result_x = MatrixDAL::executePdoGroupedAssoc($query);

				// TODO: DAL always , whereas I THINK this depends on PEAR DB's
				// ability to - so need to convert - need better way of doing this...
				$result = Array();
				foreach ($result_x as $key => $row) {
					$result[$key] = $row[0];
				}
				unset($result_x);
			} catch (Exception $e) {
				throw new Exception('Unable to get applied schemas for asset ID #'.$this->id.' due to database error: '.$e->getMessage());
			}

			$this->_tmp['urls_info'] = $result;

		}//end if not cached

		return $this->_tmp['urls_info'];

	}//end getSiteURLs()


	/**
	* Attempts to add a url to this site, site's don't have paths, only URLs
	*
	* Return 1 if we complete ok but do not update, 2 for updated and 0 for failure
	*
	* @param array	$save_urls	an Array($urlid => Array(url => '', http => boolean, https => boolean)
	*
	* @return int
	* @access public
	*/
	public function saveWebURLs(Array $save_urls)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$updated = FALSE;
		$processed_urlids = Array();
		$current_urls = $this->getSiteURLs();
		$system_root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);

		for (reset($save_urls); NULL !== ($save_urlid = key($save_urls)); next($save_urls)) {
			// If for some reason there is no base context ID yet defined, set it to zero (default)
			$save_urls[$save_urlid]['base_contextid'] = array_get_index($save_urls[$save_urlid], 'base_contextid', 0);

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
				if ($path !== $clean_path) {
					if ($clean_path != '') {
						trigger_localised_error('CORE0080', sprintf(translate('URL "%1$s" contains a path "%2$s" which was deemed to be not valid. A valid version is "%3$s"'), $save_urls[$save_urlid]['url'], $path, $clean_path), E_USER_WARNING);
					} else {
						trigger_localised_error('CORE0124', sprintf(translate('URL "%1$s" contains a path "%2$s" which was deemed to be not valid.'), $save_urls[$save_urlid]['url'], $path), E_USER_WARNING);
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
				trigger_localised_error('CORE0082', sprintf(translate('URL "%s" is not based upon an existing System Root URL'), $save_urls[$save_urlid]['url']), E_USER_WARNING);
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

			// We don't have it. Does anyone else?
			if (!$found) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL(NULL, $save_urls[$save_urlid]['url'], TRUE, TRUE);
				if (!is_null($asset)) {
					trigger_localised_error('CORE0081', sprintf(translate('URL "%1$s" is already in use by "%2$s" [#%3$s]'), $save_urls[$save_urlid]['url'], $asset->name, $asset->id), E_USER_WARNING);

					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
			}

			// are we an existing or an new one
			if (isset($current_urls[$save_urlid])) {
				// we are an existing one
				if ($current_urls[$save_urlid]['url']   != $save_urls[$save_urlid]['url'] ||
					$current_urls[$save_urlid]['http']  != $save_urls[$save_urlid]['http'] ||
					$current_urls[$save_urlid]['https'] != $save_urls[$save_urlid]['https'] ||
					$current_urls[$save_urlid]['base_contextid'] != $save_urls[$save_urlid]['base_contextid']) {

					$sql = 'UPDATE
								sq_ast_url
							SET
								url		= :url,
								http	= :http,
								https	= :https,
								base_contextid = :base_contextid
							WHERE
									urlid	= :urlid
								AND	assetid	= :assetid';

					try {
						$query = MatrixDAL::preparePdoQuery($sql);
						MatrixDAL::bindValueToPdo($query, 'url',     $save_urls[$save_urlid]['url']);
						MatrixDAL::bindValueToPdo($query, 'http',    $save_urls[$save_urlid]['http']);
						MatrixDAL::bindValueToPdo($query, 'https',   $save_urls[$save_urlid]['https']);
						MatrixDAL::bindValueToPdo($query, 'base_contextid',   $save_urls[$save_urlid]['base_contextid']);
						MatrixDAL::bindValueToPdo($query, 'urlid',   $save_urlid);
						MatrixDAL::bindValueToPdo($query, 'assetid', $this->id);
						MatrixDAL::execPdoQuery($query);
						$updated = TRUE;
					} catch (Exception $e) {
						throw new Exception('Unable to update URLs for asset "'.$this->name.'" (#'.$this->id.') due to database error: '.$e->getMessage());
					}
				}

			} else {
				// we are not going to insert same url
				foreach ($current_urls as $url) {
				   if($save_urls[$save_urlid]['url'] === $url['url']) {
					 trigger_localised_error('CORE0081', sprintf(translate('URL "%1$s" is already in use by "%2$s" [#%3$s]'), $save_urls[$save_urlid]['url'], $this->name, $this->id), E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				   }
				}

				// we are a new one
				$curr_urlid = MatrixDAL::executeOne('core', 'seqNextVal', Array('seqName' => 'sq_ast_url_seq'));

				$sql = 'INSERT INTO
							sq_ast_url
							(
								urlid,
								assetid,
								url,
								http,
								https,
								base_contextid
							)
							VALUES
							(
								:urlid,
								:assetid,
								:url,
								:http,
								:https,
								:base_contextid
							)';

				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'url',     $save_urls[$save_urlid]['url']);
					MatrixDAL::bindValueToPdo($query, 'http',    $save_urls[$save_urlid]['http']);
					MatrixDAL::bindValueToPdo($query, 'https',   $save_urls[$save_urlid]['https']);
					MatrixDAL::bindValueToPdo($query, 'urlid',   $curr_urlid);
					MatrixDAL::bindValueToPdo($query, 'assetid', $this->id);
					MatrixDAL::bindValueToPdo($query, 'base_contextid',   $save_urls[$save_urlid]['base_contextid']);
					MatrixDAL::execPdoQuery($query);
					$updated = TRUE;
				} catch (Exception $e) {
					throw new Exception('Unable to insert URLs for asset "'.$this->name.'" (#'.$this->id.') due to database error: '.$e->getMessage());
				}

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
						 urlid = :urlid';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'urlid', $urlid);
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception('Unable to delete URLs from asset "'.$this->name.'" (#'.$this->id.') due to database error: '.$e->getMessage());
			}

			// remove corresponding authentication redirects
			try {
				$bind_vars = Array('old_auth_urlid' => $urlid, 'new_auth_urlid' => NULL);
				MatrixDAL::executeQuery('site', 'changeAuthUrlId', $bind_vars);
			} catch (Exception $e) {
				throw new Exception('Unable to remove authentication redirect for auth_urlid: '.$urlid.' due to database error: '.$e->getMessage());
			}
			$updated = TRUE;
		}

		unset($this->_tmp['urls_info']);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		if ($updated) return 2;
		return 1;

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
	public function _morphCleanup($new_type_code)
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
	public function updateLookups($auto_add_remaps=TRUE)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		unset($this->_tmp['lookups']);
		unset($this->_tmp['url']);
		unset($this->_tmp['href']);

		if (!isset($this->_tmp['deleting_site'])) {
			$this->_tmp['deleting_site'] = FALSE;
		}

		// get all the lookup information that we have set (i.e. no inherited values), ignoring designs as they are set on every url
		$lookup_values = $this->getLookupValues(FALSE, $prefix='design::', FALSE);
		$our_edit_layouts = $this->getLookupValues(FALSE, 'layout::');

		// remove all normal and override lookup values for this site
		$sql = 'DELETE FROM
					sq_ast_lookup_value
				WHERE
					url IN
						(
							(SELECT
								url
							FROM
								sq_ast_lookup l
							WHERE
								assetid = :assetid_1)
							UNION
							(SELECT
								url || \'/\'
							FROM
								sq_ast_lookup l
							WHERE
								assetid = :assetid_2)
						) ';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'assetid_1', $this->id);
			MatrixDAL::bindValueToPdo($query, 'assetid_2', $this->id);
			MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception('Unable to remove existing lookups for Site asset "'.$this->name.'" (#'.$this->id.') due to database error: '.$e->getMessage());
		}

		// remove lookup information for this asset
		$sql = 'DELETE FROM
					sq_ast_lookup
				WHERE
					assetid = :assetid';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'assetid', $this->id);
			MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception('Unable to remove existing lookups for Site asset "'.$this->name.'" (#'.$this->id.') due to database error: '.$e->getMessage());
		}

		$our_design_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, 'design', FALSE);
		$design_lookups = Array();
		foreach ($our_design_links as $link) {
			if (preg_match('/^design::(system|user)::(.*)$/', $link['value'])) {
				$design_lookups[$link['value']] = $link['minorid'];
			}
		}
		$override_design_lookups = Array();
		// look for overrides and overwrite the array in these places
		foreach ($our_design_links as $link) {
			if (preg_match('/^override::design::(system|user)::(.*)$/', $link['value'])) {
				$name = preg_replace('/^override::(.*)$/', '$1', $link['value']);
				$override_design_lookups[$name] = $link['minorid'];
			}
		}
		unset($our_design_links);

		$our_layout_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, 'paint_layout_page', FALSE);
		$paint_layouts = Array();
		foreach ($our_layout_links as $link) {
			if (preg_match('/^paint_layout::(system|user)::.*$/', $link['value'])) {
				$paint_layouts[$link['value']] = Array(
												'value'	=> $link['minorid'],
											   );
			}
		}
		// override layouts take priority over normal, inheritable layouts
		$override_paint_layouts = Array();
		foreach ($our_layout_links as $link) {
			if (preg_match('/^override::paint_layout::system::.*$/', $link['value'])) {
				$name = preg_replace('/^override::(.*)$/', '$1', $link['value']);
				$override_paint_layouts[$name] = Array(
											'value'	=> $link['minorid'],
										   );
			}
		}
		unset($link);
		unset($our_layout_links);

		// edit layouts applied to the asset
		$edit_layouts = Array();
		foreach($our_edit_layouts as $layout_url => $layouts) {
			foreach($layouts as $name => $layout_data) {
				if (!isset($edit_layouts[$name])) {
					$edit_layouts[$name] = $layout_data['value'];
				}//end if
			}//end foreach
		}//end foreach
		unset($our_edit_layouts);

		$urls = $this->getSiteURLs();
		$current_urls = Array();
		// if we have urls then do some url inserting
		for (reset($urls); NULL !== ($urlid = key($urls)); next($urls)) {
			$current_urls[] = $urls[$urlid]['url'];
			// merge old paint layout lookup values with design values for inserting into the lookup_value table later
			// i.e. designs are set on an asset basis, not on a URL basis like paint layouts
			foreach ($design_lookups as $design_name => $value) {
				$lookup_values[$urls[$urlid]['url']][$design_name]['value'] = $value;
			}
			foreach ($override_design_lookups as $override_design_name => $value) {
				$lookup_values[$urls[$urlid]['url'].'/'][$override_design_name]['value'] = $value;
			}

			if (!isset($lookup_values[$urls[$urlid]['url']])) {
				// insert an entry so paint layout values are also checked even if we don't have a design set
				$lookup_values[$urls[$urlid]['url']] = '';
			}
			if (!isset($lookup_values[$urls[$urlid]['url'].'/'])) {
				// insert an entry so override paint layout values are also checked even if we don't have an override design set
				$lookup_values[$urls[$urlid]['url'].'/'] = '';
			}

			// merge in edit layout lookup values
			foreach($edit_layouts as $layout_name => $value) {
				if (!isset($lookup_values[$urls[$urlid]['url']][$layout_name])) {
					$lookup_values[$urls[$urlid]['url']][$layout_name]['value'] = $value;
				}
			}

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
							:url,
							:assetid,
							:http,
							:https,
							:urlid
						)';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'url',     $urls[$urlid]['url']);
				MatrixDAL::bindValueToPdo($query, 'assetid', $this->id);
				MatrixDAL::bindValueToPdo($query, 'http',    ($this->force_secure === '1') ? '0' : $urls[$urlid]['http']);
				MatrixDAL::bindValueToPdo($query, 'https',   $urls[$urlid]['https']);
				MatrixDAL::bindValueToPdo($query, 'urlid',   $urlid);
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception('Unable to insert lookups for Site asset "'.$this->name.'" (#'.$this->id.') due to database error: '.$e->getMessage());
			}
		}

		// If theres no lookup entry, do not insert value for it (See bug #3796)
		if (!empty($urls)) {

			// merge in asset based paint layout values if they do not already exist
			foreach ($lookup_values as $url => $data) {

				//Bug #5251, filter out lookup values for old urls
				if (!in_array(rtrim($url, '/'), $current_urls)) {
					unset($lookup_values[$url]);
					continue;
				}

				if (preg_match('/.+\/$/', $url)) {
					// override layout
					foreach ($override_paint_layouts as $name => $value) {
						if (empty($data)) {
							$lookup_values[$url][$name] = $value;
						} else if (!in_array($name, array_keys($data))) {
							$lookup_values[$url][$name] = $value;
						}
					}
				} else {
					// normal layout
					foreach ($paint_layouts as $name => $value) {
						if (empty($data)) {
							$lookup_values[$url][$name] = $value;
						} else if (!in_array($name, array_keys($data))) {
							$lookup_values[$url][$name] = $value;
						}
					}
				}
			}

			// insert lookup values
			foreach ($lookup_values as $url => $data) {
				// calculate the depth of the url
				$exploded_url = explode('/', $url);
				$depth = count($exploded_url)-1;

				if (!empty($data)) {
					foreach ($data as $layout_name => $value) {
						$sql = 'INSERT INTO
									sq_ast_lookup_value
									(
										url,
										name,
										value,
										depth
									)
									VALUES
									(
										:url,
											:name,
										:value,
										:depth
									)';

						try {
							$query = MatrixDAL::preparePdoQuery($sql);
							MatrixDAL::bindValueToPdo($query, 'url',   $url);
							MatrixDAL::bindValueToPdo($query, 'name',  $layout_name);
							MatrixDAL::bindValueToPdo($query, 'value', $value['value']);
							MatrixDAL::bindValueToPdo($query, 'depth', $depth);
							MatrixDAL::execPdoQuery($query);
						} catch (Exception $e) {
							throw new Exception('Unable to insert lookup values for Site asset "'.$this->name.'" (#'.$this->id.') due to database error: '.$e->getMessage());
						}
					}
				}
			}
		} // End if empty urls

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
	public function describeLink($linkid)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
		switch (strtolower($link['value'])) {
			case 'index' :
				return translate('The index page for the site');

			break;
			case 'not_found' :
				return translate('The not found page for the site');

			break;
			case 'archive' :
				return translate('Page to act as a "Not Found" page when archived assets are accessed');

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
	public function printFrontend()
	{
		// check that the user has read access to the site
		if (!$this->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin(translate('Login'), sprintf(translate('You do not have permission to access the asset'), $this->name));


			return;
		}

		$index_page = $this->getSpecialPage('index');
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
			trigger_localised_error('CORE0065', sprintf(translate('No Index Page Set for %1$s "%2$s" (Id #%3$s)'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'), $this->name, $this->id), E_USER_NOTICE);
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
	public function printBody()
	{
		$index_page = $this->getSpecialPage('index');
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
	* @param object $design Overriding design asset
	* @param object $layout	Overriding paint layout asset
	*
	* @return void
	* @access public
	*/
	public function paintAsset(Asset $asset, $design=NULL, $layout=NULL)
	{
		if (is_null($asset)) return;

		// check that the user has read access to the index page
		if (!$asset->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin(translate('Login'), sprintf(translate('You do not have permission to access the asset'), $this->name));


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

		// Validate the overriding design, if any
		if (!is_null($design)) {
			$design_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($design->id, Array('design'), FALSE);
			if (empty($design_info)) {
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($design);
				unset($design);
				$design = NULL;
			}
		}

		$asset_url = strip_url($asset->getURL(NULL, TRUE), TRUE);
		if (is_null($design)) {
			$url = strip_url($this->getURL(), TRUE);
			$design_name = $this->getCurrentDesignName();
			$result = $GLOBALS['SQ_SYSTEM']->am->getDesignFromURL($asset_url, $design_name);

			if (empty($result)) {
				$db = MatrixDAL::getDb();
				// try and find the default design used by the asset we are painting (i.e. not our default design)
				$sql = 'SELECT lv.value as designid, a.type_code
						FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup l
								INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_value lv ON l.url = lv.url
								INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON lv.value = a.assetid
						';
				$where ='l.assetid  = :assetid
						  AND lv.url  LIKE :url
						  AND lv.name    = :lookup_name';
				$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lv');
				$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
				$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

				$sql .= $where;
				$sql = db_extras_modify_limit_clause($sql, MatrixDAL::getDbType(), 1);

				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'assetid', $this->id);
					MatrixDAL::bindValueToPdo($query, 'url', $url.'%');
					MatrixDAL::bindValueToPdo($query, 'lookup_name', 'design::system::frontend');
					$result = MatrixDAL::executePdoAssoc($query);
					if (!empty($result)) $result = $result[0];
				} catch (Exception $e) {
					throw new Exception('Unable to get default design for asset: '.$asset->id.' due to database error: '.$e->getMessage());
				}
			}

			// we have found the design to use
			if ($result) {
				$design = $GLOBALS['SQ_SYSTEM']->am->getAsset($result['designid'], $result['type_code']);
			}
		}//end if

		global $ASSET_LINEAGE;
		if (!empty($asset_url)) {
			$ASSET_LINEAGE = $GLOBALS['SQ_SYSTEM']->am->getLineageFromUrl(NULL, $asset_url);
		}

		// ignore current url, use assetid as cache key, to globally cache the 404 page
		if($this->attr('not_found_page_cache_globally') == TRUE) {
		    $cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		    $cm->global_cache = TRUE;
		}

		if (!is_null($design)) {
			if (SQ_IN_LIMBO) {
				$this->printFrontendAsset($this, $design);
			} else {
				// if there is an appopriate paint layout for the index page, tell the system to use it
				if (!is_null($layout)) {
					// Validate the overriding paint layout, if any
					$layout_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($layout->id, Array('paint_layout_page'));
					if (!empty($layout_info)) {
						$layout_id = $layout->id;
					}
				}
				if (empty($layout_id)) {
					$layout_name = $this->getCurrentPaintLayoutName();
					$layout_id = $GLOBALS['SQ_SYSTEM']->am->getValueFromURL($asset_url, $layout_name);
				}
				if ($layout_id) {
					$GLOBALS['SQ_SYSTEM']->setGlobalDefine('SQ_PAINT_LAYOUT_ID', $layout_id);
				}
				$this->printFrontendAsset($asset, $design);
			}
		} else {
			// we can't find a design, oh well let's just print out our body
			$asset->printBody();
		}

		if($this->attr('not_found_page_cache_globally') === TRUE) {
		      $cm->global_cache = FALSE;
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
	public function initLimbo()
	{
		if (isset($_GET['limbo_assetid']) && $_GET['limbo_assetid'] == $this->id) {
			// print this site's limbo instead of the index page
			parent::initLimbo();
		} else {
			$index_page = $this->getSpecialPage('index');
			if (is_null($index_page)) {
				parent::initLimbo();
				return;
			}

			$GLOBALS['SQ_SYSTEM']->backend->out->addFormActionGetVar('limbo_assetid', $index_page->id, TRUE);
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
	public function getSpecialPageLink($page)
	{
		switch ($page) {
			case 'index'     :
			case 'not_found' :
			case 'not_found_design':
			case 'not_found_layout':
			case 'archive'   :
				return $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', TRUE, $page);
			break;

			default :
				trigger_localised_error('CORE0075', sprintf(translate('Special Page "%s" unknown'), $page), E_USER_WARNING);
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
	public function getSpecialPage($page)
	{
		$link = $this->getSpecialPageLink($page);
		if (empty($link)) {
			$null = NULL;
			return $null;
		}

		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
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
	public function getSiteNetwork()
	{
		$null = NULL;

		$network_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_SIGNIFICANT, 'site_network', FALSE, 'minor');
		if (empty($network_links)) return $null;

		$site_network = $GLOBALS['SQ_SYSTEM']->am->getAsset($network_links[0]['majorid'], $network_links[0]['major_type_code']);
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
	public function getNetworkPrimaryURL()
	{
		$site_network = $this->getSiteNetwork();
		if (is_null($site_network)) return '';
		return $site_network->getPrimaryURL();

	}//end getNetworkPrimaryURL()


	/**
	* Get keyword replacement
	*
	* Provides a mechanism for providing keyword replacement functions by
	* looking up the function named 'getKeywordNameKeywordReplacement'
	* so an asset wishing to add support for 'best_ever' custom keyword
	* should implement a function getBestEverKeywordReplacement()
	* which returns a replacement string
	*
	* @param string	$keyword	Keyword to find a replacement for
	* @param string $additional_replacements    Additional keyword replacements for 'replace_keywords' modifier
	*
	* @return string
	* @access public
	*/
	function getKeywordReplacement($keyword, $additional_replacements=Array())
	{
		$replacement = NULL;

		// Remove any modifiers from keyword - no need to get context here,
		// since the only keywords we are giving (index, not found, archived
		// page asset IDs) are saved in links, which aren't contextable
		$full_keyword = $keyword;
		$keyword = parse_keyword($keyword, $modifiers);

		$keyword_name = substr($keyword, 6);
		if (0 === strpos($keyword_name, 'index_id') || 0 === strpos($keyword_name, 'not_found_id') || 0 === strpos($keyword_name, 'archive_id') ) {
			$keyword_name = substr($keyword_name, 0, -3);
			$asset = $this->getSpecialPage($keyword_name);
			if (!is_null($asset)) $replacement = $asset->id;
		}

		if ($replacement !== NULL) {
			if (count($modifiers) > 0) {
				apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $this->id));
			}
		} else {
			// Use full keyword, so modifiers still get used
			$replacement = parent::getKeywordReplacement($full_keyword);
		}

		return $replacement;

	}//end getKeywordReplacement()



	/**
	* Returns the most recent 'last modified time' among all component parts
	*
	* If the site's URL is being loaded, normally the index page has already
	* been loaded, and therefore does not count towards the "effective last
	* updated" time. This is not satisfactory in assets where updates are not
	* dependent on child assets (such as bodycopies). Make sure the index page
	* is added to ensure this happens.
	*
	* @param array	$assetids	id of all assets that get loaded during printFrontend
	*
	* @return int
	* @access public
	*/
	public function getEffectiveLastUpdatedTime($assetids)
	{
		$index_link = $this->getSpecialPageLink('index');
		if (empty($index_link) === FALSE) {
			$assetids[] = $index_link['minorid'];
		}

		return parent::getEffectiveLastUpdatedTime($assetids);

	}//end getEffectiveLastUpdatedTime()

}//end class

?>
