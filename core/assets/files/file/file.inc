<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* File
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class File extends Asset
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function File($assetid=0)
	{
		$this->Asset($assetid);

	}//end constructor


	/**
	* Saves a copy of this asset at this point in time
	* This copy gets used when the asset is live and the user does not have write access
	*
	* @return boolean
	* @access public
	*/
	function saveSystemVersion()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		// make sure our data directory exists
		if (!create_directory($this->data_path)) {
			trigger_error('Failed saving asset "'.$this->name.'" to cache, could not create restricted data directory', E_USER_WARNING);
			return false;
		}

		// make sure our system directory exists
		if (!create_directory($this->data_path.'/.sq_system')) {
			trigger_error('Failed saving asset "'.$this->name.'" to cache, could not create restricted system directory', E_USER_WARNING);
			return false;
		}

		// make sure there is nothing in the system directory
		if (!clear_directory($this->data_path.'/.sq_system')) {
			trigger_error('Failed saving asset "'.$this->name.'" to cache, could not clear restricted system directory', E_USER_WARNING);
			return false;
		}

		// save the object for later (in the restricted directory)
		if (!string_to_file(serialize($this), $this->data_path.'/.sq_system/.object_data')) {
			trigger_error('Failed saving asset "'.$this->name.'" to cache, could not write object data file', E_USER_WARNING);
			return false;
		}

		// copy our file over to the restricted directory so we can work on it in security
		$existing = $this->_getExistingFile();
		$restricted = $this->data_path.'/'.$existing['filename'];
		$unrestricted = $this->data_path_public.'/'.$existing['filename'];
		if (!copy_file($unrestricted, $restricted)) {
			trigger_error('Failed saving file "'.$this->name.'" to cache, file copy to unrestricted data directory failed', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// now we update our lookup tables to not use __data
		if (!parent::_updateLookups()) {
			trigger_error('Failed saving file "'.$this->name.'" to cache, failed updating lookups', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		return true;

	}//end saveSystemVersion()


	/**
	* Replaces the editing version with the system (live) version
	*
	* @return boolean
	* @access public
	*/
	function revertToSystemVersion()
	{
		// pretend we are uploading a file because we may be
		// changing the name of the file
		$this->_tmp['uploading_file'] = true;
		if (!parent::revertToSystemVersion()) {
			$this->_tmp['uploading_file'] = false;
			return false;
		}
		$this->_tmp['uploading_file'] = false;

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		// clear the restricted data directory to get rid of the new
		// uploaded file we had there
		if (!clear_directory($this->data_path)) {
			trigger_error('Failed reverting file "'.$this->name.'" to system version, could not clear restricted data directory', E_USER_WARNING);
			return false;
		}

		return true;

	}//end revertToSystemVersion()


	/**
	* Morph's this asset to either one of it's parent type or one of it's children types
	* Only succeeds if $type_code asset's is allowed to link to all children that this asset has
	*
	* @param string	$new_type_code	the new type_code to attempt to cast this asset to
	*
	* @return bool
	* @access public
	*/
	function morph($new_type_code)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$file_types = array_merge($am->getTypeAncestors($this->type(), false), $am->getTypeDescendants($this->type(), false));
		
		if (in_array($new_type_code, $file_types)) {
			// we are morphing into another file type, so check that the actual
			// file currently uploaded is not going to conflict with the new type
			$am->includeAsset($new_type_code);
			$tmp = new $new_type_code();
			$existing = $this->_getExistingFile();
			if (!empty($existing)) {
				if (!$tmp->validFile($existing['path'])) {
					trigger_error('Cannot morph "'.$this->name.'" to type "'.$new_type_code.'", currently uploaded file "'.$existing['filename'].'" is not a valid '.$new_type_code.' file (see above error)', E_USER_WARNING);
					return false;
				}
			}
			unset($tmp);
		}

		return parent::morph($new_type_code);
	}


	/**
	* Processes the changes required when a new status is set
	*
	* @param int	$new_status		the new status to set for this asset
	*
	* @return boolean
	* @access public
	*/
	function processStatusChange($new_status)
	{
		// let ourselves know not to do any extra updating
		$this->_tmp['IGNORE_UPDATE'] = true;
		$old_status = $this->status;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		
		if (!parent::processStatusChange($new_status)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		if ($this->status == SQ_STATUS_LIVE && $old_status != SQ_STATUS_LIVE_APPROVAL) {
			// if we have just gone live, so we need to move the latest version 
			// of the file to the unrestricted directory to make it live
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			
			// clear out our unrestricted data directory
			if (is_dir($this->data_path_public)) {
				if (!clear_directory($this->data_path_public)) {
					trigger_error('Failed processing status change for file "'.$this->name.'", could not clear public data directory', E_USER_WARNING);
					return false;
				}
			} else {
				if (!create_directory($this->data_path_public)) {
					trigger_error('Failed processing status change for file "'.$this->name.'", could not created public data directory', E_USER_WARNING);
					return false;
				}
			}

			$existing = $this->_getExistingFile();
			$restricted = $this->data_path.'/'.$existing['filename'];
			$unrestricted = $this->data_path_public.'/'.$existing['filename'];

			// and move our file over
			if (!move_file($restricted, $unrestricted)) {
				trigger_error('Failed processing status change for file "'.$this->name.'", could move file to public data directory', E_USER_WARNING);
				return false;
			}
		}

		// update our lookups to use __data again
		$this->_updateLookups();

		// update manually
		unset($this->_tmp['IGNORE_UPDATE']);
		$this->_updated();

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end processStatusChange()


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		$name = trim($this->attr('name'));

		if ($name == '') {
			trigger_error('Unable to create a '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').' with a blank name', E_USER_WARNING);
			return false;
		}

		// check that a file was uploaded at least and that it succeeded
		$ei = $this->getEI();
		$info = get_file_upload_info($ei->getPrefix($this));
		if ($info === false || empty($info)) {
			trigger_error('File upload failed. Unable to create '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'), E_USER_WARNING);
			return false;
		}

		// check that we are not going to have web path conflicts
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_names = makeValidWebPaths(Array($info['name']));
		$name = array_shift($valid_names);

		// make sure the new web path is not already is use
		$bad_paths = $link['asset']->_webPathsInUse(Array($name));
		if (!empty($bad_paths)) {
			trigger_error('Unable to create file, web path already exists?', E_USER_WARNING);
			return false;
		}

		return parent::create($link);

	}//end create()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_TYPE_1 => Array(),
					 SQ_LINK_TYPE_2 => Array(),
					 SQ_LINK_TYPE_3 => Array('thumbnail' => Array('card' => 1, 'exclusive' => false)),
					 SQ_LINK_NOTICE => Array()
					);

	}//end _getAllowedLinks()


	/**
	* Set's the last updated info for this asset
	*
	* @return boolean
	* @access private
	*/
	function _updated()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		if (!parent::_updated()) return false;

		if ($this->status & SQ_SC_STATUS_SAFE_EDITING || isset($this->_tmp['IGNORE_UPDATE'])) {
			unset($this->_tmp['IGNORE_UPDATE']);
			return true;
		}

		$existing = $this->_getExistingFile();
		if (!empty($existing)) {
			$restricted = $this->data_path.'/'.$existing['filename'];
			$unrestricted = $this->data_path_public.'/'.$existing['filename'];

			if ($this->effectiveUnrestricted()) {
				// we should be storing our file in the unrestrcited
				// data directory for all the world to see
				// so if our file is in the restricted directory... we need to move it
				if (is_file($restricted)) {
					if (!move_file($restricted, $unrestricted)) {
						trigger_error('Update failed moving file from restricted to unrestricted', E_USER_WARNING);
						return false;
					}
				}
			} else {
				// need to make sure our file is in the restricted directory
				if (is_file($unrestricted)) {
					if (!move_file($unrestricted, $restricted)) {
						trigger_error('Update failed moving file from unrestricted to restricted', E_USER_WARNING);
						return false;
					}
				}
			}//end if effectiveUnrestricted

			$this->_updateLookups();

		}// end if

		return true;

	}//end _updated()


	/**
	* Fn that does the real processing of updateLookups()
	* This is separated to allow for the DB transaction stuff
	* Will be called by parent assets recursively on their kids so not 'really' private
	* and by the saveWebPaths() so that their transaction calls can wrap around it
	*
	* @return boolean
	* @access private
	* @see Asset::updateLookups(), Asset::saveWebPaths()
	*/
	function _updateLookups()
	{
		if (!$this->effectiveUnrestricted() || ($this->status & SQ_SC_STATUS_SAFE_EDITING)) {
			// this file is either in a safe edit or restricted
			// so we dont worry about __data paths
			return parent::_updateLookups();
		}

		$existing = $this->_getExistingFile();
		$web_path = $this->getWebDataPath().'/'.$existing['filename'];

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		unset($this->_tmp['lookups']);
		unset($this->_tmp['url']);
		unset($this->_tmp['href']);

		// Remove all the old URLs
		$where = 'assetid = '.$db->quote($this->id);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_lookup', $where)) {
			return false;
		}

		$save_urls = Array();

		$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);

		$sql = 'SELECT urlid, url FROM sq_asset_url';
		$base_urls = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_ASSOC);
		if (DB::isError($base_urls)) {
			trigger_error($base_urls->getMessage().'<br/>'.$base_urls->getUserInfo(), E_USER_ERROR);
			return false;
		}

		$parents = $this->getLinks(SQ_SC_LINK_SIGNIFICANT, '', null, 'minor');
		for($i = 0; $i < count($parents); $i++) {
			$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parents[$i]['majorid'], $parents[$i]['type_code']);
			if (is_null($parent)) continue;

			$parent_urls = $parent->getLookups();

			for($j = 0; $j < count($parent_urls); $j++) {
				$urlid = $parent_urls[$j]['root_urlid'];
				if (!isset($base_urls[$urlid])) continue;
				$base_url = $base_urls[$urlid];

				// now find the root url that matches the base url
				foreach ($root_urls as $root_url) {
					// strip out the http[s] business
					$root_url = preg_replace('|http[s]?://|', '', $root_url);
					if (strpos($base_url, $root_url) === 0) break;
					else $root_url = '';
				}

				// now $root_url has the closest url or is blank
				if (trim($root_url) == '') continue;

				$save_urls[$root_url.'/'.$web_path] = $parent_urls[$j];
			}
		}

		foreach ($save_urls as $url => $url_data) {
			$values = Array('url'        => $url,
							'assetid'    => $this->id,
							'root_urlid' => $url_data['root_urlid'],
							'designid'   => '0');
			if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_lookup', $values)) {
				return false;
			}
		}

		return true;

	}//end _updateLookups()


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
		switch($_POST['asset_action']) {
			case 'create' :
				$this->_tmp['uploading_file'] = 1;
				$ei = &$this->getEI();
				if (!$ei->process($this, $o, true)) return false;

				$prefix = $ei->getPrefix($this);

				$this->setAttrValue('name', 'temp');
				$linkid = $this->create($link);

				if ($linkid) {
					$edit = $this->getEditFns();
					unset($this->_tmp['uploading_file']);
					return $edit->processFileUpload($this, $o, $prefix);
				} else {
					return false;
				}
				break;

			default :
				return parent::processBackend($o, $link);

		}// end switch

	}// end processBackend()


	/**
	* Set the value for the passed variable to the passed value
	* If the user changes the name of this file, we also need
	* to update the file system
	*
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run 
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value, $dont_run_updated=false)
	{
		if (is_file($this->data_path.'/'.$this->attr('name'))) {
			$data_path = $this->data_path;
		} else {
			$data_path = $this->data_path_public;
		}
	
		$uploading_file = (!empty($this->_tmp['uploading_file'])) ? true : false;
		if ($name == 'name' && !$uploading_file) {
			$old_filename = $this->attr('name');
			$old_ext = strtolower(substr(strrchr($old_filename,'.'), 1));
			$new_ext = strtolower(substr(strrchr($value,'.'), 1));
			if (trim($old_ext) != '' && $old_ext != $new_ext) {
				trigger_error('You can not change the extension of the file. Upload a new file to change the extension.', E_USER_WARNING);
				$value = $old_filename;
				return false;
			}
			if (!is_file($data_path.'/'.$old_filename)) {
				trigger_error('Current file not found. Upload a file before renaming.', E_USER_WARNING);
				$value = $old_filename;
				return false;
			}
		}

		if (parent::setAttrValue($name, $value, $dont_run_updated) && $name == 'name' && !$uploading_file) {
			if (!$this->saveWebPaths(Array($value))) {
				trigger_error('Failed updating web path. Path already exists?', E_USER_WARNING);
				parent::setAttrValue($name, $old_filename, $dont_run_updated);
				return false;
			}
			if (!rename($data_path.'/'.$old_filename, $data_path.'/'.$value)) {
				trigger_error('Failed renaming file, try again.', E_USER_WARNING);
				parent::setAttrValue($name, $old_filename, $dont_run_updated);
				return false;
			}
		}

		return true;

	}//end setAttrValue()


	/**
	* Checks a file on the file system to see if it can be used as a file
	*
	* @param string	$file_path	the path of the file we are checking
	*
	* @return boolean
	* @access public
	*/
	function validFile($file_path)
	{
		return true;

	}//end validFile()


	/**
	* Prints out the frontend for this file with no design
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		if (!$this->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to access <i>'.$this->name.'</i>');
			return;
		}

		$existing = $this->_getExistingFile();
		if (!empty($existing)) {
			require_once SQ_FUDGE_PATH.'/standards_lists/mime_types.inc';
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			$ext = get_file_type($existing['filename']);
			$type = (empty($standards_lists_mime_types[$ext])) ? 'text/plain' : $standards_lists_mime_types[$ext];
			header('Cache-Control:');
			header('Pragma: cache');
			header('Expires: '. gmdate('D, d M Y H:i:s', (time() + 86400)) . ' GMT');
			header('Content-Type: ' . $type);
			header('Content-Disposition: inline; filename='.$existing['filename'].';');
			header('Last-Modified: ' . gmdate('D, d M Y H:i:s',$existing['modified']) . ' GMT');
			header('Content-Length: '.$existing['size']);
			readfile($existing['path']);
		}

	}// end printFrontend()


	/**
	* Get an array of some useful data about the existing uploaded file
	*
	* @return array(string)
	* @access public
	*/
	function _getExistingFile()
	{
		// check for an existing file
		$existing = Array();
		if (is_file($this->data_path.'/'.$this->attr('name'))) {
			$path = $this->data_path.'/'.$this->attr('name');
		} else {
			$path = $this->data_path_public.'/'.$this->attr('name');
		}

		if (is_file($path)) {
			$existing['path'] = $path;
			$existing['filename'] = $this->attr('name');
			$existing['modified'] = filemtime($existing['path']);
			$existing['size'] = filesize($existing['path']);
			$size = getimagesize($path);
			$existing['width'] = $size[0];
			$existing['height'] = $size[1];
		}

		return $existing;

	}//end _getExistingFile()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');

	}//end _getName()


}//end class

?>