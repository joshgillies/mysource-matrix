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
* $Id: image_variety.inc,v 1.3 2005/01/19 05:00:20 gsherwood Exp $
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
* @version $Revision: 1.3 $
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

		if (!$assetid || empty($data)) return;
		$this->id = $assetid;

		list($this->_imageid, $this->_varietyid) = explode(':v', $this->id);

		$image = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return;

		// set the data paths
		$this->data_path        = $image->data_path.'/varieties';
		$this->data_path_suffix = $image->data_path_suffix.'/varieties';
		$this->data_path_public = $image->data_path_public.'/varieties';

		foreach($data as $attr_name => $attr_value) {
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
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value)
	{
		if ($name == 'name' && trim($this->attr('name')) != '') {
			trigger_error('You cannot change the name of a variety', E_USER_WARNING);
			return false;
		}

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Save attributes set with setAttrValue to the database
	*
	* Overriding the asset.inc here to make sure that they are saved to the asset_image_variety
	* table instead of asset_attribute.
	*
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	* @see setAttrValue()
	*/
	function saveAttributes($dont_run_updated=false)
	{
		if (!$this->id) return true;
		if (empty($this->_tmp['vars_set'])) return true;

		if (!$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] && !$this->writeAccess('attributes')) {
			trigger_error('You do not have permission to save the attributes of "'.$this->name.'" (#'.$this->id.')', E_USER_WARNING);
			return false;
		}

		if ((int) $this->attr('width') < 0 || (int) $this->attr('height') < 0 || ((int) $this->attr('width') == 0 && (int) $this->attr('height') == 0)) {
			trigger_error('At least one of the height or width needs to be greater than zero', E_USER_WARNING);
			return false;
		}

		$image = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		return $image->saveVariety($this);

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
	function _updated($update_parents=true)
	{
		$this->name = $this->_getName();
		$this->short_name = $this->_getName('short_name');

		return true;

	}//end _updated()


	/**
	* Returns TRUE if this type of asset can be cloned
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return false;

	}//end canClone()


	/**
	* return the name of this question
	*
	* @param boolean $short_name if TRUE, returns the short name
	*
	* @access private
	* @return string
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Create the image file from our parents original
	*
	* @access public
	* @return boolean
	*/
	function createImage()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		require_once SQ_FUDGE_PATH.'/image/image_manip.inc';

		if (!is_dir($this->data_path) && !create_directory($this->data_path)) return false;

		$image = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return false;

		$source_image_info = $image->getExistingFile();
		if (empty($source_image_info)) return true;

		$ext = get_file_type($image->name);

		$s_file = $source_image_info['path'];

		$d_filename = $this->name.'.'.$ext;
		$d_file     = $this->data_path.'/'.$d_filename;
		$d_width    = (int) $this->attr('width');
		$d_height   = (int) $this->attr('height');

		if ($d_width < 0 || $d_height < 0 || ($d_width == 0 && $d_height == 0)) {
			trigger_error('At least one of the height or width needs to be greater than zero', E_USER_WARNING);
			return false;
		}

		// if either are blank calc the other to scale
		if     ($d_width  == 0) $d_width  = round((int) $source_image_info['width']  * ($d_height / (int) $source_image_info['height']));
		elseif ($d_height == 0) $d_height = round((int) $source_image_info['height'] * ($d_width  / (int) $source_image_info['width']));


		// make sure we have the latest version checked out if we are modifying the file
		$fv = &$GLOBALS['SQ_SYSTEM']->getFileVersioning();
		$up_to_date = $fv->upToDate($d_file);

		if ($up_to_date & FUDGE_FV_OLD_VERSION) {
			if (!$fv->checkOut($this->data_path_suffix.'/'.$d_filename, $this->data_path)) {
				trigger_error('Update failed checking-out latest version of file', E_USER_WARNING);
				return false;
			}
		}

		if (!Image_Manip::resize($s_file, $d_file, $d_width, $d_height)) return false;

		$up_to_date = $fv->upToDate($d_file);

		// file not in repository
		if ($up_to_date & FUDGE_FV_NOT_CHECKED_OUT) {
			if (!$fv->add($this->data_path_suffix, $d_file, '', false)) return false;

			if (!$fv->checkOut($this->data_path_suffix.'/'.$d_filename, $this->data_path)) {
				trigger_error('Update failed checking-out latest version of file', E_USER_WARNING);
				return false;
			}

		} elseif ($up_to_date & FUDGE_FV_MODIFIED) {
			if (!$fv->commit($d_file, '', false)) return false;

		}

		return true;


	}// end createImage()


	/**
	* Delete's the image file that we have created
	*
	* @access public
	* @return boolean
	*/
	function deleteImage()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		if (!is_dir($this->data_path)) return true;

		$image = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return false;

		$source_image_info = $image->getExistingFile();

		$ext = get_file_type($image->name);
		$d_filename = $this->name.'.'.$ext;
		$d_file     = $this->data_path.'/'.$d_filename;

		// make sure we have the latest version checked out if we are modifying the file
		$fv = &$GLOBALS['SQ_SYSTEM']->getFileVersioning();
		$up_to_date = $fv->upToDate($d_file);

		$this->_tmp['deleting_image'] = true;
		$updated = $this->updateLookups();
		$this->_tmp['deleting_image'] = false;
		if (!$updated) return false;

		// file not in repository, hmm, something must have gone wrong, let's see if there is a file there and remove it.
		if (($up_to_date & FUDGE_FV_NOT_CHECKED_OUT)) {
			if (file_exists($d_file)) {
				if (!unlink($d_file)) return false;
			}

		} else {
			if (!($fv->remove($d_file, '', false) & FUDGE_FV_OK)) {
				trigger_error('Failed removing "'.$d_filename.'" from the repository', E_USER_WARNING);
				return false;
			}

		}//end if

		return true;


	}// end deleteImage()


	/**
	* Look's after the placing and removing of out files in the public directory
	*
	* @param boolean $public	whether we have are allowed to have files in the public dir or not
	*
	* @return boolean
	* @access public
	*/
	function checkFileState($public)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		$image = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return false;

		$fv = &$GLOBALS['SQ_SYSTEM']->getFileVersioning();

		$ext = get_file_type($image->name);

		$filename = $this->name.'.'.$ext;
		$unrestricted = $this->data_path_public.'/'.$filename;

		// if we should be storing our file in the public
		// data directory for all the world to see
		if ($public) {
			// make sure our data directory exists
			if (!create_directory($this->data_path_public)) return false;

			// if there is no current file or if the current file is out of date, check out a fresh copy
			if ($fv->upToDate($unrestricted) & (FUDGE_FV_NOT_CHECKED_OUT | FUDGE_FV_OLD_VERSION)) {
				if (!$fv->checkOut($this->data_path_suffix.'/'.$filename, $this->data_path_public)) {
					trigger_error('Update failed checking-out latest version of file', E_USER_WARNING);
					return false;
				}
			}
		} else {
			// need to make sure our file is in the NOT in the public data directory
			if (is_file($unrestricted)) {
				if (!$fv->clearOut($unrestricted) & FUDGE_FV_OK) {
					trigger_error('Update failed deleting file from public data directory', E_USER_WARNING);
					return false;
				}
			}
		}//end if

		return true;

	}//end checkFileState()


	/**
	* Called to force an update of this assets lookup information
	*
	* @param boolean $public		whether we have are allowed to have files in the public dir or not
	*
	* @return boolean
	* @access public
	* @see Asset::updateLookups(), Asset::saveWebPaths()
	*/
	function updateLookups($public=false)
	{
		// if we aren't meant to be having the public paths, let our parent run as normal
		if (!$public) {
			return parent::updateLookups();
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		unset($this->_tmp['lookups']);
		unset($this->_tmp['url']);
		unset($this->_tmp['href']);

		// Remove all the old URLs

		$sub_sql = 'SELECT url
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup l
					'.$GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause('assetid = '.$db->quote($this->id), 'l');


		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('ast_lookup_design', 'url IN ('.$sub_sql.')')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$where_cond = 'assetid = '.$db->quote($this->id);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('ast_lookup', $where_cond)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// if we are deleting the image, then there is no need to add to the urls
		if (!empty($this->_tmp['deleting_image'])) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		}


		$image = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return false;

		$ext = get_file_type($image->name);
		$filename = $this->name.'.'.$ext;

		$web_path = str_replace(sq_web_path('root_url').'/', '', $this->getWebDataPath()).'/'.$filename;

		// Basically what we are going to do here is find all our parents that are sites
		// and take the system root urls that are closest to these site urls and tack on __data/... bit
		$save_urls = Array();
		$root_urls = explode("\n", trim(SQ_CONF_SYSTEM_ROOT_URLS));

		$site_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'site', false);
		foreach ($site_parents as $assetid => $type_code) {
			$site = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
			if (is_null($site)) continue;
			$urls = $site->getSiteURLS();

			foreach ($urls as $urlid => $data) {

				$root_url = '';

				foreach ($root_urls as $tmp_url) {
					if (substr($data['url'], 0, strlen($tmp_url)) == $tmp_url) {
						$root_url = $tmp_url;
						continue;
					}
				}

				if (empty($root_url)) continue;

				if (isset($save_urls[$root_url])) {
					if (empty($save_urls[$root_url]['http']))  $save_urls[$root_url]['http']  = $data['http'];
					if (empty($save_urls[$root_url]['https'])) $save_urls[$root_url]['https'] = $data['https'];
				} else {
					$save_urls[$root_url] = Array('http' => $data['http'], 'https' => $data['https']);
				}

			}//end foreach

		}//end foreach


		foreach ($save_urls as $url => $url_data) {
			$values = Array(
						'url'        => $url.'/'.$web_path,
						'assetid'    => $this->id,
						'http'       => ($this->force_secure) ? '0' : $url_data['http'],
						'https'      => $url_data['https'],
						'root_urlid' => 0,
					);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('ast_lookup', $values)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			// NOTE: we are not going to update the design's lookups because no
			//       designs can be attached to a file anyway (well not at this time)
			//       and it's one thing less to worry about updating for the moment

		}//end foreach

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end updateLookups()


	/**
	* Returns the href for this asset closest to the $base_url, but relative to the current url
	*
	* @param string	$base_url	the url that we are want to match closest to
	*
	* @return string
	* @access public
	*/
	function getWebDataPath()
	{
		$image = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return false;

		return $image->getWebDataPath().'/varieties';

	}//end getWebDataPath()


	/**
	* Returns a list of paths that this asset has
	*
	* @return Array()
	* @access public
	*/
	function getWebPaths()
	{
		// if we are deleting the image....don't pass the paths back because we don't have any any more
		if (!empty($this->_tmp['deleting_image'])) return Array();
		if (!isset($this->_tmp['paths'])) {

			$image = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
			if (is_null($image)) return false;

			$ext = get_file_type($image->name);
			$this->_tmp['paths'] = Array($this->name.'.'.$ext);

		}// end if

		return $this->_tmp['paths'];

	}//end getWebPaths()


	/**
	* Get an array of some useful data about the existing uploaded file
	*
	* @return array(string)
	* @access private
	*/
	function getExistingFile()
	{
		$image = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return Array();

		$ext = get_file_type($image->name);
		$filename = $this->name.'.'.$ext;
		$old_name = $this->attr('name');

		// we are going to temporarily set the name of this image
		// variety to have an extension, so it can be found by the File
		// asset's getExistingFile() function
		$this->vars['name']['value'] = $filename;

		$info = image::getExistingFile();
		if (!empty($info['path'])) {
			$size = getimagesize($info['path']);
			$info['width']  = $size[0];
			$info['height'] = $size[1];
		}

		// change the name back to what it was
		$this->vars['name']['value'] = $old_name;

		return $info;

	}//end getExistingFile()


	/**
	* Prints out the backend interface for the asset
	*
	* @param object Backend_Outputter	&$o
	*
	* @access public
	*/
	function paintBackend(&$o)
	{
		if (!isset($_REQUEST['asset_ei_screen'])) return parent::paintBackend($o);
		switch ($_REQUEST['asset_ei_screen']) {
			case 'image_info' :
				$image = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
				if (is_null($image)) exit();

				$file_info = $this->getExistingFile();

				include_once SQ_FUDGE_PATH.'/var_serialise/var_serialise.inc';
				$ouput = Array();
				$output['alt'] = $image->attr('alt');
				$output['width'] = $file_info['width'];
				$output['height'] = $file_info['height'];
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
		if (SQ_IN_LIMBO) return parent::printFrontend();

		if (!$this->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to access <i>'.$this->name.'</i>');
			return;
		}

		$image = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return false;

		$ext = get_file_type($image->name);
		$filename = $this->name.'.'.$ext;
		$file = $this->data_path.'/'.$filename;

		if (file_exists($file)) {
			require_once SQ_FUDGE_PATH.'/standards_lists/mime_types.inc';
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			$type = (empty($standards_lists_mime_types[$ext])) ? 'text/plain' : $standards_lists_mime_types[$ext];

			header('Cache-Control:');
			header('Pragma: cache');
			header('Expires: '. gmdate('D, d M Y H:i:s', (time() + 86400)) . ' GMT');
			header('Content-Type: ' . $type);

			header('Content-Disposition: inline; filename='.$filename.';');
			header('Last-Modified: ' . gmdate('D, d M Y H:i:s', filemtime($file)) . ' GMT');
			header('Content-Length: '.filesize($file));
			readfile($file);

		}

	}//end printFrontend()


//--        PERMISSIONS/ACCESS (DEFERS TO IMAGE)        --//


	/**
	* Returns true if at least one of the passed assetids has read access to this asset
	*
	* @param array	$assetids	An array of user_group and/or user ids to check for read access
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function readAccess($assetids=Array())
	{
		if (!$this->id) return true;

		$image = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return false;

		return $image->readAccess($assetids);

	}//end readAccess()


	/**
	* Returns true if at least one of the passed assetids has write access to this asset
	*
	* @param string	$lock_type	the type of write access required - if blank returns a plain check against
	*							permissions set for this asset, otherwise returns effective access based on locking
	*							and status
	* @param Array	$assetids	An array of user_group and/or user ids to check for write access
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function writeAccess($lock_type='', $assetids=Array())
	{
		if (!$this->id) return true;

		$image = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return false;

		return $image->writeAccess($lock_type, $assetids);

	}//end writeAccess()


	/**
	* Returns true if at least one of the passed assetids has admin access to this asset
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
		if (!$this->id) return true;

		$image = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return false;

		return $image->adminAccess($lock_type, $assetids);

	}//end adminAccess()


	/**
	* Returns true if the current user can live edit this asset
	* Live edit means the asset is currently SQ_STATUS_LIVE but this
	* user can edit the asset without going through a safe edit
	* NOTE: will return false if asset is not currently live
	*
	* @return boolean
	* @access public
	* @see canliveEdit()
	*/
	function liveEditAccess($lock_type)
	{
		$image = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return false;

		return $image->liveEditAccess($lock_type);

	}//end liveEditAccess()


	/**
	* Returns true if at least one of the passed assetids has the passed permission
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
		$image = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return false;

		return $image->checkAccess($perm, $lock_type, $assetids);

	}//end checkAccess()


	/**
	* Returns true if at least one of the passed assetids has the passed permission
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
		$image = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_imageid);
		if (is_null($image)) return false;

		return $image->_checkPermissionAccess($perm, $assetids);

	}//end _checkPermissionAccess()


}//end class

?>