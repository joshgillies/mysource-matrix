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
* $Id: file.inc,v 1.221 2013/10/17 05:34:55 cupreti Exp $
*
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
* @version $Revision: 1.221 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class File extends Asset
{


	/**
	* List of file extensions this asset type can handle. Empty means no restriction.
	* @var Array(string)
	*/
	public $allowed_extensions = Array();


	/**
	 * Flag used in processStatusChange() to delay file system updates in _updated()
	 * @var boolean
	 */
	public $ignore_update = FALSE;


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function File($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* Folders ensure a name has been set before they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _preCreateCheck(Array &$link)
	{
		if (!parent::_preCreateCheck($link)) return FALSE;

		$name = trim($this->attr('name'));
		if ($name == '') {
			trigger_localised_error('CORE0083', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
			return FALSE;
		}

		return TRUE;

	}//end _preCreateCheck()


	/**
	* Create this asset
	*
	* @param array	&$link	information used to create the initial link
	* @param array	$info	info about the uploaded file
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	public function create(Array &$link, $info=Array())
	{
		$this->_tmp['file_create_data'] =& $info;

		if (empty($info)) {
			// Try getting the uploaded file details
			$info = get_file_upload_info($this->getPrefix(), isset($info['non_uploaded_file']) && $info['non_uploaded_file']);
		}

		if ($info === FALSE || empty($info)) {
			// See if they've chosen a pre-uploaded file
			$edit = $this->getEditFns();
			$info = $edit->getChosenFileInfo($this->getPrefix());
		}

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_DATA_VALIDATION)) {
			if ($info === FALSE || empty($info)) {
				trigger_localised_error('CORE0056', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
				return FALSE;
			}
		}

		// Make the info array look like a result from getExistingFile()
		if (!isset($info['path'])) {
			$info['path'] = array_get_index($info, 'tmp_name', '');
		}
		if (!isset($info['filename'])) {
			$info['filename'] = array_get_index($info, 'name', '');
		}

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_DATA_VALIDATION)) {
			if (!$this->validFile($info)) return FALSE;
		}

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_DATA_VALIDATION)) {
			// Check that we are not going to have web path conflicts
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$valid_names = make_valid_web_paths(Array($info['name']));
			$name = array_shift($valid_names);

			// Make sure the new web path is not already is use
			if (!defined('SQ_IN_IMPORT')) {
				$bad_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], Array($name));
				if (!empty($bad_paths)) {
					trigger_localised_error('CORE0086', E_USER_WARNING, $name);
					return FALSE;
				}
			}

			$this->setAttrValue('name', $name);
			//creating the asset? set the allow_unrestricted to true ...which is default value anyways
			if (!isset($this->_tmp['vars_set']['allow_unrestricted'])) $this->setAttrValue('allow_unrestricted', TRUE);

		}//end if

		return parent::create($link);

	}//end create()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* File assets commit their uploaded file to the file system
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		// Save return value of acquireLock(), if it is 1 then lock was newly acquired
		// and we have to give it back at the end, if it is 2 then it's already taken
		// and we should leave it be
		if (!$lock_held = $GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'attributes')) {
			return FALSE;
		}

		$edit = $this->getEditFns();

		// we need to fudge a few attributes to make sure we can
		// process the file upload in the middle of creation
		$this->_tmp['uploading_file'] = 1;
		$prefix = $this->type().'_0';
		$o = NULL;
		if ((!is_null($GLOBALS['SQ_SYSTEM']->backend)) && (!is_null($GLOBALS['SQ_SYSTEM']->backend->out))) {
			$o = $GLOBALS['SQ_SYSTEM']->backend->out;
		}
		$info = $this->_tmp['file_create_data'];

		$ret_val = $edit->processFileChooser($this, $o, $prefix, TRUE) || $edit->processFileUpload($this, $o, $prefix, $info, FALSE);

		$this->saveAttributes();

		if ($lock_held == 1) {
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'attributes');
		}

		return $ret_val;

	}//end _createAdditional()


	/**
	* A public interface for _createAdditional.
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	public function createAdditional(Array &$link)
	{
		return $this->_createAdditional($link);

	}//end createAdditional()


	/**
	* Processes the file upload from the source file so that file versioning cloned properly
	*
	* @param Asset		$clone		the clone asset shell
	* @param array		$components	the wanted components to clone eg.
	* <pre>
	* 						Array(
	* 							'attributes',
	* 							'metadata_schemas',
	* 							'metadata',
	* 							'workflow',
	* 							'permissions',
	* 							'data',
	* 							'content_tags',
	* 							'roles',
	* 						);
	* 						or alternately
	* 						Array('all');
	* </pre>
	* @param boolean	$override	whether or not to override the existing permission, metadata schemas, workflow schemas with the new ones.
	*
	* @return boolean
	* @access public
	* @see asset::cloneComponents()
	*/
	public function cloneComponents(Asset $clone, Array $components, $override=FALSE)
	{
		$GLOBALS['SQ_CLONE_COMPONENTS'] = TRUE;

		// create some information so we know where to clone the file from
		$temp_info = Array('name' => $this->name, 'tmp_name' => $this->data_path.'/'.$this->name, 'non_uploaded_file' => TRUE);

		$edit_fns = $clone->getEditFns();
		// fix for bug 3625:
		// we make sure to tell the processFileChooser function that we are cloning and not creating new asset
		if (!$edit_fns->processFileUpload($clone, $GLOBALS['SQ_SYSTEM']->backend->out, $clone->getPrefix(), $temp_info,TRUE, TRUE)) {
			$GLOBALS['SQ_CLONE_COMPONENTS'] = FALSE;
			trigger_localised_error('CORE0010', E_USER_WARNING, $this->name, $this->id);
			return FALSE;
		}

		$GLOBALS['SQ_CLONE_COMPONENTS'] = FALSE;

		return parent::cloneComponents($clone, $components, $override);

	}//end cloneComponents()


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
	* Saves a copy of this asset at this point in time
	* This copy gets used when the asset is live and the user does not have write access
	*
	* @return boolean
	* @access public
	*/
	public function saveSystemVersion()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		// make sure our data directory exists
		if (!create_directory($this->data_path)) {
			trigger_localised_error('CORE0049', E_USER_WARNING, $this->name);
			return FALSE;
		}

		// make sure our system directory exists
		if (!create_directory($this->data_path.'/.sq_system')) {
			trigger_localised_error('CORE0050', E_USER_WARNING, $this->name);
			return FALSE;
		}

		// make sure there is nothing in the system directory
		if (!clear_directory($this->data_path.'/.sq_system')) {
			trigger_localised_error('CORE0046', E_USER_WARNING, $this->name);
			return FALSE;
		}

		// save the object for later (in the restricted directory)
		if (!string_to_file(serialize($this), $this->data_path.'/.sq_system/.object_data')) {
			trigger_localised_error('CORE0051', E_USER_WARNING, $this->name);
			return FALSE;
		}

		$contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
		foreach ($contexts as $contextid => $context_data) {
			if ($contextid === 0) {
				$files_to_copy = Array('metadata.php', 'metadata_field_values.php');
			} else {
				$files_to_copy = Array('metadata.'.$contextid.'.php', 'metadata_field_values.'.$contextid.'.php');
			}
			foreach ($files_to_copy as $filename) {
				if (file_exists($this->data_path.'/'.$filename) === TRUE) {
					if (!copy_file($this->data_path.'/'.$filename, $this->data_path.'/.sq_system/'.$filename)){
						trigger_localised_error('SYS0166', E_USER_WARNING, $this->name, $filename);
						return FALSE;
					}
				}
			}
		}

		// update our lookup tables to not use __data
		if (!parent::updateLookups()) {
			trigger_localised_error('CORE0053', E_USER_WARNING, $this->name);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}
		// still insert the __data url for safe edit
		if($this->attr('allow_unrestricted') && $this->effectiveUnrestricted()) {
		    $this->insertDataLookup();
		}

		// store the current version of the file
		$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
		$file_info = $fv->_checkOutCheck($this->data_path_suffix.'/'.$this->name);
		$latest_version = $file_info['version'];
		if (!string_to_file($latest_version, $this->data_path.'/.sq_system/sq_system_version_no')) {
			trigger_localised_error('CORE0051', E_USER_WARNING, $this->name);
			return FALSE;
		}
		
		//  we need to save the current notice links
		if(!$this->saveNoticeLinksForSystemVersion()) return FALSE;

		return TRUE;

	}//end saveSystemVersion()


	/**
	* Replaces the editing version with the system (live) version
	*
	* @return boolean
	* @access public
	*/
	public function revertToSystemVersion()
	{
		// get the safe edit file name before it's lost
		$se_name = $this->attr('name');

		// pretend we are uploading a file because we may be changing the name of the file
		// and we want to fool setAttributeValue
		$this->_tmp['uploading_file'] = TRUE;
		if (!parent::revertToSystemVersion()) {
			$this->_tmp['uploading_file'] = FALSE;
			return FALSE;
		}
		$this->_tmp['uploading_file'] = FALSE;

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		$file_version = NULL;
		if (is_file($this->data_path.'/sq_system_version_no')) {
			$file_version = file_to_string($this->data_path.'/sq_system_version_no');
		}

		// clear the private data directory to get rid of the new uploaded file we had there
		// but we should not clear the meta data files (see Bug #4460)
		if (!clear_directory($this->data_path, Array('metadata.php', 'metadata_field_values.php'))){
			trigger_localised_error('CORE0045', E_USER_WARNING, $this->name);
			return FALSE;
		}

		$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();

		// we want to remove the safe edit file name from the repository because
		// there's a chance someone will try and use that name later
		if ($this->attr('name') != $se_name) {
			$se_rep_path = $this->data_path_suffix.'/'.$se_name;
			$result = $fv->remove($se_rep_path, '');
			if ($result != FUDGE_FV_OK && $result != FUDGE_FV_NOT_CHECKED_OUT) {
				trigger_localised_error('CORE0042', E_USER_WARNING);
				return FALSE;
			}
			$removed_file_info = $fv->_checkOutCheck($se_rep_path);
			$fv->_updateFile($removed_file_info['fileid'], $this->data_path_suffix, $this->data_path.'/'.$se_name);
		}

		// check out the old version of the file into the private data directory
		$rep_path = $this->data_path_suffix.'/'.$this->name;
		if (!$fv->checkOut($rep_path, $this->data_path, $file_version)) {
			trigger_localised_error('CORE0044', E_USER_WARNING, $this->name);
			return FALSE;
		}

		// create a new versioning entry for the old version
		$file_info = $fv->_checkOutCheck($rep_path, $file_version);
		$fv->_updateFile($file_info['fileid'], $this->data_path_suffix, $this->data_path.'/'.$this->name);

		// check out the new version
		if (!$fv->checkOut($rep_path, $this->data_path)) {
			trigger_localised_error('CORE0044', E_USER_WARNING, $this->name);
			return FALSE;
		}

		return TRUE;

	}//end revertToSystemVersion()


	/**
	* Clear the directory for saving the system version
	*
	* @return boolean
	* @access public
	*/
	public function clearSystemVersion()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		// Bug #5084: After safe edit, old files remain in public data directory.
		// So clean it out, latest version will be checked out from _checkFileState() function.
		// make sure our data directory exists
		if (!create_directory($this->data_path_public)) {
			trigger_localised_error('CORE0049', E_USER_WARNING, $this->name);
			return FALSE;
		}

		if (!clear_directory($this->data_path_public)) {
			trigger_localised_error('CORE0305', E_USER_WARNING, $this->name);
			return FALSE;
		}

		return parent::clearSystemVersion();

	}//end clearSystemVersion()


	/**
	* Morph's this asset to either one of it's parent type or one of it's children types
	* Only succeeds if $type_code asset's is allowed to link to all children that this asset has
	*
	* @param string	$new_type_code	the new type_code to attempt to cast this asset to
	*
	* @return boolean
	* @access public
	*/
	public function morph($new_type_code)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;

		$file_types = array_merge($am->getTypeAncestors($this->type(), FALSE), $am->getTypeDescendants($this->type()));

		if (in_array($new_type_code, $file_types)) {
			// we are morphing into another file type, so check that the actual
			// file currently uploaded is not going to conflict with the new type
			$am->includeAsset($new_type_code);
			$tmp = new $new_type_code();
			$existing = $this->getExistingFile();
			if (!empty($existing)) {
				if (!$tmp->validFile($existing)) {
					trigger_localised_error('CORE0008', E_USER_WARNING, $this->name, $new_type_code, $existing['filename'], $new_type_code);
					return FALSE;
				}

				$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
				$fv->changeFvTypeCode($existing['path'], $new_type_code);
			}
			unset($tmp);
		}

		return parent::morph($new_type_code);

	}//end morph()


	/**
	* Processes the changes required when a new status is set
	*
	* @param int	$new_status	the new status to set for this asset
	* @param boolean	$update_parents	Specify if we want to update the parent as well
	* @param boolean	$run_updated	specify it we want to run _upated
	*
	* @return boolean
	* @access public
	*/
	public function processStatusChange($new_status, $update_parents=TRUE, $run_updated=TRUE)
	{
		// let ourselves know not to do any extra updating
		$this->ignore_update = TRUE;

		// event data to be passed to the trigger action
		$old_status = $this->status;
		$this->_tmp['old_status'] = $old_status;
		$this->_tmp['old_urls'] = $this->getURLs();
		$this->_tmp['allow_unrestricted'] = $this->attr('allow_unrestricted');

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!parent::processStatusChange($new_status, $update_parents, $run_updated)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// NOTE : no need to update the files states or the lookups here because they are done in _updated() if a file exists

		// update manually
		$this->ignore_update = FALSE;
		if (!$this->_updated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end processStatusChange()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return void
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_1	=> Array(),
				SQ_LINK_TYPE_2	=> Array(),
				SQ_LINK_TYPE_3	=> Array(),
				SQ_LINK_NOTICE	=> Array(
									'asset'		=> Array(
													'card'		=> 'M',
													'exclusive'	=> FALSE,
												   ),
								   ),
			   );

	}//end _getAllowedLinks()


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
	public function _updated($update_parents=TRUE)
	{
		if (!parent::_updated($update_parents)) return FALSE;

		if ($this->ignore_update == TRUE) {
			return TRUE;
		}
		
		if($this->status & SQ_SC_STATUS_SAFE_EDITING) {
		    $this->updateLookups();
		    return TRUE;	
		}

		return $this->_checkFileState();

	}//end _updated()


	/**
	* Called when the any permission is updated for this asset
	* Returns TRUE if everything is OK
	*
	* @return boolean
	* @access public
	*/
	public function permissionsUpdated()
	{
		if ($this->status & SQ_SC_STATUS_SAFE_EDITING) {
		    $this->updateLookups();
		    return TRUE; //don't touch public file in safe edit
		}
		return $this->_checkFileState();

	}//end permissionsUpdated()


	/**
	* Look's after the placing and removing of out files in the public directory
	*
	* @param string 	$filename		The filename to check the state of
	* @param boolean 	$update_lookups	Whether to update the lookups
	*
	* @return boolean
	* @access private
	*/
	protected function _checkFileState($filename='', $update_lookups=TRUE)
	{

		if ($this->status & SQ_SC_STATUS_SAFE_EDITING) return TRUE; //don't touch public file in safe edit

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		if (empty($filename)) {
			$existing = $this->getExistingFile();
			if (empty($existing)) return TRUE;
			$filename = $existing['filename'];
		}

		$unrestricted = $this->data_path_public.'/'.$filename;
		$changed = FALSE;

		// if we should be storing our file in the public
		// data directory for all the world to see
		if ($this->effectiveUnrestricted() && $this->attr('allow_unrestricted')) {
			// make sure our data directory exists
			if (!create_directory($this->data_path_public)) {
				trigger_localised_error('CORE0049', E_USER_WARNING, $this->name);
				return FALSE;
			}

			$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
			// if there is no current file or if the current file is out of date, check out a fresh copy
			if ($fv->upToDate($unrestricted) & (FUDGE_FV_NOT_CHECKED_OUT | FUDGE_FV_OLD_VERSION)) {
				if (file_exists($unrestricted)) {
					unlink($this->data_path_public.'/'.$filename);
				}

				if ($fv->checkOut($this->data_path_suffix.'/'.$filename, $this->data_path_public)) {
					$changed = TRUE;
				} else {
					trigger_localised_error('CORE0032', E_USER_WARNING);
					return FALSE;
				}
			} else {
				// Because we are live, and we might be be escalated from EDITING_APPROVED, so lets update lookup to get the public url back
				$changed	= TRUE;
			}//end else
		} else {
			// need to make sure our file is NOT in the public data directory
			if (is_file($unrestricted)) {
				$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
				if ($fv->clearOut($unrestricted) & FUDGE_FV_OK) {
					$changed = TRUE;
				} else {
					trigger_localised_error('CORE0104', E_USER_WARNING);
					return FALSE;
				}
			}
		}//end if effectiveUnrestricted

		if ($changed && $update_lookups) {
			return $this->updateLookups();
		} else {
			return TRUE;
		}

	}//end _checkFileState()


	/**
	* Look's after the removing of files in the public directory
	*
	* @return boolean
	* @access private
	*/
	protected function _removePublicFile()
	{
		$existing = $this->getExistingFile();
		if (empty($existing)) return TRUE;

		$unrestricted = $this->data_path_public.'/'.$existing['filename'];
		if (is_file($unrestricted)) {
			$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
			if ($fv->clearOut($unrestricted) & FUDGE_FV_OK) {
				return TRUE;
			} else {
				trigger_localised_error('CORE0104', E_USER_WARNING);
				return FALSE;
			}
		}

		return TRUE;

	}//end _removePublicFile()


	/**
	* Look's after the updating of files in the public directory.
	* Only call if File::usePublicPath() returns TRUE;
	*
	* @return boolean
	* @access private
	*/
	protected function _updatePublicFile()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		$existing = $this->getExistingFile();
		if (empty($existing)) return TRUE;

		$unrestricted = $this->data_path_public.'/'.$existing['filename'];

		// make sure our data directory exists
		if (!create_directory($this->data_path_public)) {
			trigger_localised_error('CORE0049', E_USER_WARNING, $this->name);
			return FALSE;
		}

		$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
		// if there is no current file or if the current file is out of date, check out a fresh copy
		if ($fv->upToDate($unrestricted) & (FUDGE_FV_NOT_CHECKED_OUT | FUDGE_FV_OLD_VERSION)) {
			if (file_exists($unrestricted)) {
				unlink($this->data_path_public.'/'.$existing['filename']);
			}

			if ($fv->checkOut($this->data_path_suffix.'/'.$existing['filename'], $this->data_path_public)) {
				return TRUE;
			} else {
				trigger_localised_error('CORE0032', E_USER_WARNING);
				return FALSE;
			}
		}

		return TRUE;

	}//end _updatePublicFile()


	/**
	* Returns whether to use the public path or not for this file
	*
	* @return boolean
	* @access public
	* @see File::updateLookups()
	*/
	public function usePublicPath()
	{
		return ($this->attr('allow_unrestricted') && $this->effectiveUnrestricted() && !($this->status & SQ_SC_STATUS_SAFE_EDITING));

	}//end usePublicPath()


	/**
	* Called to force an update of this assets lookup information
	*
	* @return boolean
	* @access public
	* @see Asset::updateLookups(), Asset::saveWebPaths()
	*/
	public function updateLookups($auto_add_remaps = TRUE)
	{
		$old_urls = $this->getURLs();

		if (!isset($this->_tmp['deleting_file'])) {
			$this->_tmp['deleting_file'] = FALSE;
		}
		
		if (!$this->usePublicPath() && !$this->_tmp['deleting_file']) {
			// this file is either in a safe edit, restricted, or doesnt want to be unrestricted
			// so we just call parent to update urls, if the asset is safe edit, but unrestricted otherwise, we still want to insert the __data url
			$result =  parent::updateLookups($auto_add_remaps);
			if(($this->status & SQ_SC_STATUS_SAFE_EDITING) && $this->attr('allow_unrestricted') && $this->effectiveUnrestricted()) {
			    $this->insertDataLookup();
			}
			return $result;
		}

		// Remove all the old URLs
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		unset($this->_tmp['lookups']);
		unset($this->_tmp['url']);
		unset($this->_tmp['href']);

		try {
			$bind_vars = Array('assetid' => $this->id);
			$result = MatrixDAL::executeQuery('core', 'deleteLookup', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to delete lookup information for asset: '.$this->id.' due to database error: '.$e->getMessage());
		}
		
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		// insert __data URL
		$result = $this->insertDataLookup();

		//Bug #5072: Trashed file assets still accessible via public data URL
		//We only reached here because we are supposed to have a public web path but
		//if we e.g. have moved to the trash or under a site with no URL, we should remove the public file.
		if (empty($result)){
			$this->_removePublicFile();
		} else {
			$this->_updatePublicFile();
		}

		// broadcast the lookup updated trigger event, passing old status as event data
		$parameter = Array();
		if (isset($this->_tmp['old_status'])) {
			$parameter['old_status'] = $this->_tmp['old_status'];
		}
		if (!empty($old_urls)) {
			$parameter['old_urls'] = $old_urls;
		}
		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($this, 'LookupsUpdated', Array('asset_name' => $this->name));
		$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_lookups_updated', $this, $parameter);

		return TRUE;

	}//end updateLookups()

	
	/**
	* Called to insert a __data url
	*
	* @return int
	* @access protected
	*/	
	protected function insertDataLookup()
	{
		// if the file is efffectively safe edit, we will only need to insert __data url pointing to the previous live version of it
		// pretend we are public user, and get file's previous public file info
		$restore = FALSE;
		if(($this->status & SQ_SC_STATUS_SAFE_EDITING) && $this->attr('allow_unrestricted') && $this->effectiveUnrestricted()) {
		    $public_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('public_user');
		    $GLOBALS['SQ_SYSTEM']->setCurrentUser($public_user);
		    $this->reload();
		    $restore = TRUE;
		}
		$existing = $this->getExistingFile();
		if($restore) {
		    $GLOBALS['SQ_SYSTEM']->restoreCurrentUser();
		    $this->reload();
		}
		
		if (!isset($existing['filename'])) {
			// there is no filename, so commit the removal of the current web paths and return
			return 0;
		}
		
		// noting to insert if asset's in trash
		if ($GLOBALS['SQ_SYSTEM']->am->assetInTrash($this->id, TRUE)){
		    return 0;
		}

		// Basically what we are going to do here is find all our parents that are sites
		// and take the system root urls that are closest to these site urls and tack on __data/... bit
		$save_urls = Array();
		$root_urls = explode("\n", trim(SQ_CONF_SYSTEM_ROOT_URLS));

		if (!SQ_CONF_STATIC_ROOT_URL == '') {
		    // use the static root URL data
		    $web_path = str_replace(sq_web_path('data').'/', '', $this->getWebDataPath()).'/'.$existing['filename'];
		    $save_urls[SQ_CONF_STATIC_ROOT_URL] = Array('http' => SQ_CONF_STATIC_ROOT_HTTP, 'https' => SQ_CONF_STATIC_ROOT_HTTPS);
		} else {

			$web_path = str_replace(sq_web_path('root_url').'/', '', $this->getWebDataPath()).'/'.$existing['filename'];
			$site_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'site', FALSE);
			foreach ($site_parents as $assetid => $type_code) {
				$site = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
				if (is_null($site)) continue;
				$urls = $site->getSiteURLS();

				foreach ($urls as $urlid => $data) {

					$matching_roots = Array();

					foreach ($root_urls as $tmp_url) {
						if (substr($data['url'].'/', 0, strlen($tmp_url) + 1) == $tmp_url.'/') {
							$matching_roots[] = $tmp_url;
						}
					}

					if (empty($matching_roots)) continue;

					foreach ($matching_roots as $root_url) {
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
					}

				}//end foreach

			}//end foreach

		}//end else - if SQ_CONF_STATIC_ROOT_URL is not empty
		
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		
		$result = 0;
		foreach ($save_urls as $url => $url_data) {
			try {
				$bind_vars	= Array (
								'url'			=> $url.'/'.$web_path,
								'assetid'		=> $this->id,
								'http'			=> ($this->force_secure === '1') ? '0' : $url_data['http'],
								'https'			=> ($this->force_secure === '-') ? '0' : $url_data['https'],
								'root_urlid'	=> 0,
							  );
				$result = MatrixDAL::executeQuery('core', 'insertLookup', $bind_vars);
			} catch (Exception $e) {
				throw new Exception('Unable to insert lookups for asset "'.$this->name.'" (#'.$this->id.') due to database error: '.$e->getMessage());
			}

			// NOTE: we are not going to update the design's lookups because no
			//       designs can be attached to a file anyway (well not at this time)
			//       and it's one thing less to worry about updating for the moment
			// NOTE NOTE: same applies to paint layouts...

		}//end foreach

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		
		return $result;
	}//end insertDataLookup()
	

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
		if (in_array($_POST['asset_action'], Array('create', 'create_custom'))) {
			$this->_tmp['uploading_file'] = 1;
		}
		switch ($_POST['asset_action']) {
			case 'create' :
				$ei = $this->getEI();
				if (!$ei->process($this, $o, TRUE)) return FALSE;
				$this->setAttrValue('name', 'temp');
				return $this->create($link);
			break;

			default :
				return parent::processBackend($o, $link);

		}//end switch

	}//end processBackend()


	/**
	* Set the value of an attribute
	*
	* @param boolean	$dont_run_updated	if TRUE this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	*/
	public function saveAttributes($dont_run_updated=FALSE)
	{
		if (!$this->id) return TRUE;

		// if there are no values for the title attribute use the filename
		if (trim($this->attr('title')) == '') {
			$this->setAttrValue('title', $this->attr('name'));
		}//end if

		if (isset($this->_tmp['vars_set']['name'])) {

			$uploading_file = !empty($this->_tmp['uploading_file']);

			if (!$uploading_file) {

				$old_filename = $this->_tmp['vars_set']['name']['old_value'];
				$new_filename = $this->attr('name');

				// try and set the new filename - return FALSE if it fails
				if (!parent::saveAttributes($dont_run_updated)) {
					return FALSE;
				}

				if (is_file($this->data_path.'/'.$old_filename)) {
					if (!$this->_renameFile($this->data_path, $old_filename, $new_filename)) {
						parent::setAttrValue('name', $old_filename);
						parent::saveAttributes($dont_run_updated);
						return FALSE;
					}
				}

				if ($this->effectiveUnrestricted()) {
					if (is_file($this->data_path_public.'/'.$old_filename)) {
						// trigger_event_lookups_updated needs old status event data
						$this->_tmp['old_status'] = $this->status;
						unlink($this->data_path_public.'/'.$old_filename);
						$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
						$fv->checkOut($this->data_path_suffix.'/'.$new_filename, $this->data_path_public);
					}
				}

				// update the lookups so that the __data path gets rewritten if applicable
				$this->updateLookups();

				return TRUE;

			}//end if not uploading a file

		}//end if we are changing the filename

		return parent::saveAttributes($dont_run_updated);

	}//end saveAttributes()


	/**
	* Rename a file in file versioning (copy, create new FV entry and then delete old FV entry)
	*
	* @param string	$data_path		data path of the asset
	* @param string	$old_filename	old filename
	* @param string	$new_filename	file to rename to
	*
	* @return boolean
	* @access private
	*/
	protected function _renameFile($data_path, $old_filename, $new_filename)
	{

		$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();

		// copy the file instead of a direct rename because removing the
		// file from the repository will also remove it from the file system
		if (!copy($data_path.'/'.$old_filename, $data_path.'/'.$new_filename)) {
			trigger_localised_error('CORE0026', E_USER_WARNING);
			return FALSE;
		}

		// we are renaming the file, so we need to remove the old name from
		// the repository and add the new one in
		$result = $fv->remove($data_path.'/'.$old_filename, '');
		if ($result != FUDGE_FV_OK && $result != FUDGE_FV_NOT_CHECKED_OUT) {
			trigger_localised_error('CORE0042', E_USER_WARNING);
			return FALSE;
		}

		// attempt to add a new version of the file to the repository
		if (!$fv->add($this->data_path_suffix, $data_path.'/'.$new_filename, '')) {
			trigger_localised_error('CORE0026', E_USER_WARNING);
			return FALSE;
		}

		// make sure we have the latest version of our file
		if (!$fv->checkOut($this->data_path_suffix.'/'.$new_filename, $data_path)) {
			trigger_localised_error('CORE0032', E_USER_WARNING);
			return FALSE;
		}

		return TRUE;

	}//end _renameFile()


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
	public function setAttrValue($name, $value)
	{
		$uploading_file = (!empty($this->_tmp['uploading_file'])) ? TRUE : FALSE;
		$created_file = isset($this->_tmp['file_create_data']['created_file']) ? $this->_tmp['file_create_data']['created_file'] : FALSE;

		if ($name == 'name' && !$uploading_file) {
			// check that we are not going to have web path conflicts
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$valid_names = make_valid_web_paths(Array($value));
			$value = array_shift($valid_names);

			if (is_file($this->data_path.'/'.$this->attr('name'))) {
				$data_path = $this->data_path;
			} else {
				$data_path = $this->data_path_public;
			}

			require_once SQ_FUDGE_PATH.'/general/file_system.inc';

			$old_filename = $this->attr('name');
			$old_ext = get_file_type($old_filename);
			$new_ext = get_file_type($value);
			if (trim($old_ext) != '' && $old_ext != $new_ext) {
				trigger_localised_error('CORE0115', E_USER_WARNING);
				$value = $old_filename;
				return FALSE;
			}
			// Should be able to set attribute of file before if its created by matrix
			if (!is_file($data_path.'/'.$old_filename) && !$created_file) {
				trigger_localised_error('CORE0023', E_USER_WARNING);
				$value = $old_filename;
				return FALSE;
			}
		}

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Checks a file on the file system to see if it can be used as a file
	*
	* @param array	$info	an array of information about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	public function validFile(Array $info)
	{
		$pre_uploaded = FALSE;
		if (isset($info['non_uploaded_file']) && $info['non_uploaded_file']) {
			$pre_uploaded = TRUE;
		}

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$ext = get_file_type($info['filename']);
		if (!empty($this->allowed_extensions) && !in_array($ext, $this->allowed_extensions)) {
			trigger_localised_error('CORE0106', E_USER_WARNING, make_readable_list($this->allowed_extensions, strtolower(translate('or'))), strtoupper($ext));
			// Get rid of temp copy of the uploaded file
			unlink($info['path']);

			return FALSE;
		}

		if ($this->_validFileSize($info, $pre_uploaded) && $this->_validFileExtension($info, $pre_uploaded)) {
			return TRUE;
		}

		unlink($info['path']);

		return FALSE;

	}//end validFile()


	/**
	* Checks a file size which is valid according to Global and User group settings
	*
	* @param array		&$info			an array of information about the uploaded file
	* @param boolean	$pre_uploaded	TRUE if the pre-uploaded file info is passed
	*
	* @return boolean
	* @access public
	*/
	protected function _validFileSize(Array &$info, $pre_uploaded=FALSE)
	{
		// check the file size to ensure we are not violating any restrictions
		$size = filesize($info['path']);

		$pre_uploaded = FALSE;
		if (isset($info['non_uploaded_file']) && $info['non_uploaded_file']) {
			$pre_uploaded = TRUE;
		}

		if ($pre_uploaded) {
			$file_max_size = $GLOBALS['SQ_SYSTEM']->getUserPrefs('file', 'SQ_FILE_PREUPLOADED_MAX_SIZE');
			$error_code = 'CORE0262';
		} else {
			$file_max_size = $GLOBALS['SQ_SYSTEM']->getUserPrefs('file', 'SQ_FILE_MAX_SIZE');
			$error_code = 'CORE0057';
		}

		if (empty($file_max_size)) return TRUE;

		$max_size = strtolower($file_max_size);
		if (!$max_size) return TRUE;

		if (substr($max_size, -1) == 'k') {
			$max_size = $max_size * 1024;
		} else if (substr($max_size, -1) == 'm') {
			$max_size = $max_size * 1024 * 1024;
		}
		if ($size > $max_size) {
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			trigger_localised_error($error_code, E_USER_WARNING, strtoupper(easy_filesize($max_size)), strtoupper(easy_filesize($size)));
			return FALSE;
		}

		return TRUE;

	}//end _validFileSize()


	/**
	* Checks a file type which is valid according to Global and User group settings
	*
	* @param array		&$info			an array of information about the uploaded file
	* @param boolean	$pre_uploaded	TRUE if the pre-uploaded file info is passed
	*
	* @return boolean
	* @access public
	*/
	protected function _validFileExtension(Array &$info, $pre_uploaded=FALSE)
	{
		// check the file size to ensure we are not violating any restrictions
		$filename = $info['filename'];

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$extension = get_file_type($filename);

		// If the script is running from the command line, do not restrict file type
		// e.g. installation scripts.
		if (php_sapi_name() == 'cli') return TRUE;

		// Root and System Administrators can over rule the restriction
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if ($is_root || $is_admin) {
			return TRUE;
		}

		// File with no extension
		$extension = trim($extension);
		if (empty($extension)) {
			$allowed_no_extension = $GLOBALS['SQ_SYSTEM']->getUserPrefs('file', 'SQ_FILE_ALLOW_NO_EXTENSION');
			if ($allowed_no_extension) return TRUE;
			trigger_localised_error('CORE0267', E_USER_WARNING);
			return FALSE;
		}

		if ($pre_uploaded) {
			$allowed_extensions = $GLOBALS['SQ_SYSTEM']->getUserPrefs('file', 'SQ_FILE_ALLOWED_PREUPLOADED_TYPES');
		} else {
			$allowed_extensions = $GLOBALS['SQ_SYSTEM']->getUserPrefs('file', 'SQ_FILE_ALLOWED_TYPES');
		}

		if (!empty($allowed_extensions)) {
			$allowed_extensions_list = explode(',', $allowed_extensions);
			if (!in_array($extension, $allowed_extensions_list)) {
				trigger_localised_error('CORE0266', E_USER_WARNING, $filename);
				return FALSE;
			}
		}

		if (isset($this->allowed_extensions) && !empty($this->allowed_extensions)) {
			if (!in_array($extension, $this->allowed_extensions)) {
				trigger_localised_error('CORE0266', E_USER_WARNING, $filename);
				return FALSE;
			}
		}

		return TRUE;

	}//end _validFileExtension()


	/**
	* Prints out the frontend for this file with no design
	*
	* This function only prints out the header information
	* according to the permission and file type
	*
	* @return void
	* @access public
	*/
	public function printFrontend()
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

		// we want to tell mysource::start() that it should not try to replace
		// keywords in this output, it could be a binary file after all
		$GLOBALS['SQ_SYSTEM']->setGlobalDefine('SQ_REPLACE_MYSOURCE_LEVEL_KEYWORDS', FALSE);

		$existing = $this->getExistingFile();
		if (!empty($existing)) {
			require_once SQ_FUDGE_PATH.'/standards_lists/mime_types.inc';
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';

			$ext = get_file_type($existing['filename']);
			if (($this->status & SQ_SC_STATUS_SAFE_EDITING) && ($GLOBALS['SQ_SYSTEM']->user instanceof Public_User)) {
				// strip file versioning suffix from the file extension, e.g. 'pdf,ffv5' becomes 'pdf'
				$ext = preg_replace('/,ffv\d+$/', '', $ext);
				$existing['filename'] = preg_replace('/,ffv\d+$/', '', $existing['filename']);
			}
			$type = (empty($standards_lists_mime_types[$ext])) ? 'text/plain' : $standards_lists_mime_types[$ext];

			// get protocol information
			$ssl_connection = FALSE;
			$url_info = parse_url(current_url());
			$protocol = (isset($url_info['scheme'])) ? $url_info['scheme'] : NULL;
			if (!is_null($protocol) && $protocol == 'https') {
				$ssl_connection = TRUE;
			}
			$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
			$using_ie6_8 = isset ($_SERVER['HTTP_USER_AGENT']) ? preg_match('/(MSIE\s[0-8]+\.)/', $_SERVER['HTTP_USER_AGENT']) : FALSE;

			// send cacheable headers if file asset meets the credentials
			if (basename($_SERVER['PHP_SELF']) != SQ_CONF_NOCACHE_SUFFIX && SQ_CONF_SEND_CACHEABLE_HEADER && ($GLOBALS['SQ_SYSTEM']->user instanceof Public_User) && empty($_POST) && $cm->cacheableHeadersEnabledForCurrentProtocol() && $this->status >= SQ_STATUS_LIVE && $this->readAccess()) {

				$browser_cache_expiry = $cm->getBrowserCacheExpiry($this->type(), $this->id);
				if (empty($browser_cache_expiry)) {
					$browser_cache_expiry = $cm->getExpiry($this->type(), $this->id);
				}

				$last_updated = $this->getEffectiveLastUpdatedTime(Array());
				header('Expires: '.gmdate('D, d M Y H:i:s', time() + $browser_cache_expiry).' GMT');
				header('Cache-Control: max-age='.$browser_cache_expiry.', '.$cm->cacheControlLevel());
				header('Pragma: cache');
				header('Last-Modified: '.gmdate('D, d M Y H:i:s',$existing['modified']).' GMT');
			} else if (!$using_ie6_8 || !$ssl_connection) {
				// Internet Explorer must forcibly reload some file types from the server for correct inline operation in popup windows. (ref. http://support.microsoft.com/default.aspx?scid=kb;EN-US;q297822)
				// If such files do not load as expected, "Send no-cache header option for HTTP requests" should be set to "No" on the System Configuration screen. The default operation is to send this header (i.e; set to "Yes").
				if (SQ_CONF_SEND_NO_CACHE_HEADER) {
					header('Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0');
				} else {
					header('Cache-Control: must-revalidate, post-check=0, pre-check=0');
				}

				header('Pragma: no-cache');
				header('Expires: '.gmdate('D, d M Y H:i:s', time()-3600).' GMT');
			} else {
				// internet explorer has a problem with SSL connection (https)
				// cant send no-cache header or we will get "cannot download file" error
				// http://support.microsoft.com/default.aspx?scid=kb;en-us;812935
				// http://support.microsoft.com/default.aspx?scid=kb;en-us;316431
				header('Cache-Control: private, max-age=0, must-revalidate');
				header('Pragma: private');
				header('Expires: '.gmdate('D, d M Y H:i:s', time()-3600).' GMT');
			}
			
			$disposition = 'inline';
			// stop rendering a html file
			if(strpos($type, 'html') !== FALSE) {
			    $type = 'application/download';
			    $disposition = 'attachment';
			}

			header('Content-Type: '.$type);
			header('Content-Disposition: '.$disposition.'; filename="'.$existing['filename'].'";');
			header('Content-Length: '.$existing['size']);
			ob_end_flush();

			readfile($existing['path']);

		}//end if

	}//end printFrontend()


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
	public function getURL($base_url=NULL, $ignore_rollback=FALSE)
	{
		if (SQ_ROLLBACK_VIEW) {
			if (is_null($base_url)) $base_url = current_url();
			$base_url = strip_url($base_url, FALSE);
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
	public function getHref($base_url=NULL, $ignore_rollback=FALSE)
	{
		if (SQ_ROLLBACK_VIEW) {
			return './?a='.$this->id;
		} else {
			return parent::getHref($base_url, $ignore_rollback);
		}

	}//end getHref()


	/**
	* Get an array of some useful data about the existing uploaded file
	*
	* @return array
	* @access public
	*/
	public function getExistingFile()
	{
		$existing = Array();
		$path = '';

		if (SQ_ROLLBACK_VIEW) {
			// get an older version of this file
			$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
			$rep_file = $this->data_path_suffix.'/'.$this->name;
			$then = iso8601_ts($_SESSION['sq_rollback_view']['rollback_time']);
			$info = $fv->_checkOutCheck($rep_file, NULL, $then);
			$path = $info['source_file'];
			$existing['filename'] = $info['filename'];
		} else if (($this->status & SQ_SC_STATUS_SAFE_EDITING) && ($GLOBALS['SQ_SYSTEM']->user instanceof Public_User)) {
			// safe edit, get the previous live version from the file versioning repository for public user (bug #1049)
			if (is_file($this->data_path.'/sq_system_version_no')) {
				$file_version = file_to_string($this->data_path.'/sq_system_version_no');
				$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
				$rep_file = $this->data_path_suffix.'/'.$this->name;
				$info = $fv->_checkOutCheck($rep_file, $file_version);
				$existing['filename'] = $info['filename'];
				$path = $info['source_file'];
			}
		} else {
			// get the latest file
			$path = $this->data_path.'/'.$this->attr('name');
		}

		if (is_file($path)) {
			$existing['path'] = $path;
			$existing['modified'] = filemtime($existing['path']);
			$existing['size'] = filesize($existing['path']);

			if (isset($existing['filename']) === false) {
				$existing['filename'] = basename($existing['path']);
			}//end if
		}

		return $existing;

	}//end getExistingFile()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	* @param int		$contextid	What context to return the name from
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	public function _getName($short_name=FALSE, $contextid=NULL)
	{
		// No context specified, using the current context
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}//end if

		// Obtain the attribute value for Title from the specified context
		if ($short_name) {
			$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('title', $this->type(), Array($this->id), $contextid);
			if (empty($values) === TRUE) {
				return $this->attr('title');
			} else {
				return $values[$this->id];
			}
		}

		return $this->attr('name');

	}//end _getName()


	/**
	* Return the keywords for this asset type.
	* This function is called in asset-listing situations like the asset listing, search page etc
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*   'name' => 'description' => 'the asset name',
	* );
	* </pre>
	*
	* @return array
	* @access public
	*/
	public function getAvailableKeywords()
	{
		$keywords = parent::getAvailableKeywords();

		$keywords['asset_file_size_in_bytes'] = translate('core_file_keyword_size_in_bytes');
		$keywords['asset_file_size_readable'] = translate('core_file_keyword_size_readable');
		$keywords['asset_file_contents'] = translate('core_file_keyword_file_contents');
		$keywords['asset_file_type'] = translate('core_file_keyword_file_type');


		return $keywords;

	}//end getAvailableKeywords()


	/**
	* Get keyword replacement
	*
	* @return string
	* @access public
	*/
	public function getAssetFileSizeInBytesKeywordReplacement()
	{
		$existing = $this->getExistingFile();
		$size = (isset($existing['size'])) ? $existing['size'] : 0;
		return $size;

	}//end getAssetFileSizeInBytesKeywordReplacement()


	/**
	* Get keyword replacement
	*
	* @return string
	* @access public
	*/
	public function getAssetFileSizeReadableKeywordReplacement()
	{
		$existing = $this->getExistingFile();
		$size = (isset($existing['size'])) ? $existing['size'] : 0;
		return easy_filesize($size);

	}//end getAssetFileSizeReadableKeywordReplacement()

	/**
	* Get %asset_file_contents% keyword replacement
	*
	* @return string
	* @access public
	*/
	public function getAssetFileContentsKeywordReplacement()
	{
		$existing = $this->getExistingFile();
		$file_contents = isset($existing['path']) ? file_get_contents($existing['path']) : '';
		return $file_contents;

	}//end getAssetFileContentsKeywordReplacement()

	/**
	* Delete this asset from the trash
	*
	* Note that the asset <i>MUST</i> be in the trash to delete it <i>FROM</i> the trash.
	*
	* @param boolean	$release_lock	should we release the lock after deleting
	*
	* @return boolean
	* @access public
	*/
	public function delete($release_lock=TRUE)
	{
		$this->_tmp['deleting_file'] = TRUE;
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!parent::delete($release_lock)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		$this->_tmp['deleting_file'] = FALSE;

		return TRUE;

	}//end delete()


	/**
	* Scan this asset using the external tools virus checker
	*
	* @param string		$path			If no path set, check the current file
	*
	* @return boolean
	* @access public
	*/
	public function scanFile($path='')
	{
		// Default to not scanned
		$status = TRUE;
		// Load the virus checker
		require_once SQ_FUDGE_PATH.'/antivirus/antivirus.inc';

		if (empty($path)) {
			$file_to_scan = $this->data_path.'/'.$this->attr('name');
		} else {
			$file_to_scan = $path;
		}//end if

		$report = '';
		$status = Antivirus::scan_file($file_to_scan, $report);

		// Return
		return $status;

	}//end scanFile()


	/**
	* Get keyword replacement
	*
	* @return string
	* @access public
	*/
	public function getFileTypeKeywordReplacement()
	{
		$ext = '';
		$file_info = $this->getExistingFile();
		if(isset($file_info['filename'])) {
			$ext = substr($file_info['filename'], strrpos($file_info['filename'], '.') + 1);
		}
		return $ext;

	}//end getFileTypeKeywordReplacement()


	/**
	* Returns the user customized summary for this asset
	*
	* @return string
	* @access public
	*/
	public function getAssetSummary()
	{
		// Get the summary content defination
		$summary = $GLOBALS['SQ_SYSTEM']->getUserPrefs('file', 'SQ_FILE_ASSET_SUMMARY');
		$keywords = retrieve_keywords_replacements($summary);

		$keyword_replacement = Array();
		foreach($keywords as $full_keyword) {
			$modifiers = NULL;
			$part_keyword = parse_keyword($full_keyword, $modifiers);
			$keyword_value = $this->getKeywordReplacement($part_keyword);
			if ($part_keyword != $full_keyword) {
				apply_keyword_modifiers($keyword_value, $modifiers, Array('assetid' => $this->id));
			}

			$keyword_replacement[$full_keyword] = $keyword_value;
		}

		replace_keywords($summary, $keyword_replacement);

		return $summary;

	}//end getAssetSummary()


	/**
	* Returns original URL for a __data type url for file type assets
	*
	* @param string		data_url	asset's __data  url out of which we get the original URL
	* @param string		site_url	url of site under which we have our asset

	*
	* @return array
	* @access public
	*/
	public function getOriginalURL($data_url, $site_url)
	{
		$original_url = Array();

		// if the url passed doenst have __data in it then there is nothing to do
		// just return it right away
		if(strpos($data_url, '__data') === FALSE) return $data_url;

		// *should never happen*
		if(empty($site_url)) return $data_url;

		$site_parts = explode('://', $site_url);
		$site = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL($site_parts[0], $site_parts[1]);

		$site_treeid = $GLOBALS['SQ_SYSTEM']->am->getAssetTreeids($site->id);

		$web_paths = $this->getWebPaths();
		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, '', TRUE, NULL, NULL, TRUE, 1, 1);

		$url_found = Array();
		foreach($parents as $parentid => $type_code) {
			if($site->id != $parentid){
				$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($parentid);
			} else {
				$parent_asset = $site;
			}
			$parent_urls = $parent_asset->getURLs();
			foreach ($parent_urls as $index => $parent_url) {
				if(strpos($parent_url['url'], $site_parts[1]) !== FALSE || $parent_url['url'] == $site_url) {
					$url_found = $parent_url;
					break(2);
				}
			}
		}

		if(!empty($url_found)) {
			$construct_url = ($url_found['https'] == '1') ? 'https://' : 'http://';
			foreach ($web_paths as $webpath) {
				$original_url[] = $construct_url.$url_found['url'].'/'.$webpath;
			}
		}

		return $original_url;

	}//end getOriginalURL()


}//end class

?>
