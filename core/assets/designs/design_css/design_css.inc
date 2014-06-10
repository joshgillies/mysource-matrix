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
* $Id: design_css.inc,v 1.50 2013/08/06 03:18:20 akarelia Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design/design.inc';

/**
* Design_Css
*
* Purpose
* This is a CSS-specific asset which is parsed on upload/edit and can be linked
* to an existing design. This asset supports all the design_areas that can be used
* in standard Designs
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.50 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Css extends Design
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);
		// We store a temp var that indicates whether the cache has been updated
		// on this request
		$this->_tmp['updated_cache'] = FALSE;

	}//end constructor


	/**
	 * Wakeup.
	 */
	function __wakeup()
	{
		// Ensure this is reinitialised when restored from a serialised state.
		$this->_tmp['updated_cache'] = FALSE;
		
	}//end wakeup


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* CSS Files ensure their name is valid before they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _preCreateCheck(Array &$link)
	{
		if (!parent::_preCreateCheck($link)) return FALSE;

		$new_name = strtolower(trim($this->attr('id_name')));

		// no ampersands in css file name
		$new_name = str_replace('&', SQ_CONF_WEB_PATH_SEPARATOR.'and'.SQ_CONF_WEB_PATH_SEPARATOR,  $new_name);

		// no spaces in css file name
		$new_name = preg_replace('/\\s+/', SQ_CONF_WEB_PATH_SEPARATOR, $new_name);

		// taken (in part) from info here -> http://www.w3.org/Addressing/URL/5_URI_BNF.html
		$new_name = preg_replace('/[^a-zA-Z0-9\-$_@.!*~(),]/', '',  $new_name);

		// ensure a virtual .css file extension
		if (substr($new_name, -4) != '.css') {
			$new_name .= '.css';
		}

		$this->setAttrValue('id_name', $new_name);

		return TRUE;

	}//end _preCreateCheck()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* For example, creating other child assets should be performed in this method
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		// set an initial web path
		$initial_path = strtolower($this->attr('id_name'));
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_paths = make_valid_web_paths(Array($initial_path));
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $this->id, TRUE);
		$this->saveWebPaths($good_paths);

		$this->_createCacheFile();
		return TRUE;

	}//end _createAdditional()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_1	=> Array(),
				SQ_LINK_TYPE_2	=> Array(
									'design_css_customisation'	=> Array(
																	'card'		=> 'M',
																	'exclusive'	=> FALSE,
																   ),
									'file'						=> Array(
																	'card'		=> 'M',
																	'exclusive'	=> FALSE,
																   ),
								   ),
				SQ_LINK_TYPE_3	=> Array(
									'design_area'	=> Array(
														'card'		=> 'M',
														'exclusive'	=> FALSE,
													   ),
									'file'			=> Array(
														'card'		=> 'M',
														'exclusive'	=> FALSE,
													   ),
								   ),
				SQ_LINK_NOTICE	=> Array(),
			   );

	}//end _getAllowedLinks()


	/**
	* Creates a link between this asset and the passed asset, of a certain type
	* and returns the linkid of the created link
	*
	* @param Asset	$minor		the minor asset that we are linking to
	* @param string	$link_type	the type of link this is
	* @param string	$value		the value that is to be associated with this link
	* @param string	$sort_order	the position in the links list that this link should take,
	*							if NULL or less than zero places at end of list
	* @param string	$dependant	'0' / '1' on whether the this asset is dependant on the
	*							asset that will be linked by this new link
	* @param string	$exclusive	'0' / '1' on whether the this asset is linked exclusivly
	*							to the asset that will be linked by this new link
	*
	* @return int
	* @access public
	*/
	public function createLink(Asset $minor, $link_type, $value='', $sort_order=NULL, $dependant='0', $exclusive='0')
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$linkid = parent::createLink($minor, $link_type, $value, $sort_order, $dependant, $exclusive);
		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// special processing if we are creating a TYPE_2 linked file (inherited by customisations)
		if ($minor instanceof File) {

			$am = $GLOBALS['SQ_SYSTEM']->am;

			$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'design_css_customisation', TRUE, 'major', 'customisation');
			foreach ($customisation_links as $link) {
				$customisation = $am->getAsset($link['minorid'], $link['minor_type_code']);
				if (is_null($customisation)) continue;

				$file_link_ids = Array();
				$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($customisation->id, SQ_LINK_TYPE_2, 'file', FALSE);
				foreach ($file_links as $link) {
					$file_link_ids[] = $link['minorid'];
				}
				$file_link_info = $am->getAssetInfo(array_unique($file_link_ids));

				// is there already a direct file with the same name
				$insert_link = TRUE;
				foreach ($file_link_info as $info) {
					if ($info['name'] == $minor->name) {
						$insert_link = FALSE;
					}
				}

				if ($insert_link) {
					if (!$customisation->createLink($minor, SQ_LINK_TYPE_3, '', NULL, 1)) {
						trigger_localised_error('CORE0162', E_USER_WARNING);
						$am->forgetAsset($customisation);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						return FALSE;
					}
				}

				$am->forgetAsset($customisation);

			}//end foreach customisation

		}//end if is_a file

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $linkid;

	}//end createLink()


	/**
	* Removes a link where this asset is the major player
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return boolean
	* @access public
	*/
	public function deleteLink($linkid)
	{
		// first, we should try and find the link
		$delete_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid, $this->id);
		if (empty($delete_link)) {
			trigger_localised_error('CORE0155', E_USER_WARNING, $linkid);
			return FALSE;
		}

		$am = $GLOBALS['SQ_SYSTEM']->am;
		$minor = $am->getAsset($delete_link['minorid'], $delete_link['minor_type_code']);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// special processing if we are deleting a TYPE_2 linked file (a file overridding a design file)
		if ($minor instanceof File) {

			$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'design_css_customisation', TRUE, 'major', 'customisation');
			foreach ($customisation_links as $link) {
				$customisation = $am->getAsset($link['minorid'], $link['minor_type_code']);
				if (is_null($customisation)) continue;

				$file_link_ids = Array();
				$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($customisation->id, SQ_LINK_TYPE_3, 'file', FALSE);

				foreach ($file_links as $link) {
					if ($link['minorid'] == $delete_link['minorid']) {
						// found the link to delete
						if (!$customisation->deleteLink($link['linkid'])) {
							trigger_localised_error('CORE0165', E_USER_WARNING);
							$am->forgetAsset($customisation);
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							return FALSE;
						}
					}
				}//end foreach $file_links

				$am->forgetAsset($customisation);

			}//end foreach customisation
		} //end is_a file

		if (!parent::deleteLink($linkid)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end deleteLink()


	/**
	* Does the actual removing of the link for the customisation
	*
	* The default deleteLink() replaces the (Type-2) links of any customised design files with a Type-3
	* to the uncustomised version. This screws things around when creating the customised versions in
	* the first place.
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return boolean
	* @access public
	*/
	public function deleteExistingLink($linkid)
	{
		// first, we should try and find the link
		$delete_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid, $this->id);
		if (empty($delete_link)) {
			trigger_localised_error('CORE0155', E_USER_WARNING, $linkid);
			return FALSE;
		}

		$am = $GLOBALS['SQ_SYSTEM']->am;
		$minor = $am->getAsset($delete_link['minorid'], $delete_link['minor_type_code']);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!parent::deleteLink($linkid)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end deleteExistingLink()


	/**
	* Prints out the Frontend for this asset
	* it's up to the kids to override
	*
	* @return void
	* @access public
	*/
	public function printFrontend()
	{
		if (!$this->readAccess()) { 	 
		    $GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $this->name)); 	 
		    return; 	 
		}
		$this->paint($this);

	}//end printFrontend()


	/**
	* Returns the parent design/design_customisation that this customisation is customising
	*
	* @return Design_CSS
	* @access public
	*/
	public function getParentDesign()
	{
		// a css file is its own customisation
		return $this;

	}//end getParentDesign()


	/**
	* Outputs the either as straight HTML or PHP code for the var, depending on the type
	*
	* @param string	$var	the name of the var to print out
	* @param array	$args	array of any extra arguments that existed in the print command
	*
	* @return void
	* @access public
	*/
	public function printVar($var, Array $args)
	{
		switch ($var) {
			case 'css_header' :
				if (SQ_CONF_DISABLE_ATTRIBUTION === '0') {
echo '/*
  Running Squiz Matrix
  Developed by Squiz - http://www.squiz.net
  Squiz, Squiz Matrix, MySource, MySource Matrix and Squiz.net are registered Trademarks of Squiz Pty Ltd
  Page generated: <?php echo date(\'d F Y H:i:s\')."\n"; ?>
*/

';
				}
			break;

			default :
				parent::printVar($var, $args);

		}//end switch

	}//end printVar()


	/**
	* Prints the header for the output
	*
	* @return void
	* @access public
	*/
	public function _getDesignFileHeader()
	{
		return '<?php if (!isset($NO_HEADER)) {	header("Content-type: text/css"); }	?>
		'.parent::_getDesignFileHeader();

	}//end _getDesignFileHeader()


	/**
	* Processes a backend submission from this asset, returns TRUE if all OK
	*
	* @param Backend_Outputter	$o		Backend Outputter
	* @param array				&$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	public function processBackend(Backend_Outputter $o, Array &$link)
	{	
		$public_userid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');
		
		//store old attributes, status and public read permission that can affect the regeneration of static cache file
		$cache = $this->attr('cache_css');
		$strip = $this->attr('strip_whitespace');
		//status and public read permission can make the URLs of child assets changed, so we will need to regenerate cache file 
		$status = $this->status;
		$read_access = $this->_checkPermissionAccess(SQ_PERMISSION_READ, Array($public_userid));
		
		if (!parent::processBackend($o, $link)) return FALSE;

		//URLs only changed if status is changed from or to live, and/or when public read permission is changed
		$status_changed = ($status != $this->status) && (($status == SQ_STATUS_LIVE) || ($this->status == SQ_STATUS_LIVE));
		$read_access_changed = ($read_access != $this->_checkPermissionAccess(SQ_PERMISSION_READ, Array($public_userid)));
		
		$regen = $this->attr('cache_css') && (($cache != $this->attr('cache_css')) || ($strip != $this->attr('strip_whitespace')) || $status_changed
												|| $read_access_changed);
		// Okay, if after processing everything in the backend, and the
		// status of the cache_css attribute is TRUE (use static cache file) and either the cache_css attribute, the strip_whitespace attribute,
		// the status (changed to or from live), or the public read permission is changed,
		// we've gotta update the static cache to reflect that this has changed.
		if ($regen && !$this->_tmp['updated_cache']) {
			$this->_updateCacheFile();
			$am = $GLOBALS['SQ_SYSTEM']->am;
			$customisation_links = $am->getLinks($this->id, SQ_LINK_TYPE_2, 'design_css_customisation', TRUE, 'major', 'customisation');

			foreach ($customisation_links as $link) {
				$customisation = $am->getAsset($link['minorid'], $link['minor_type_code']);
				if (is_null($customisation)) continue;
				$customisation->_updateCacheFile();
			}// end foreach
		}

		return TRUE;

	}//end processBackend()


	/**
	* (Re)Generates the design file
	*
	* @param boolean	$do_customisations	Generate design files for any customisations
	*
	* @return boolean
	* @access public
	*/
	public function generateDesignFile($do_customisations=TRUE)
	{
		$success = TRUE;
		$success &= parent::generateDesignFile($do_customisations);

		if ($this->attr('cache_css')) {
			$success &= $this->_updateCacheFile();
		}
		return $success;
	}//end generateDesignFile()


	/**
	* Updates each of our customisations from ourselves
	*
	* @return void
	* @access public
	*/
	public function updateCustomisations()
	{
		$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'design_css_customisation', TRUE, 'major', 'customisation');
		$am = $GLOBALS['SQ_SYSTEM']->am;
		foreach ($customisation_links as $link) {
			$customisation = $am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($customisation)) continue;
			if ($acquired = $GLOBALS['SQ_SYSTEM']->am->acquireLock($customisation->id, 'all')) {
				$customisation->updateFromParent($this);
				if ($acquired != 2) {
					$GLOBALS['SQ_SYSTEM']->am->releaseLock($customisation->id, 'all');
				}
			}
		}// end foreach

	}//end updateCustomisations()


	/**
	* Creates / updates the static file for this css design
	*
	* @return boolean
	* @access public
	*/
	public function _updateCacheFile()
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;

		// Generate the CSS static file. Note that we've turned error_reporting off for this
		// because design_file.php does a whole bunch of define()ing that can cause problems
		$old_level = error_reporting();
		error_reporting(0);
		ob_start();
			$this->paint($this, Array(), FALSE);
			$file_contents = ob_get_contents();
		ob_end_clean();
		error_reporting($old_level);

		$cache_link = $am->getLink($this->id, SQ_LINK_TYPE_3, 'file', FALSE, 'css_cache');
		if (empty($cache_link)) {
			$cache_asset = $this->_createCacheFile();
		} else {
			$cache_asset = $am->getAsset($cache_link['minorid']);
		}
		if ($cache_asset == NULL) return FALSE;

		// Strip excess whitespace
		if ($this->attr('strip_whitespace')) {
			$file_contents = preg_replace('/\s+/is', ' ', $file_contents);
		}
		$existing = $cache_asset->getExistingFile();
		string_to_file($file_contents, $existing['path']);

		$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
		$file_status = $fv->upToDate($cache_asset->data_path.'/'.$cache_asset->_getName());
		if (FUDGE_FV_MODIFIED & $file_status) {
			if (!$fv->commit($cache_asset->data_path.'/'.$cache_asset->_getName())) {
				trigger_localised_error('CORE0034', E_USER_WARNING);
				return FALSE;
			}
		}
		$this->_tmp['updated_cache'] = TRUE;
		return $cache_asset->_updated();

	}//end _updateCacheFile()
	
	
	/**
	* Gets the static cache file asset id of this CSS asset
	*
	* @return string
	* @access public
	*/
	public function getCacheFileId()
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$cache_link = $am->getLink($this->id, SQ_LINK_TYPE_3, 'file', true, 'css_cache');
		if (empty($cache_link)) return 0;
		return $cache_link['minorid'];

	}//end getCacheFileId()

	
	/**
	* Creates the cache file asset that this CSS asset will display in the cache
	*
	* @return File
	* @access public
	*/
	public function _createCacheFile()
	{
		$null = NULL;

		// check if we need to add the .css extension to the file
		$filename = $this->_getName();
		if (substr($filename, -4) != '.css') {
			$filename .= '.css';
		}
		if (!is_dir($this->data_path)) {
			create_directory($this->data_path);
		}
		if (!string_to_file('', $this->data_path.'/'.$filename)) {
			return $null;
		}

		$tmp_file = $this->data_path.'/'.$filename;

		$import_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_3, 'value' => 'css_cache', 'sort_order' => 1, 'is_dependant' => 1);
		$temp_info = Array('name' => $filename, 'tmp_name' => $tmp_file, 'non_uploaded_file' => TRUE);

		require_once SQ_CORE_PACKAGE_PATH.'/files/file/file.inc';
		$new_file = new File();
		$new_file->_tmp['uploading_file'] = TRUE;
		$new_file->setAttrValue('name', $filename);
		$new_file->setAttrValue('allow_unrestricted', FALSE);

		if (!$new_file->create($import_link, $temp_info)) {
			trigger_localised_error('CORE0058', E_USER_WARNING, $filename);
			return $null;
		}

		$new_file->saveWebPaths(Array());

		return $new_file;

	}//end _createCacheFile()


	/**
	* Returns the link information for the design area with the given id_name or all design area links
	*
	* @param string	$id_name	the id of the design area to get
	*							if blank returns links for all design areas
	*
	* @return mixed array|NULL
	* @access public
	*/
	public function getDesignAreaLink($id_name='')
	{
		if ($id_name) {
			return @$GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_3, 'design_area', FALSE, $id_name);
		} else {
			return @$GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_3, 'design_area', FALSE);
		}

	}//end getDesignAreaLink()


	/**
	* Paints out our design
	*
	* @param Asset	$ASSET		the asset that we are going to print the body for
	* @param array	$FILE_URLS	an array of filename => URL to use to print files in the design
	*							doesnt need to be passed in unless some URLs are overridden by
	*							a customisation of the design
	* @param boolean $send_header	Whether to send 'css' header when painting
	*
	* @return void
	* @access public
	*/
	public function paint(Asset $ASSET, $FILE_URLS=Array(), $send_header=TRUE)
	{
		// generate an array of all the URLs we are going to need for the files in this design
		$file_link_ids = Array();
		$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'file', FALSE);

		foreach ($file_links as $link) {
			$file_link_ids[] = $link['minorid'];
		}

		$file_link_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_unique($file_link_ids));
		$file_link_urls = $GLOBALS['SQ_SYSTEM']->am->getAssetURL(array_unique($file_link_ids));

		$system_root_url = sq_web_path('root_url');
		foreach ($file_link_info as $minorid => $file_info) {
			$file_url = $file_link_urls[$minorid];
			if ($file_url == '') {
				$file_url = $system_root_url.'/?a='.$minorid;
			} else if ($this->attr('static_url_versioning')){
				$served_by_apache = strpos($file_url, '/__data/') !== FALSE || (SQ_CONF_STATIC_ROOT_URL && strpos($file_url, SQ_CONF_STATIC_ROOT_URL.'/') !== FALSE);
				if ($served_by_apache) $file_url .= '?v='.$file_info['version'];
			}
			$FILE_URLS[$file_info['name']] = $file_url;
		}

		$DATA_PATH = sq_web_path('data'); // this works because we know that all design areas are not going to be restricted
		$LIB_PATH  = sq_web_path('lib');

		// work out where the design file is
		if (SQ_ROLLBACK_VIEW) {
			// get an older version of our design file
			$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
			$rep_file = $this->data_path_suffix.'/design_file.php';
			$then = iso8601_ts($_SESSION['sq_rollback_view']['rollback_time']);
			$info = $fv->_checkOutCheck($rep_file, NULL, $then);
			$file_path = $info['source_file'];
		} else {
			$file_path = $this->data_path.'/design_file.php';
		}

		// design file not found for this asset
		if (!file_exists($file_path)) {
			// use the template file of parent design_css asset
			$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'design_css', FALSE);
			foreach ($parents as $id => $type) {
				$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
				if ($parent) {
					$file_path = $parent->data_path.'/design_file.php';
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent);
					if (file_exists($file_path)) {
						break;
					}
				}
			}
		}

		// Now we just include the design file to get it to print
		if (file_exists($file_path)) {
			// When $NO_HEADER is set, the content file will not send the css header
			// See $this->_getDesignFileHeader()
			if (!$send_header) {
				$NO_HEADER = TRUE;
			}
			require $file_path;
		} else {
			trigger_localised_error('CORE0259', E_USER_WARNING, $ASSET->name);
		}

	}//end paint()


	/**
	* Save attrs to the DB
	*
	* @return void
	* @access public
	*/
	public function saveAttributes()
	{
		if (!parent::saveAttributes()) {
			return FALSE;
		}
		if ($this->id) $this->_setCacheFileEnabled($this->attr('cache_css'));
		return TRUE;

	}//end saveAttributes()


	/**
	* Enable or disable the cache file, by setting or clearing its web paths
	*
	* @param boolean	$enabled	Whether the cache should be enabled
	*
	* @return void
	* @access public
	*/
	public function _setCacheFileEnabled($enabled)
	{
		$cache_id = $this->getCacheFileId();
		if (empty($cache_id)) {
			$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id);
			if (empty($parents)) return;
			$cache_file = $this->_createCacheFile();
			$cache_id = $cache_file->id;
		}

		$cache_url = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($cache_id);
		if (empty($cache_url) == $enabled) {
			// it either has a url but shouldn't or doesn't have a url but should
			$cache_file = $GLOBALS['SQ_SYSTEM']->am->getAsset($cache_id);
			$paths = $enabled ? Array($cache_file->name) : Array();
			$cache_file->saveWebPaths($paths);
			$cache_file->setAttrValue('allow_unrestricted', $enabled);
			$cache_file->saveAttributes();
		}

	}//end _setCacheFileEnabled()



}//end class
?>
