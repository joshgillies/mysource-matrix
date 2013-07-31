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
* $Id: image_variety.inc,v 1.62.8.2 2013/07/31 04:35:21 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Image_Variety
*
* Purpose
*
*
* @author  Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.62.8.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Image_Variety extends Asset
{

	/**
	* the asset ID of the image that the question belongs to
	* @var int
	*/
	var $_imageid = 0;

	/**
	* the shadow ID of the variety - unique for each parent asset
	* @var int
	*/
	var $_varietyid = 0;


	/**
	* Constructor
	*
	* @param string	$assetid    the asset id to be loaded
	* @param Array	$data		the shadow asset data that will construct this variety
	*
	*/
	function Image_Variety($assetid=0, $data=Array())
	{
		$this->Asset();

		if (!$assetid) return;
		list($this->_imageid, $this->_varietyid) = explode(':v', $assetid);

		$image = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return;

		// set the data paths
		$this->data_path        = $image->data_path.'/varieties';
		$this->data_path_suffix = $image->data_path_suffix.'/varieties';
		$this->data_path_public = $image->data_path_public.'/varieties';

		// if there is no varietyid then we haven't been created yet, so don't do anymore
		if (!$this->_varietyid) return;

		$this->id = $assetid;

		foreach ($data as $attr_name => $attr_value) {
			if (!isset($this->vars[$attr_name])) continue;
			$this->vars[$attr_name]['value'] = ($this->vars[$attr_name]['type'] == 'serialise') ? unserialize($attr_value) : $attr_value;
		}

		// set general object properties, now we have the name of the thing
		$this->name = $this->_getName();
		$this->short_name = $this->_getName('short_name');
		$this->status = $image->status;
		$this->version = '0.1';

	}//end constructor


	/**
	* Set the value for the passed variable to the passed value
	* If the user changes the name of this file, we also need
	* to update the file system
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value)
	{
		switch ($name) {
			case 'name' :
				if (trim($this->attr('name')) != '') {
					trigger_localised_error('CORE0116', E_USER_WARNING);
					return FALSE;
				} else if (trim($value) == '') {
					trigger_localised_error('CORE0120', E_USER_WARNING);
				} else {
					// make sure the name is valid for a web path
					require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
					list($value) = make_valid_web_paths(Array(trim($value)));
				}
			break;

			case 'width' :
			case 'height' :
				if ((int) $value < 0) {
					trigger_localised_error('CORE0101', E_USER_WARNING, $name, $this->name, $this->id);
					return FALSE;
				}
				if ((int) $value == 0 && ($this->attr('constrain') == 'none' || ($this->attr('constrain') == $name && $this->attr('variety_type') == 'resize'))) {
					trigger_localised_error('CORE0102', E_USER_WARNING, $name, $this->name, $this->id);
					return FALSE;
				}
				// If constrain = dimension but there's none set, use the value from here
				if ($this->attr('constrain') == 'dimension') {
					$this->setAttrValue('dimension', $value);
				}
			break;

		}// end switch



		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Processes the changes required when a new status is set
	*
	* Ignore the status changes since the bridge handles it.
	*
	* @param int		$new_status		the new status to set for this asset
	* @param boolean	$update_parent	Specify if we want to update the parent as well
	*
	* @return boolean
	* @access public
	*/
	function processStatusChange($new_status, $update_parent=TRUE)
	{
		return TRUE;

	}//end processStatusChange()


	/**
	* Save attributes set with setAttrValue to the database
	*
	* Overriding the asset.inc here to make sure that they are saved to the asset_image_variety
	* table instead of asset_attribute.
	*
	* @param boolean	$dont_run_updated	if TRUE this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	* @see setAttrValue()
	*/
	function saveAttributes($dont_run_updated=FALSE)
	{
		if (!$this->id) return TRUE;
		if (empty($this->_tmp['vars_set'])) return TRUE;

		if (!$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] && !$this->writeAccess('attributes')) {
			trigger_localised_error('CORE0121', E_USER_WARNING, $this->name, $this->id);
			return FALSE;
		}

		if ($this->attr('variety_type') == 'resize' && $this->attr('constrain') != 'none' && $this->attr('constrain') != 'centre_weighted_square') {
			if ((int) $this->attr($this->attr('constrain')) == 0) {
				trigger_localised_error('CORE0096', E_USER_WARNING, $this->name, $this->id, $this->attr('constrain'));
				return FALSE;
			}
		}

		$image = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		return (bool) $image->saveVariety($this);

	}//end saveAttributes()


	/**
	* Set the last updated info for this asset
	*
	* Call this function when the asset has been changed in some way so we can indicated in the
	* asset table when the asset was last changed and who by. This function will also increment the micro
	* version number for the asset.
	*
	* @param boolean	$update_parents	should we go through an update all our dependant parents as well ?
	*									NOTE: this is passed to incrementVersion to let it know that the parents
	*									should have their versions updated as well.
	*
	* @return boolean
	* @access private
	*/
	function _updated($update_parents=TRUE)
	{
		$this->name = $this->_getName();
		$this->short_name = $this->_getName('short_name');

		return TRUE;

	}//end _updated()


	/**
	* Returns TRUE if this type of asset can be cloned
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return FALSE;

	}//end canClone()


	/**
	* return the name of this question
	*
	* @param boolean	$short_name	if TRUE, returns the short name
	*
	* @return string
	* @access private
	*/
	function _getName($short_name=FALSE)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* (Re)Create the image file from our parents original
	*
	* @param boolean	$file_versioning	Whether or not to use file versioning
	*
	* @return boolean
	* @access public
	*/
	function updateImage($file_versioning=TRUE)
	{
		$image = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return FALSE;

		if ($this->attr('variety_type') == 'resize') {

			if ($file_versioning) {
				require_once SQ_FUDGE_PATH.'/general/file_system.inc';
				require_once SQ_FUDGE_PATH.'/image/image_manip.inc';
				$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
			}

			if (!is_dir($this->data_path) && !create_directory($this->data_path)) {
				return FALSE;
			}

			$source_image_info = $image->getExistingFile();
			if (empty($source_image_info)) return TRUE;

			$ext = get_file_type($image->name);

			$s_file = $source_image_info['path'];
			$s_x = 0;
			$s_y = 0;
			$s_width = 0;
			$s_height = 0;

			$d_old_filename = $this->attr('filename');
			$d_new_filename = $this->name.'.'.$ext;
			$d_old_file     = $this->data_path.'/'.$d_old_filename;
			$d_new_file     = $this->data_path.'/'.$d_new_filename;
			$d_width        = (int) $this->attr('width');
			$d_height       = (int) $this->attr('height');
			$d_dimension	= (int) $this->attr('dimension');

			// if either are blank calc the other to scale
			switch ($this->attr('constrain')) {
				case 'width' :
					if ($d_width <= 0) {
						trigger_localised_error('CORE0007', E_USER_WARNING);
						return FALSE;
					}

					$d_height = round((int) $source_image_info['height'] * ($d_width / (int) $source_image_info['width']));
				break;

				case 'height' :
					if ($d_height <= 0) {
						trigger_localised_error('CORE0006', E_USER_WARNING);
						return FALSE;
					}
					$d_width  = round((int) $source_image_info['width']  * ($d_height / (int) $source_image_info['height']));
				break;

				case 'dimension' :
					$old_width = (int) $source_image_info['width'];
					$old_height = (int) $source_image_info['height'];
					$proportion = 1;
					if ($old_width > $old_height) {
						$proportion = $d_dimension / $old_width;
					} else {
						$proportion = $d_dimension / $old_height;
					}
					$d_width = round($old_width * $proportion);
					$d_height = round($old_height * $proportion);
				break;

				case 'centre_weighted_square':
					// The width and height of the original centre weighted square are the smaller of the width and height of the source image
					$s_width = (int) ($source_image_info['width'] < $source_image_info['height'] ? $source_image_info['width'] : $source_image_info['height']);
					$s_height = $s_width;
					// Calculate the top left position of the centre weighted square
					$s_x = (int) (($source_image_info['width'] - $s_width)/2);
					$s_y = (int) (($source_image_info['height'] - $s_height)/2);
					// The width and height of the square are equal
					$d_width = $d_dimension;
					$d_height = $d_width;
					break;
			}

			if ($d_width <= 0 || $d_height <= 0) {
				trigger_localised_error('CORE0005', E_USER_WARNING);
				return FALSE;
			}

			// if the filename has changed then we need to remove the old file
			if ($d_old_filename != $d_new_filename) {
				$this->deleteImage();

			} else {
				if ($file_versioning) {
					// make sure we have the latest version checked out if we are modifying the file
					$up_to_date = $fv->upToDate($d_new_file);
					if ($up_to_date & FUDGE_FV_OLD_VERSION) {
						if (!$fv->checkOut($this->data_path_suffix.'/'.$d_new_filename, $this->data_path)) {
							trigger_localised_error('COR0103', E_USER_WARNING);
							return FALSE;
						}
					}
				}

			}// endif

			require_once(SQ_FUDGE_PATH.'/image/image_manip.inc');
			if (!Image_Manip::resize($s_file, $d_new_file, $d_width, $d_height, $s_x, $s_y, $s_width, $s_height)) {
				return FALSE;
			}

			if ($file_versioning) {
				$up_to_date = $fv->upToDate($d_new_file);

				// file not in repository
				if ($up_to_date & FUDGE_FV_NOT_CHECKED_OUT) {
					if (!$fv->add($this->data_path_suffix, $d_new_file, '')) {
						return FALSE;
					}

					if (!$fv->checkOut($this->data_path_suffix.'/'.$d_new_filename, $this->data_path)) {
						trigger_localised_error('COR0103', E_USER_WARNING);
						return FALSE;
					}

				} else if ($up_to_date & FUDGE_FV_MODIFIED) {
					if (!$fv->commit($d_new_file, '', FALSE)) return FALSE;

				}
			}

			// if the filename has changed then we need to upadte the look ups
			if ($d_old_filename != $d_new_filename) {
				if (!$this->setAttrValue('filename', $d_new_filename)) {
					return FALSE;
				}
			}

		}//end if

		if (!$this->updateLookups($image->usePublicPath())) {
			return FALSE;
		}

		return TRUE;

	}//end updateImage()


	/**
	* Delete's the image file that we have created
	*
	* @return boolean
	* @access public
	*/
	function deleteImage()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		if (!is_dir($this->data_path)) return TRUE;

		$d_filename = $this->attr('filename');
		$d_file     = $this->data_path.'/'.$d_filename;

		if (empty($d_filename)) return TRUE;

		if (!$this->setAttrValue('filename', '')) return FALSE;

		// make sure we have the latest version checked out if we are modifying the file
		$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
		$up_to_date = $fv->upToDate($d_file);

		$updated = $this->updateLookups(FALSE);
		if (!$updated) return FALSE;

		// file not in repository, hmm, something must have gone wrong, let's see if there is a file there and remove it.
		if (($up_to_date & FUDGE_FV_NOT_CHECKED_OUT)) {
			if (file_exists($d_file)) {
				if (!unlink($d_file)) return FALSE;
			}

		} else {
			if (!($fv->remove($d_file, '') & FUDGE_FV_OK)) {
				trigger_localised_error('CORE0041', E_USER_WARNING, $d_filename);
				return FALSE;
			}

		}//end if

		return TRUE;


	}//end deleteImage()


	/**
	* Look's after the placing and removing of out files in the public directory
	*
	* @param boolean	$public	whether we have are allowed to have files in the public dir or not
	*
	* @return boolean
	* @access public
	*/
	function checkFileState($public)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();

		$filename = $this->attr('filename');
		if (!$filename) return TRUE;
		$unrestricted = $this->data_path_public.'/'.$filename;

		// if we should be storing our file in the public
		// data directory for all the world to see
		if ($public) {
			// make sure our data directory exists
			if (!create_directory($this->data_path_public)) {
				return FALSE;
			}

			// if there is no current file or if the current file is out of date, check out a fresh copy
			if ($fv->upToDate($unrestricted) & (FUDGE_FV_NOT_CHECKED_OUT | FUDGE_FV_OLD_VERSION)) {
				if (!$fv->checkOut($this->data_path_suffix.'/'.$filename, $this->data_path_public)) {
					trigger_localised_error('COR0103', E_USER_WARNING);
					return FALSE;
				}
			}
		} else {
			// need to make sure our file is in the NOT in the public data directory
			if (is_file($unrestricted)) {
				if (!$fv->clearOut($unrestricted) & FUDGE_FV_OK) {
					trigger_localised_error('COR0104', E_USER_WARNING);
					return FALSE;
				}
			}
		}//end if

		return TRUE;

	}//end checkFileState()

	
	/**
	* Look's after the removing of files in the public directory
	*
	* @return boolean
	* @access private
	*/
	protected function _removePublicFile()
	{
		
		$filename = $this->attr('filename');
		if (!$filename) return TRUE;
		
		$unrestricted = $this->data_path_public.'/'.$filename;
		if (is_file($unrestricted)) {
			$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
			if (!$fv->clearOut($unrestricted) & FUDGE_FV_OK) {
				trigger_localised_error('COR0104', E_USER_WARNING);
				return FALSE;
			}
		}
		
		return TRUE;

	}//end _removePublicFile()

	
	/**
	* Look's after the updating of files in the public directory
	* Only call if File::usePublicPath() returns TRUE;
	*
	* @return boolean
	* @access private
	*/
	protected function _updatePublicFile()
	{	
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		$filename = $this->attr('filename');
		if (!$filename) return TRUE;
		
		$unrestricted = $this->data_path_public.'/'.$filename;

		// make sure our data directory exists
		if (!create_directory($this->data_path_public)) {
			return FALSE;
		}

		$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
		// if there is no current file or if the current file is out of date, check out a fresh copy
		if ($fv->upToDate($unrestricted) & (FUDGE_FV_NOT_CHECKED_OUT | FUDGE_FV_OLD_VERSION)) {
			if (!$fv->checkOut($this->data_path_suffix.'/'.$filename, $this->data_path_public)) {
				trigger_localised_error('COR0103', E_USER_WARNING);
				return FALSE;
			}
		}
		
		return TRUE;

	}//end _updatePublicFile()
	

	/**
	* Called to force an update of this assets lookup information
	*
	* @return boolean
	* @access public
	* @see Asset::updateLookups(), Asset::saveWebPaths()
	*/
	function updateLookups()
	{
		// if we aren't meant to be having the public paths, let our parent run as normal
		$image = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (!empty($image)) {
			$public = $image->usePublicPath();
		}
		if (!$public) return parent::updateLookups();

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		unset($this->_tmp['lookups']);
		unset($this->_tmp['url']);
		unset($this->_tmp['href']);

		// Remove all the old URLs
		try {
			$bind_vars = Array('assetid' => $this->id);
			MatrixDAL::executeQuery('core', 'deleteLookup', $bind_vars);
		} catch (DALException $e) {
			throw new Exception('Unable to delete lookups of asset "'.$this->name.'" (#'.$this->id.') due to database error: '.$e->getMessage());
		}

		$filename = $this->attr('filename');

		// if there is no filename there is nothing more for us to do
		if (!$filename) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return TRUE;
		}

		$web_path = str_replace(sq_web_path('root_url').'/', '', $this->getWebDataPath()).'/'.$filename;

		// Basically what we are going to do here is find all our parents that are sites
		// and take the system root urls that are closest to these site urls and tack on __data/... bit
		$save_urls = Array();
		$root_urls = explode("\n", trim(SQ_CONF_SYSTEM_ROOT_URLS));

		if (!SQ_CONF_STATIC_ROOT_URL == '') {
			if (!$GLOBALS['SQ_SYSTEM']->am->assetInTrash($this->id, TRUE)){
				// use the static root URL data
				$web_path = str_replace(sq_web_path('data').'/', '', $web_path);
				$save_urls[SQ_CONF_STATIC_ROOT_URL] = Array('http' => SQ_CONF_STATIC_ROOT_HTTP, 'https' => SQ_CONF_STATIC_ROOT_HTTPS);
			}
		} else {

			$site_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'site', FALSE);
			foreach ($site_parents as $assetid => $type_code) {
				$site = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
				if (is_null($site)) continue;
				$urls = $site->getSiteURLS();

				foreach ($urls as $urlid => $data) {

					$root_url = '';
					foreach ($root_urls as $tmp_url) {
						$tmp_url = rtrim($tmp_url, '/');
						if ($data['url'] == $tmp_url || substr($data['url'], 0, strlen($tmp_url.'/')) == $tmp_url.'/') {
							$root_url = $tmp_url;
							continue;
						}
					}
					if (empty($root_url)) continue;

					if (isset($save_urls[$root_url])) {
						if (empty($save_urls[$root_url]['http'])) {
							$save_urls[$root_url]['http']  = $data['http'];
						}
						if (empty($save_urls[$root_url]['https'])) {
							$save_urls[$root_url]['https'] = $data['https'];
						}
					} else {
						$save_urls[$root_url] = Array('http' => $data['http'], 'https' => $data['https']);
					}

				}//end foreach

			}//end foreach

		}//end else - if SQ_CONF_STATIC_ROOT_URL is not empty

		foreach ($save_urls as $url => $url_data) {

			try {
				$bind_vars = Array(
								'url'			=> $url.'/'.$web_path,
								'assetid'		=> $this->id,
								'http'			=> ($this->force_secure === '1') ? '0' : $url_data['http'],
								'https'			=> ($this->force_secure === '-') ? '0' : $url_data['https'],
								'root_urlid'	=> 0,
							 );

				MatrixDAL::executeQuery('core', 'insertLookup', $bind_vars);
			} catch (DALException $e) {
				throw new Exception('Unable to delete lookups of asset "'.$this->name.'" (#'.$this->id.') due to database error: '.$e->getMessage());
			}

			// NOTE: we are not going to update the design's lookups because no
			//       designs can be attached to a file anyway (well not at this time)
			//       and it's one thing less to worry about updating for the moment

		}//end foreach

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		
		//Bug #5072: Trashed file assets still accessible via public data URL
		//We only reached here because we are supposed to have a public web path but
		//if we e.g. have moved to the trash or under a site with no URL, we should remove the public file.
		if (empty($save_urls)){
			$this->_removePublicFile();
		} else {
			$this->_updatePublicFile();
		}
		
		return TRUE;

	}//end updateLookups()


	/**
	* Returns the href for this asset closest to the $base_url, but relative to the current url
	* .
	* @param string	$base_url	the url that we are want to match closest to
	*
	* @return string
	* @access public
	*/
	function getWebDataPath()
	{
		$image = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return FALSE;

		return $image->getWebDataPath().'/varieties';

	}//end getWebDataPath()


	/**
	* Returns a list of paths that this asset has
	*
	* @return array
	* @access public
	*/
	function getWebPaths()
	{
		$filename = $this->attr('filename');
		if ($filename) {
			return Array($filename);
		} else {
			return Array();
		}

	}//end getWebPaths()


	/**
	* Prints out the backend interface for the asset
	*
	* @param object	&$o	Backend_Outputter
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$o)
	{
		if (!isset($_REQUEST['asset_ei_screen'])) {
			return parent::paintBackend($o);
		}
		switch ($_REQUEST['asset_ei_screen']) {
			case 'image_info' :
				$image = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
				if (is_null($image)) exit();

				include_once SQ_FUDGE_PATH.'/var_serialise/var_serialise.inc';

				$filename = $this->data_path.'/'.$this->attr('filename');
				$size = getimagesize($filename);

				$ouput = Array();
				$output['alt']    = $image->attr('alt');
				$output['width']  = $size[0];
				$output['height'] = $size[1];
				$output['name'] = $this->name;
				echo var_serialise($output);
				exit();
			default :
				return parent::paintBackend($o);

		}//end switch

	}//end paintBackend()


	/**
	* Prints out the frontend for this file with no design
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		// if we are in limbo, we want to show our editing interface and design
		// instead of just sending our file over
		// there is an exception to it: In Limbo, if we have ?a=<assetid> at the end of the URL
		// and <assetid> value is different from the current asset assetid we are sending our file over
		if (SQ_IN_LIMBO && !(isset($_REQUEST['a']) && $_REQUEST['a'] == $this->id)) {
			return parent::printFrontend();
		}

		if (!$this->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $this->name));
			return;
		}

		$GLOBALS['SQ_SYSTEM']->setGlobalDefine('SQ_REPLACE_MYSOURCE_LEVEL_KEYWORDS', FALSE);

		$image = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return FALSE;

		if (($this->status & SQ_SC_STATUS_SAFE_EDITING) && ($GLOBALS['SQ_SYSTEM']->user instanceof Public_User)) {
			if (is_file($image->data_path.'/varieties/sq_system_version_no')) {
				include ($image->data_path.'/varieties/sq_system_version_no');
				$file_version = isset($versions[$this->attr('filename')]) ? $versions[$this->attr('filename')] : NULL;
				$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
				$rep_file = $this->data_path_suffix.'/'.$this->attr('filename');
				$info = $fv->_checkOutCheck($rep_file, $file_version);
				
				$filename = $info['filename'];
				$ext = get_file_type($filename);
				$file = $info['source_file'];				
			}
		} else {
			$ext = get_file_type($image->name);
			$filename = $this->name.'.'.$ext;
			$file = $this->data_path.'/'.$filename;
		}

		if (isset($file) && file_exists($file)) {
			require_once SQ_FUDGE_PATH.'/standards_lists/mime_types.inc';
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			$type = (empty($standards_lists_mime_types[$ext])) ? 'text/plain' : $standards_lists_mime_types[$ext];

			// get protocol information
			$ssl_connection = FALSE;
			$url_info = parse_url(current_url());
			$protocol = (isset($url_info['scheme'])) ? $url_info['scheme'] : NULL;
			if (!is_null($protocol) && $protocol == 'https') {
				$ssl_connection = TRUE;
			}

			$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
			$using_ie6 = strpos($_SERVER['HTTP_USER_AGENT'], 'MSIE 6.') !== FALSE;

			// send cacheable headers if file asset meets the credentials
			if (basename($_SERVER['PHP_SELF']) != SQ_CONF_NOCACHE_SUFFIX && SQ_CONF_SEND_CACHEABLE_HEADER && ($GLOBALS['SQ_SYSTEM']->user instanceof Public_User) && empty($_POST) && $cm->cacheableHeadersEnabledForCurrentProtocol() && $this->status >= SQ_STATUS_LIVE && $this->readAccess()) {
					header('Cache-Control: '.$cm->cacheControlLevel());
					header('Pragma: cache');
					header('Expires: '.gmdate('D, d M Y H:i:s', (time()+86400)).' GMT');
					header('Last-Modified: '.gmdate('D, d M Y H:i:s', filemtime($file)).' GMT');
			} else if (!$using_ie6 || !$ssl_connection) {
					header('Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0');
					header('Pragma: no-cache');
					header('Expires: '.gmdate('D, d M Y H:i:s', time()-3600).' GMT');
			} else {
					// internet explorer has a problem with SSL connection (https)
					// cant send no-cache header or we will get "cannot download file" error
					// http://support.microsoft.com/default.aspx?scid=kb;en-us;812935
					// http://support.microsoft.com/default.aspx?scid=kb;en-us;316431
					header('Cache-Control: max-age=0, must-revalidate');
					header('Pragma: private');
					header('Expires: '.gmdate('D, d M Y H:i:s', time()-3600).' GMT');
			}

			header('Content-Type: '.$type);
			header('Content-Disposition: inline; filename='.$filename.';');
			header('Content-Length: '.filesize($file));

			readfile($file);
			@ob_flush();
		}//end if

	}//end printFrontend()


	/**
	* Prints the tag for an image
	*
	* The output includes the width and height of the image, and the image
	* name for the alt tag
	*
	* @param string		$alt	the alt tag content. If empty the alt var is used
	* @param string		$title	the alt tag content. If empty the and $alt is set, then that is used, otherwise the title var is used
	* @param boolean	$return	if TRUE the tag will be returned
	*
	* @return mixed string|void
	* @access public
	*/
	function printImageTag($alt, $title, $return=FALSE)
	{
		$filename = $this->attr('filename');
		if (!$filename) return;
		$size = getimagesize($this->data_path.'/'.$filename);

		$tag = '<img src="'.$this->getURL().'" '.$size[3].' alt="'.$alt.'" title="'.$title.'" />';
		if ($return) return $tag;
		echo $tag;

	}//end printImageTag()


	/**
	* Get bodycopy keyword replacement
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function getKeywordReplacement($keyword)
	{
		$replacement = NULL;
		
		// Remove any modifiers from keyword
		$full_keyword = $keyword;
		$keyword = parse_keyword($keyword, $modifiers);
		$contextid = extract_context_modifier($modifiers);

		if ($contextid !== NULL) {
			// If we were able to extract a context ID to change to, and it's
			// different to our current one, then change and then reload a copy
			// of the asset in that context (as we would need to do anyway)

			if ((int)$contextid !== $GLOBALS['SQ_SYSTEM']->getContextId()) {
				$GLOBALS['SQ_SYSTEM']->changeContext($contextid);
				$contexted_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id);

				// Get the keyword without any modifiers
				$replacement = $contexted_asset->getKeywordReplacement($keyword);

				// Then apply the ones we had remaining, then return it
				apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $contexted_asset->id));
				
				unset($contexted_asset);
				$GLOBALS['SQ_SYSTEM']->restoreContext();
				return $replacement;

			}//end if contextid is not the currently active context

		}//end if contextid is not NULL

		// additional shadow asset keyword replacement
		// Note: using the full keyword is NOT wanted here, since the modifiers
		// are still being applied below.
		$replacement = parent::getKeywordReplacement($keyword);
		if (empty($replacement)) {
			 switch ($keyword) {
				case 'asset_name_linked' :
					$replacement = '<a href="'.$this->getUrl().'">'.$this->name.'</a>';
				break;
				case 'asset_contents' :
					$replacement = $this->printImageTag($this->name, $this->name, TRUE);
				break;
				default :

				break;
			}
		}

		if (count($modifiers) > 0) {
			apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $this->id));
		}

		return $replacement;

	}//end getKeywordReplacement()


//--        PERMISSIONS/ACCESS (DEFERS TO IMAGE)        --//


	/**
	* Returns TRUE if at least one of the passed assetids has read access to this asset
	*
	* @param array	$assetids	An array of user_group and/or user ids to check for read access
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function readAccess($assetids=Array())
	{
		if (!$this->id) return TRUE;

		$image = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return FALSE;

		return $image->readAccess($assetids);

	}//end readAccess()


	/**
	* Returns TRUE if at least one of the passed assetids has write access to this asset
	*
	* @param string	$lock_type	the type of write access required - if blank returns a plain check against
	*							permissions set for this asset, otherwise returns effective access based on locking
	*							and status
	* @param array	$assetids	An array of user_group and/or user ids to check for write access
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function writeAccess($lock_type='', $assetids=Array())
	{
		if (!$this->id) return TRUE;

		$image = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return FALSE;

		return $image->writeAccess($lock_type, $assetids);

	}//end writeAccess()


	/**
	* Returns TRUE if at least one of the passed assetids has admin access to this asset
	*
	* @param string	$lock_type	the type of write access required - if blank returns a plain check against
	*							permissions set for this asset, otherwise returns effective access based on locking
	*							and status
	* @param array	$assetids	an array of user_group and/or user ids to check for write access
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function adminAccess($lock_type='', $assetids=Array())
	{
		if (!$this->id) return TRUE;

		$image = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return FALSE;

		return $image->adminAccess($lock_type, $assetids);

	}//end adminAccess()


	/**
	* Returns TRUE if the current user can live edit this asset
	* Live edit means the asset is currently SQ_STATUS_LIVE but this
	* user can edit the asset without going through a safe edit
	* NOTE: will return FALSE if asset is not currently live
	*
	* @param string	$lock_type	Type of lock to check
	*
	* @return boolean
	* @access public
	* @see canliveEdit()
	*/
	function liveEditAccess($lock_type)
	{
		$image = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return FALSE;

		return $image->liveEditAccess($lock_type);

	}//end liveEditAccess()


	/**
	* Returns TRUE if at least one of the passed assetids has the passed permission
	* Can perform effective access check based on current status, lock, workflow, and metadata
	* NOTE: Only for write and above access
	*
	* @param int	$perm		the permission to check access for
	* @param string	$lock_type	the type of write access required - if blank returns a plain check against
	*							permissions set for this asset, otherwise returns effective access based on locking
	*							and status
	* @param array	$assetids	An array of user_group and/or user ids to check for permission
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function checkAccess($perm, $lock_type, $assetids=Array())
	{
		$image = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return FALSE;

		return $image->checkAccess($perm, $lock_type, $assetids);

	}//end checkAccess()


	/**
	* Returns TRUE if at least one of the passed assetids has the passed permission
	* Does not check current status of lock
	*
	* @param int	$perm		the permission to check access for
	* @param array	$assetids	An array of user_group and/or user ids to check for permission
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function _checkPermissionAccess($perm, $assetids=Array())
	{
		$image = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return FALSE;

		return $image->_checkPermissionAccess($perm, $assetids);

	}//end _checkPermissionAccess()


}//end class

?>
