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
* $Id: file.inc,v 1.123 2006/01/27 00:00:58 lwright Exp $
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
* @version $Revision: 1.123 $
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
	* Create this asset
	*
	* @param array	&$link	information used to create the initial link
	* @param array	$info	info about the uploaded file
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	function create(&$link, $info=Array())
	{
		$this->_tmp['file_create_data'] =& $info;

		$name = trim($this->attr('name'));
		if ($name == '') {
			trigger_localised_error('CORE0083', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
			return FALSE;
		}

		// check that a file was uploaded at least and that it succeeded
		if (empty($info)) {
			$info = get_file_upload_info($this->getPrefix());
		}
		if ($info === FALSE || empty($info)) {
			$edit = $this->getEditFns();
			$info = $edit->getChosenFileInfo($this->getPrefix());
			if ($info === FALSE || empty($info)) {
				trigger_localised_error('CORE0056', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
				return FALSE;
			}
		}

		// make the info array look like a result from getExistingFile()
		if (!isset($info['path'])) {
			$info['path'] = array_get_index($info, 'tmp_name', '');
		}
		if (!isset($info['filename'])) {
			$info['filename'] = array_get_index($info, 'name', '');
		}
		if (!$this->validFile($info)) return FALSE;

		// check that we are not going to have web path conflicts
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_names = make_valid_web_paths(Array($info['name']));
		$name = array_shift($valid_names);

		// make sure the new web path is not already is use
		$bad_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], Array($name));
		if (!empty($bad_paths)) {
			trigger_localised_error('CORE0086', E_USER_WARNING, $name);
			return FALSE;
		}

		$this->setAttrValue('name', $name);

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
	function _createAdditional(&$link)
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
		$o =& $GLOBALS['SQ_SYSTEM']->backend->out;
		$info = $this->_tmp['file_create_data'];

		$ret_val = $edit->processFileChooser($this, $o, $prefix, TRUE) || $edit->processFileUpload($this, $o, $prefix, $info, FALSE);

		$this->saveAttributes();

		if ($lock_held == 1) {
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'attributes');
		}

		return $ret_val;

	}//end _createAdditional()


	/**
	* Processes the file upload from the source file so that file versioning cloned properly
	*
	* @param object	&$clone		the clone asset shell
	* @param array	$components	the wanted components to clone eg.
	*							Array(
	*							'attributes',
	*							'metadata_shemas',
	*							'metadata',
	*							'workflow',
	*							'permissions',
	*							'data'
	*							);
	*							or alternately Array('all');
	*
	* @return boolean
	* @access public
	* @see asset::cloneComponents()
	*/
	function cloneComponents(&$clone, $components)
	{
		// create some information so we know where to clone the file from
		$temp_info = Array('name' => $this->name, 'tmp_name' => $this->data_path.'/'.$this->name, 'non_uploaded_file' => TRUE);

		$edit_fns = $clone->getEditFns();
		if (!$edit_fns->processFileUpload($clone, $GLOBALS['SQ_SYSTEM']->backend->out, $clone->getPrefix(), $temp_info)) {
			trigger_localised_error('CORE0010', E_USER_WARNING, $this->name, $this->id);
			return FALSE;
		}

		return parent::cloneComponents($clone, $components);

	}//end cloneComponents()


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return array
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

		// update our lookup tables to not use __data
		if (!parent::updateLookups()) {
			trigger_localised_error('CORE0053', E_USER_WARNING, $this->name);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// store the current version of the file
		$fv =& $GLOBALS['SQ_SYSTEM']->getFileVersioning();
		$file_info = $fv->_checkOutCheck($this->data_path_suffix.'/'.$this->name);
		$latest_version = $file_info['version'];
		if (!string_to_file($latest_version, $this->data_path.'/.sq_system/sq_system_version_no')) {
			trigger_localised_error('CORE0051', E_USER_WARNING, $this->name);
			return FALSE;
		}

		return TRUE;

	}//end saveSystemVersion()


	/**
	* Replaces the editing version with the system (live) version
	*
	* @return boolean
	* @access public
	*/
	function revertToSystemVersion()
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
		if (!clear_directory($this->data_path)) {
			trigger_localised_error('CORE0045', E_USER_WARNING, $this->name);
			return FALSE;
		}

		$fv =& $GLOBALS['SQ_SYSTEM']->getFileVersioning();

		// we want to remove the safe edit file name from the repository because
		// there's a chance someone will try and use that name later
		if ($this->attr('name') != $se_name) {
			$se_rep_path = $this->data_path_suffix.'/'.$se_name;
		}

		// check out the old version of the file into the private data directory
		$rep_path = $this->data_path_suffix.'/'.$this->name;
		if (!$fv->checkOut($rep_path, $this->data_path, $file_version)) {
			trigger_localised_error('CORE0044', E_USER_WARNING, $this->name);
			return FALSE;
		}

		// create a new versioning entry for the old version
		$file_info = $fv->_checkOutCheck($rep_path);
		$fv->_updateFile($file_info['fileid'], $this->data_path_suffix, $this->data_path.'/'.$this->name);

		// check out the new version
		if (!$fv->checkOut($rep_path, $this->data_path)) {
			trigger_localised_error('CORE0044', E_USER_WARNING, $this->name);
			return FALSE;
		}

		// this will ensure that the lookups are updated and that the
		// public file is properly checked out
		$this->_checkFileState();

		return TRUE;

	}//end revertToSystemVersion()


	/**
	* Morph's this asset to either one of it's parent type or one of it's children types
	* Only succeeds if $type_code asset's is allowed to link to all children that this asset has
	*
	* @param string	$new_type_code	the new type_code to attempt to cast this asset to
	*
	* @return boolean
	* @access public
	*/
	function morph($new_type_code)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$file_types = array_merge($am->getTypeAncestors($this->type(), FALSE), $am->getTypeDescendants($this->type()));

		if (in_array($new_type_code, $file_types)) {
			// we are morphing into another file type, so check that the actual
			// file currently uploaded is not going to conflict with the new type
			$am->includeAsset($new_type_code);
			$tmp =& new $new_type_code();
			$existing = $this->getExistingFile();
			if (!empty($existing)) {
				if (!$tmp->validFile($existing)) {
					trigger_localised_error('CORE0008', E_USER_WARNING, $this->name, $new_type_code, $existing['filename'], $new_type_code);
					return FALSE;
				}

				$fv =& $GLOBALS['SQ_SYSTEM']->getFileVersioning();
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
	*
	* @return boolean
	* @access public
	*/
	function processStatusChange($new_status)
	{
		// let ourselves know not to do any extra updating
		$this->_tmp['IGNORE_UPDATE'] = TRUE;
		$old_status = $this->status;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!parent::processStatusChange($new_status)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// NOTE : no need to update the files states or the lookups here because they are done in _updated() if a file exists

		// update manually
		unset($this->_tmp['IGNORE_UPDATE']);
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
	function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_1	=> Array(),
				SQ_LINK_TYPE_2	=> Array(),
				SQ_LINK_TYPE_3	=> Array(),
				SQ_LINK_NOTICE	=> Array('image' => Array('card' => 1, 'exclusive' => FALSE)),
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
	function _updated($update_parents=TRUE)
	{
		if (!parent::_updated($update_parents)) return FALSE;

		if ($this->status & SQ_SC_STATUS_SAFE_EDITING || !empty($this->_tmp['IGNORE_UPDATE'])) {
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
	function permissionsUpdated()
	{
		return $this->_checkFileState();

	}//end permissionsUpdated()


	/**
	* Look's after the placing and removing of out files in the public directory
	*
	* @return boolean
	* @access private
	*/
	function _checkFileState()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		$existing = $this->getExistingFile();
		if (empty($existing)) return TRUE;

		$unrestricted = $this->data_path_public.'/'.$existing['filename'];

		$changed = FALSE;

		// if we should be storing our file in the public
		// data directory for all the world to see
		if ($this->effectiveUnrestricted() && $this->attr('allow_unrestricted')) {
			// make sure our data directory exists
			if (!create_directory($this->data_path_public)) {
				trigger_localised_error('CORE0049', E_USER_WARNING, $this->name);
				return FALSE;
			}

			$fv =& $GLOBALS['SQ_SYSTEM']->getFileVersioning();
			// if there is no current file or if the current file is out of date, check out a fresh copy
			if ($fv->upToDate($unrestricted) & (FUDGE_FV_NOT_CHECKED_OUT | FUDGE_FV_OLD_VERSION)) {
				if (file_exists($unrestricted)) {
					unlink($this->data_path_public.'/'.$existing['filename']);
				}

				if ($fv->checkOut($this->data_path_suffix.'/'.$existing['filename'], $this->data_path_public)) {
					$changed = TRUE;
				} else {
					trigger_localised_error('CORE0032', E_USER_WARNING);
					return FALSE;
				}
			}
		} else {
			// need to make sure our file is in the NOT in the public data directory
			if (is_file($unrestricted)) {
				$fv =& $GLOBALS['SQ_SYSTEM']->getFileVersioning();
				if ($fv->clearOut($unrestricted) & FUDGE_FV_OK) {
					$changed = TRUE;
				} else {
					trigger_localised_error('CORE0104', E_USER_WARNING);
					return FALSE;
				}
			}
		}//end if effectiveUnrestricted

		if ($changed) {
			return $this->updateLookups();
		} else {
			return TRUE;
		}

	}//end _checkFileState()


	/**
	* Returns whether to use the public path or not for this file
	*
	* @return boolean
	* @access public
	* @see File::updateLookups()
	*/
	function usePublicPath()
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
	function updateLookups()
	{
		if (!isset($this->_tmp['deleting_file'])) {
			$this->_tmp['deleting_file'] = FALSE;
		}
		if (!$this->usePublicPath() && !$this->_tmp['deleting_file']) {
			// this file is either in a safe edit, restricted, or doesnt want to be unrestricted
			// so we dont worry about __data paths
			return parent::updateLookups();
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		unset($this->_tmp['lookups']);
		unset($this->_tmp['url']);
		unset($this->_tmp['href']);

		// Remove all the old URLs

		// design lookups
		$sql = 'DELETE FROM
					sq_ast_lookup_value
				WHERE
					url IN
						(
							SELECT
								url
							FROM
								sq_ast_lookup l
							WHERE
								assetid = '.$db->quoteSmart($this->id).'
						)
					AND	(name LIKE '.$db->quoteSmart('design::%').'
						OR name LIKE '.$db->quoteSmart('paint_layout::%').')';

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$sql = 'DELETE FROM
					sq_ast_lookup
				WHERE
					assetid = '.$db->quote($this->id);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$existing = $this->getExistingFile();
		if (!isset($existing['filename'])) {
			// there is no filename, so commit the removal of the current web paths and return
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return TRUE;
		}

		$web_path = str_replace(sq_web_path('root_url').'/', '', $this->getWebDataPath()).'/'.$existing['filename'];

		// Basically what we are going to do here is find all our parents that are sites
		// and take the system root urls that are closest to these site urls and tack on __data/... bit
		$save_urls = Array();
		$root_urls = explode("\n", trim(SQ_CONF_SYSTEM_ROOT_URLS));

		if (!SQ_CONF_STATIC_ROOT_URL == '') {
			// use the static root URL data
			$web_path = str_replace(sq_web_path('data').'/', '', $web_path);
			$save_urls[SQ_CONF_STATIC_ROOT_URL] = Array('http' => SQ_CONF_STATIC_ROOT_HTTP, 'https' => SQ_CONF_STATIC_ROOT_HTTPS);

		} else {

			$site_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'site', FALSE);
			foreach ($site_parents as $assetid => $type_code) {
				$site =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
				if (is_null($site)) continue;
				$urls = $site->getSiteURLS();

				foreach ($urls as $urlid => $data) {

					$matching_roots = Array();

					foreach ($root_urls as $tmp_url) {
						if (substr($data['url'], 0, strlen($tmp_url)) == $tmp_url) {
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

		foreach ($save_urls as $url => $url_data) {

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
							'.$db->quoteSmart($url.'/'.$web_path).',
							'.$db->quoteSmart($this->id).',
							'.$db->quoteSmart(($this->force_secure) ? '0' : $url_data['http']).',
							'.$db->quoteSmart($url_data['https']).',
							'.$db->quoteSmart(0).'
						)';

			$result = $db->query($sql);
			assert_valid_db_result($result);

			// NOTE: we are not going to update the design's lookups because no
			//       designs can be attached to a file anyway (well not at this time)
			//       and it's one thing less to worry about updating for the moment
			// NOTE NOTE: same applies to paint layouts...

		}//end foreach

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end updateLookups()


	/**
	* Processes a backend submission from this asset, returns TRUE if all OK
	*
	* @param object	&$o		Backend Outputter
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	function processBackend(&$o, &$link)
	{
		switch ($_POST['asset_action']) {
			case 'create' :
				$this->_tmp['uploading_file'] = 1;
				$ei =& $this->getEI();
				if (!$ei->process($this, $o, TRUE)) return FALSE;

				$prefix = $this->getPrefix();
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
	function saveAttributes($dont_run_updated=FALSE)
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
						unlink($this->data_path_public.'/'.$old_filename);
						$fv =& $GLOBALS['SQ_SYSTEM']->getFileVersioning();
						$fv->checkOut($this->data_path_suffix.'/'.$new_filename, $this->data_path_public);
					}
				}

				// update the lookups so that the __data path gets rewritten if applicable
				$this->updateLookups();

				return TRUE;

			}//end if not uploading a file

		}//end if we are changing the filename

		if (isset($this->_tmp['vars_set']['allow_unrestricted'])) {
			// the unrestricted setting is being changed, so we may need to update lookups
			if ($this->effectiveUnrestricted() && !($this->status & SQ_SC_STATUS_SAFE_EDITING)) {
				$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$vars = Array('assetids' => Array($this->id));
				$hh->queueHipo('hipo_job_update_lookups', $vars);
			}
		}

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
	function _renameFile($data_path, $old_filename, $new_filename)
	{

		$fv =& $GLOBALS['SQ_SYSTEM']->getFileVersioning();

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
	function setAttrValue($name, $value)
	{
		$uploading_file = (!empty($this->_tmp['uploading_file'])) ? TRUE : FALSE;

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
			if (!is_file($data_path.'/'.$old_filename)) {
				trigger_localised_error('CORE0023', E_USER_WARNING);
				$value = $old_filename;
				return FALSE;
			}
		} else if ($name == 'title') {
			$value = htmlspecialchars($value);
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
	function validFile($info)
	{
		// check the file size to ensure we are not violating any restrictions
		$size = filesize($info['path']);

		$sq_file_max_size = $GLOBALS['SQ_SYSTEM']->getUserPrefs('file', 'SQ_FILE_MAX_SIZE');
		if (empty($sq_file_max_size)) return TRUE;

		$max_size = strtolower($sq_file_max_size);
		if (!$max_size) return TRUE;

		if (substr($max_size, -1) == 'k') {
			$max_size = $max_size * 1024;
		} else if (substr($max_size, -1) == 'm') {
			$max_size = $max_size * 1024 * 1024;
		}
		if ($size > $max_size) {
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			trigger_localised_error('CORE0057', E_USER_WARNING, strtoupper(easy_filesize($max_size)), strtoupper(easy_filesize($size)));
			return FALSE;
		}
		return TRUE;

	}//end validFile()


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
			$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $this->name));
			return;
		}

		$existing = $this->getExistingFile();
		if (!empty($existing)) {
			require_once SQ_FUDGE_PATH.'/standards_lists/mime_types.inc';
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';

			$ext = get_file_type($existing['filename']);
			if (($this->status & SQ_SC_STATUS_SAFE_EDITING) && (is_a($GLOBALS['SQ_SYSTEM']->user, 'public_user'))) {
				// strip file versioning suffix from the file extension, e.g. 'pdf,ffv5' becomes 'pdf'
				$ext = preg_replace('/,ffv\d+$/', '', $ext);
			}
			$type = (empty($standards_lists_mime_types[$ext])) ? 'text/plain' : $standards_lists_mime_types[$ext];
			header('Cache-Control:');
			header('Pragma: cache');
			header('Expires: '.gmdate('D, d M Y H:i:s', (time() + 86400)).' GMT');
			header('Content-Type: '.$type);
			header('Content-Disposition: inline; filename='.$existing['filename'].';');
			header('Last-Modified: '.gmdate('D, d M Y H:i:s',$existing['modified']).' GMT');
			header('Content-Length: '.$existing['size']);
			readfile($existing['path']);
		}

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
	function getURL($base_url=NULL, $ignore_rollback=FALSE)
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
	function getHref($base_url=NULL, $ignore_rollback=FALSE)
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
	function getExistingFile()
	{
		$existing = Array();

		if (SQ_ROLLBACK_VIEW) {
			// get an older version of this file
			$fv =& $GLOBALS['SQ_SYSTEM']->getFileVersioning();
			$rep_file = $this->data_path_suffix.'/'.$this->name;
			$then = iso8601_ts($_SESSION['sq_rollback_view']['rollback_time']);
			$info = $fv->_checkOutCheck($rep_file, NULL, $then);
			$path = $info['source_file'];
		} else if (($this->status & SQ_SC_STATUS_SAFE_EDITING) && (is_a($GLOBALS['SQ_SYSTEM']->user, 'public_user'))) {
			// safe edit, get the previous live version from the file versioning repository for public user (bug #1049)
			if (is_file($this->data_path.'/sq_system_version_no')) {
				$file_version = file_to_string($this->data_path.'/sq_system_version_no');
				$fv =& $GLOBALS['SQ_SYSTEM']->getFileVersioning();
				$rep_file = $this->data_path_suffix.'/'.$this->name;
				$info = $fv->_checkOutCheck($rep_file, $file_version);
				$path = $info['source_file'];
			}
		} else {
			// get the latest file
			$path = $this->data_path.'/'.$this->attr('name');
		}

		if (is_file($path)) {
			$existing['path'] = $path;
			$existing['filename'] = basename($existing['path']);
			$existing['modified'] = filemtime($existing['path']);
			$existing['size'] = filesize($existing['path']);
		}

		return $existing;

	}//end getExistingFile()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return (($short_name) ? $this->attr('title') : $this->attr('name'));

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
	function getAvailableKeywords()
	{
		$keywords = parent::getAvailableKeywords();

		$keywords['asset_file_size_in_bytes'] = translate('core_file_keyword_size_in_bytes');
		$keywords['asset_file_size_readable'] = translate('core_file_keyword_size_readable');

		return $keywords;

	}//end getAvailableKeywords()


	/**
	* Get keyword replacement
	*
	* @return string
	* @access public
	*/
	function getAssetFileSizeInBytesKeywordReplacement()
	{
		$existing = $this->getExistingFile();
		return $existing['size'];

	}//end getAssetFileSizeInBytesKeywordReplacement()


	/**
	* Get keyword replacement
	*
	* @return string
	* @access public
	*/
	function getAssetFileSizeReadableKeywordReplacement()
	{
		$existing = $this->getExistingFile();
		return easy_filesize($existing['size']);

	}//end getAssetFileSizeReadableKeywordReplacement()


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
	function delete($release_lock=TRUE)
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


}//end class

?>
