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
* $Id: file.inc,v 1.57.2.2 2004/03/03 19:39:18 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


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
* @package MySource_Matrix_Packages
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
	* Processes the file upload from the source file so that file versioning cloned properly
	*
	* @param object asset &$clone the clone asset shell
	* @param array			$components	the wanted components to clone eg.
	*                       Array(
	*                           'attributes',
	*                           'metadata_shemas',
	*                           'metadata',
	*                           'workflow',
	*                           'permissions',
	*                           'data'
	*                       );
	*                       or alternately
	*                       Array('all');
	* 
	* @return boolean
	* @access public
	* @see asset::cloneComponents()
	*/
	function cloneComponents(&$clone, $components)
	{
		// create some information so we know where to clone the file from
		$temp_info = Array('name' => $this->name, 'tmp_name' => $this->data_path.'/'.$this->name, 'non_uploaded_file' => true);

		$edit_fns = $clone->getEditFns();
		if (!$edit_fns->processFileUpload($clone, $GLOBALS['SQ_SYSTEM']->backend->out, $clone->getPrefix(), $temp_info)) {
			trigger_error('Could not clone file '.$this->name.' (#'.$this->id.')');
			return false;
		}

		return parent::cloneComponents($clone, $components);
	
	}//end cloneComponents()


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return Array()
	* @access public
	*/
	function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['attr_links'] = ($lock_types['attributes'] | $lock_types['links']);
		return $lock_types;

	}//end lockTypes()

	
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

		// update our lookup tables to not use __data
		if (!parent::updateLookups()) {
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
		// pretend we are uploading a file because we may be changing the name of the file
		// and we want to fool setAttributeValue
		$this->_tmp['uploading_file'] = true;
		if (!parent::revertToSystemVersion()) {
			$this->_tmp['uploading_file'] = false;
			return false;
		}
		$this->_tmp['uploading_file'] = false;

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		// clear the private data directory to get rid of the new uploaded file we had there
		if (!clear_directory($this->data_path)) {
			trigger_error('Failed reverting file "'.$this->name.'" to system version, could not clear restricted data directory', E_USER_WARNING);
			return false;
		}

		// check out the latest version of the file into the private data directory
		$fv = &$GLOBALS['SQ_SYSTEM']->getFileVersioning();
		$existing = $this->_getExistingFile();
		if (!$fv->checkOut($this->data_path_suffix.'/'.$existing['filename'], $this->data_path)) {
			trigger_error('Failed reverting file "'.$this->name.'" to system version, could not check-out latest version of file', E_USER_WARNING);
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
		$am = &$GLOBALS['SQ_SYSTEM']->am;

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

		// NOTE : no need to update the files states or the lookups here because they are done in _updated() if a file exists

		// update manually
		unset($this->_tmp['IGNORE_UPDATE']);
		if (!$this->_updated()) { 
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end processStatusChange()


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	* @param array()	$info	info about the uploaded file
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link, $info=Array())
	{
		$name = trim($this->attr('name'));

		if ($name == '') {
			trigger_error('Unable to create a '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').' with a blank name', E_USER_WARNING);
			return false;
		}

		// check that a file was uploaded at least and that it succeeded
		if (empty($info)) $info = get_file_upload_info($this->getPrefix());
		if ($info === false || empty($info)) {
			trigger_error('File upload failed. Unable to create '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'), E_USER_WARNING);
			return false;
		}

		// check that we are not going to have web path conflicts
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_names = make_valid_web_paths(Array($info['name']));
		$name = array_shift($valid_names);

		// make sure the new web path is not already is use
		$bad_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], Array($name));
		if (!empty($bad_paths)) {
			trigger_error('Unable to create file with name "'.$name.'", web path already exists?', E_USER_WARNING);
			return false;
		}

		$this->setAttrValue('name', $name);

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

		if ($this->status & SQ_SC_STATUS_SAFE_EDITING || !empty($this->_tmp['IGNORE_UPDATE'])) {
			return true;
		}

		return $this->_checkFileState();

	}//end _updated()


	/**
	* Called when the any permission is updated for this asset

	* @return boolean	whether everything is OK
	* @access public
	*/
	function permissionsUpdated()
	{
		return $this->_checkFileState();

	}// end permissionsUpdated()


	/**
	* Look's after the placing and removing of out files in the public directory
	*
	* @return boolean
	* @access private
	*/
	function _checkFileState()
	{
		$existing = $this->_getExistingFile();
		if (empty($existing)) return true;

		$unrestricted = $this->data_path_public.'/'.$existing['filename'];

		$changed = false;

		// if we should be storing our file in the public
		// data directory for all the world to see
		if ($this->effectiveUnrestricted()) {
			// make sure our data directory exists
			if (!create_directory($this->data_path_public)) {
				trigger_error('Failed saving asset "'.$this->name.'" to cache, could not create restricted data directory', E_USER_WARNING);
				return false;
			}

			$fv = &$GLOBALS['SQ_SYSTEM']->getFileVersioning();
			// if there is no current file or if the current file is out of date, check out a fresh copy
			if ($fv->upToDate($unrestricted) & (SQ_FV_NOT_CHECKED_OUT | SQ_FV_OLD_VERSION)) {
				if ($fv->checkOut($this->data_path_suffix.'/'.$existing['filename'], $this->data_path_public)) {
					$changed = true;
				} else {
					trigger_error('Update failed checking-out latest version of file', E_USER_WARNING);
					return false;
				}
			}
		} else {
			// need to make sure our file is in the NOT in the public data directory
			if (is_file($unrestricted)) {
				$fv = &$GLOBALS['SQ_SYSTEM']->getFileVersioning();
				if ($fv->clearOut($unrestricted) & SQ_FV_OK) {
					$changed = true;
				} else {
					trigger_error('Update failed deleting file from public data directory', E_USER_WARNING);
					return false;
				}
			}
		}//end if effectiveUnrestricted

		if ($changed) {
			return $this->updateLookups();
		} else {
			return true;
		}

	}//end _checkFileState()


	/**
	* Called to force an update of this assets lookup information
	*
	* @return boolean
	* @access private
	* @see Asset::updateLookups(), Asset::saveWebPaths()
	*/
	function updateLookups()
	{
		if (!$this->effectiveUnrestricted() || ($this->status & SQ_SC_STATUS_SAFE_EDITING)) {
			// this file is either in a safe edit or restricted
			// so we dont worry about __data paths
			return parent::updateLookups();
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		unset($this->_tmp['lookups']);
		unset($this->_tmp['url']);
		unset($this->_tmp['href']);

		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		// Remove all the old URLs
		$subs = Array( 'SELECT url
						FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_lookup l
						'.$GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause('assetid = '.$db->quote($this->id), 'l'));

		$where = 'url IN (~SQ0~)';
		$where = db_extras_subquery($db, $where, $subs);
		if (DB::isError($where)) {
			trigger_error($where->getMessage().'<br/>'.$where->getUserInfo(), E_USER_ERROR);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_lookup_design', $where)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$where_cond = 'assetid = '.$db->quote($this->id);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_lookup', $where_cond)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$existing = $this->_getExistingFile();
		if (!isset($existing['filename'])) {
			// there is no filename, so commit the removal of the current web paths and return
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		}

		$web_path = $this->getWebDataPath().'/'.$existing['filename'];

		// Basically what we are going to do here is find all our parents that are sites
		// and take the system root urls that are closest to these site urls and tack on __data/... bit
		$save_urls = Array();
		$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
		$site_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'site', false);
		foreach ($site_parents as $assetid) {
			$site = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if (is_null($site)) continue;
			$urls = $site->getSiteURLS();
			foreach($urls as $urlid => $data) {

				$root_url = '';

				foreach($root_urls as $tmp_url) {
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

			}// end foreach

		}// end foreach

		foreach ($save_urls as $url => $url_data) {
			$values = Array(
						'url'        => $url.'/'.$web_path,
						'assetid'    => $this->id,
						'http'       => ($this->force_secure) ? '0' : $url_data['http'],
						'https'      => $url_data['https'],
						'root_urlid' => 0,
					);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_lookup', $values)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			// NOTE: we are not going to update the design's lookups because no
			//       designs can be attached to a file anyway (well not at this time)
			//       and it's one thing less to worry about updating for the moment

		}// end foreach

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end updateLookups()


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

				$prefix = $this->getPrefix();
				$this->setAttrValue('name', 'temp');

				$linkid = $this->create($link);

				if ($linkid && $GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'attributes')) {
					$edit = $this->getEditFns();
					$this->_tmp['uploading_file'] = 1;
					$ret_val = $edit->processFileUpload($this, $o, $prefix);
					$this->saveAttributes();
					$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'attributes');
					return $ret_val;
				}

				return false;
			break;

			default :
				return parent::processBackend($o, $link);

		}// end switch

	}// end processBackend()


	/**
	* Set the value of an attribute
	*
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	*/
	function saveAttributes($dont_run_updated=false)
	{
		if (!$this->id) return true;

		if (isset($this->_tmp['vars_set']['name'])) {

			$uploading_file = (!empty($this->_tmp['uploading_file'])) ? true : false;

			if (!$uploading_file) {

				$old_filename = $this->_tmp['vars_set']['name']['old_value'];
				$new_filename = $this->attr('name');

				if (is_file($this->data_path.'/'.$old_filename)) $data_path = $this->data_path;
				else $data_path = $this->data_path_public;

				// try and set the new filename - return false if it fails
				if (!parent::saveAttributes($dont_run_updated)) return false;

				// the name has changed so we need to save the web paths
				if (!$this->saveWebPaths(Array($new_filename))) {
					trigger_error('Failed updating web path. Path already exists?', E_USER_WARNING);
					parent::setAttrValue('name', $old_filename);
					return false;
				}

				//// FILE VERSIONING ////

				$fv = &$GLOBALS['SQ_SYSTEM']->getFileVersioning();

				// copy the file instead of a direct rename because removing the
				// file from the repository will also remove it from the file system
				if (!copy($data_path.'/'.$old_filename, $data_path.'/'.$new_filename)) {
					trigger_error('Failed renaming file, try again.', E_USER_WARNING);
					parent::setAttrValue('name', $old_filename);
					return false;
				}

				// we are renaming the file, so we need to remove the old name from
				// the repository and add the new one in
				if (!$fv->remove($data_path.'/'.$old_filename)) {
					trigger_error('Failed removing file from repository', E_USER_WARNING);
					parent::setAttrValue('name', $old_filename);
					return false;
				}

				// attempt to add a new version of the file to the repository
				if (!$fv->add($this->data_path_suffix, $this->data_path.'/'.$new_filename)) {
					trigger_error('Failed adding file version', E_USER_WARNING);
					parent::setAttrValue('name', $old_filename);
					return false;
				}

				// make sure we have the latest version of our file
				if (!$fv->checkOut($this->data_path_suffix.'/'.$new_filename, $this->data_path)) {
					trigger_error('Failed checking out latest version of file', E_USER_WARNING);
					parent::setAttrValue('name', $old_filename);
					return false;
				}

				return true;

			}//end if not uploading a file

		}//end if we are changing the filename

		return parent::saveAttributes($dont_run_updated);

	}//end saveAttributes()


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
		$uploading_file = (!empty($this->_tmp['uploading_file'])) ? true : false;

		if ($name == 'name' && !$uploading_file) {
			if (is_file($this->data_path.'/'.$this->attr('name'))) $data_path = $this->data_path;
			else $data_path = $this->data_path_public;

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

		return parent::setAttrValue($name, $value);

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
	* Returns the URL for this asset, closest to the $base_url
	*
	* @param string		$base_url			the url that we are want to match closest to
	* @param boolean	$ignore_rollback	ignore rollback checks in asset manager?
	*										this should only get set by the asset manager itself
	*
	* @return string
	* @access public
	*/
	function getURL($base_url=null, $ignore_rollback=false)
	{
		if (SQ_ROLLBACK_VIEW) {
			if (is_null($base_url)) $base_url = current_url();
			$base_url = strip_url($base_url, false);
			return $base_url.'/?a='.$this->id;
		} else {
			return parent::getURL($base_url, $ignore_rollback);
		}

	}//end getURL()


	/**
	* Returns the href for this asset closest to the $base_url, but relative to the current url
	*
	* @param string		$base_url			the url that we are want to match closest to
	* @param boolean	$ignore_rollback	ignore rollback checks in asset manager?
	*										this should only get set by the asset manager itself
	*
	* @return string
	* @access public
	*/
	function getHref($base_url=null, $ignore_rollback=false)
	{
		if (SQ_ROLLBACK_VIEW) return './?a='.$this->id;
		else return parent::getHref($base_url, $ignore_rollback);

	}//end getHref()


	/**
	* Get an array of some useful data about the existing uploaded file
	*
	* @return array(string)
	* @access public
	*/
	function _getExistingFile()
	{
		$existing = Array();

		if (SQ_ROLLBACK_VIEW) {
			// get an older version of this file
			$fv = &$GLOBALS['SQ_SYSTEM']->getFileVersioning();
			$rep_file = $this->data_path_suffix.'/'.$this->name;
			$then = iso8601_ts($_SESSION['sq_rollback_view']['rollback_time']);
			$info = $fv->_checkOutCheck($rep_file, null, $then);
			$path = $info['source_file'];
		} else {
			// get the latest file
			$path = $this->data_path.'/'.$this->attr('name');
		}

		if (is_file($path)) {
			$existing['path'] = $path;
			$existing['filename'] = $this->name;
			$existing['modified'] = filemtime($existing['path']);
			$existing['size'] = filesize($existing['path']);
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