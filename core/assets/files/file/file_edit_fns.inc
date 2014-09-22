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
* $Id: file_edit_fns.inc,v 1.100 2013/07/25 23:23:48 lwright Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';

/**
* File_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.100 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class File_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function File_Edit_Fns()
	{
		parent::__construct();
		$this->static_screens['details']['lock_type'] = 'attr_links';
		unset($this->static_screens['lookupValues']);

	}//end constructor


	/**
	* Prints the interface for editing filename
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintFilename(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			text_box($prefix.'_filename', $asset->attr('name'), 25);
		} else {
			echo $asset->attr('name');
		}
		return $wa;

	}//end paintFilename()


	/**
	* Processes the interface for editing filename
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processFilename(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$submitted = isset($_REQUEST[$prefix.'_filename']) && !empty($_REQUEST[$prefix.'_filename']);
		$process = $wa && $submitted;

		if ($process) {
			$info = get_file_upload_info($prefix);

			// if the user is uploading the file, do not process the name
			if ($info !== FALSE && !empty($info)) {
				return FALSE;
			}

			if ($asset->name != $_REQUEST[$prefix.'_filename']) {
				$info = Array();
				$info['name'] = $_REQUEST[$prefix.'_filename'];			// Asset new filename
				$info['tmp_name'] = $asset->data_path.'/'.$asset->name;	// Source file new
				$info['non_uploaded_file'] = TRUE;						// Specify that file is already uploaded.
				$info['setting_filename'] = TRUE;						// Tell the function that we are in setting the file name process.
				if (!$this->processFileUpload($asset, $o, $prefix, $info)) {
					return TRUE;
				}
			}
		}

		return $process;

	}//end processFilename()


	/**
	* Prints the interface for uploading a file
	*
	* @param File				$asset	the file asset
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintFileUpload(File $asset, Backend_Outputter $o, $prefix)
	{
		// We redirect the header frame to the new location only if
		// we are in limbo, the file name has been changed and allow_unrestricted is false.
		// If allow_unrestricted is true, the asset is identified by limbo_assetid GET var
		if (SQ_IN_LIMBO) {
			if (!empty($_REQUEST['redirect_header'])) {
				$url = strip_url($asset->getUrl()).'/'.SQ_CONF_LIMBO_SUFFIX
						.'?SQ_BACKEND_PAGE=header'
						.'&current_assetid='.$asset->id
						.'&sq_popups_blocked=0';
				?>
				<script type="text/javascript">
					if (parent.frames["sq_header"]) {
						lock_type = parent.frames["sq_main"].get_form_element_value('sq_lock_type');
						parent.frames["sq_header"].location = "<?php echo $url ?>" + "&sq_lock_type=" + lock_type;
					}
				</script>
				<?php
			}
		}
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$options = $this->getPreuploadOptions($asset);
		echo '<div id="'.$prefix.'_file_pre_upload" style="display:none;">';
		combo_box($prefix.'_server_file', Array(''=>'')+$options, FALSE, Array());
		echo '&nbsp;&nbsp;';
		normal_button($prefix.'_simple_upload_button', translate('Upload a new file'),		 'document.getElementById(\''.$prefix.'_file_pre_upload\').style.display=\'none\';

		document.getElementById(\''.$prefix.'_choose_server_file_button\').parentNode.style.display=\'\'; document.getElementById(\''.$prefix.'_server_file\').value = \'\'');
		echo '</div>';

		if ($asset->writeAccess('attributes')) {
			echo '<div id="'.$prefix.'_file_upload">';
			file_upload($prefix);
			if ($this->havePreUploadedFiles($asset, $prefix)) {
				echo '&nbsp;&nbsp;';
				normal_button($prefix.'_choose_server_file_button', translate('Choose a pre-uploaded file'),

				'document.getElementById(\''.$prefix.'_file_upload\').style.display=\'none\';
				document.getElementById(\''.$prefix.'_simple_upload_button\').parentNode.style.display=\'\'');
			}
		}

		$this->printExistingFileInfo($asset);
		if ($asset->writeAccess('attributes')) {
			$max_size = strtolower($GLOBALS['SQ_SYSTEM']->getUserPrefs('file', 'SQ_FILE_MAX_SIZE'));
			if ($max_size) {
				if (substr($max_size, -1) == 'k') {
					$max_size = $max_size * 1024;
				} else if (substr($max_size, -1) == 'm') {
					$max_size = $max_size * 1024 * 1024;
				}
			} else {
				// work out the max file size that PHP is allowing
				$ini_size = strtolower(ini_get('upload_max_filesize'));
				if (substr($ini_size, -1) == 'k') {
					$ini_size = $ini_size * 1024;
				} else if (substr($ini_size, -1) == 'm') {
					$ini_size = $ini_size * 1024 * 1024;
				}

				// work out the max post size that PHP is allowing
				$post_size = strtolower(ini_get('post_max_size'));
				if (substr($post_size, -1) == 'k') {
					$post_size = $post_size * 1024;
				} else if (substr($post_size, -1) == 'm') {
					$post_size = $post_size * 1024 * 1024;
				}

				$mem_limit_size = strtolower(ini_get('memory_limit'));
				if (substr($mem_limit_size, -1) == 'k') {
					$mem_limit_size = $mem_limit_size * 1024;
				} else if (substr($mem_limit_size, -1) == 'm') {
					$mem_limit_size = $mem_limit_size * 1024 * 1024;
				}
				$max_size = min($ini_size, $post_size, $mem_limit_size);
			}
			?><br/><span style="font-size: 10px;"><?php echo sprintf(translate('You may only upload files no larger than %s'), strtoupper(easy_filesize($max_size))); ?></span><?php

			if (!empty($asset->allowed_extensions)) {
				$o->note(sprintf(translate('<b>NOTE:</b> You may only upload files of type %s.'), make_readable_list($asset->allowed_extensions, strtolower(translate('And')))));


			}
			echo '</div>';
		}//end if

		return TRUE;

	}//end paintFileUpload()


	/**
	* Processes the interface for uploading a file
	*
	* @param File				$asset				the file asset
	* @param Backend_Outputter	$o					the Backend Outputter object
	* @param string				$prefix				prefix for form elements
	* @param array				$info				info about the uploaded file
	* @param boolean			$redirect_in_limbo	redirect when the file name changes in limbo
	*
	* @return boolean
	* @access public
	*/
	public function processFileUploadPreUpload(File $asset, $o, $prefix, Array $info=Array(), $redirect_in_limbo=TRUE) {
		$processed = FALSE;

		if (isset($_POST[$prefix.'_server_file']) && empty($_POST[$prefix.'_server_file'])){
			$processed = $this->processFileUpload($asset, $o, $prefix, $info);
		} else {
			$processed = $this->processFileChooser($asset, $o, $prefix);
		}

		return $processed;

	}//end processFileUploadPreUpload()


	/**
	* Processes the interface for uploading a file
	*
	* NOTE: this function is called from several places other than by asset_edit_interface,
	* for example file::_createAdditional and trigger_action_replace_file::execute
	*
	* If you want to process an upload of a file that was not uploaded through PHP
	* (possible because of FTP or it was in a tar archive), ensure you pass the information
	* about the file in the $info array.
	* The fields required are:
	*     name              => the filename without path (eg file.txt)
	*     tmp_name          => the location of the file on the file system
	*     non_uploaded_file => must equal TRUE
	*
	* @param File				$asset				the file asset
	* @param Backend_Outputter	$o					the Backend Outputter object
	* @param string				$prefix				prefix for form elements
	* @param array				$info				info about the uploaded file
	* @param boolean			$redirect_in_limbo	redirect when the file name changes in limbo
	* @param boolean			$asset_cloned		to check if we creating a new asset or just cloning
	*
	* @return boolean
	* @access public
	*/
	public function processFileUpload(File $asset, $o, $prefix, Array $info=Array(), $redirect_in_limbo=TRUE, $asset_cloned=FALSE)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		if (empty($info) && isset($_POST[$prefix.'_server_file']) && !empty($_POST[$prefix.'_server_file'])) {
			return FALSE;
		}

		// assetid will not be set if we are creating
		if (!$asset->id) return TRUE;

		if (empty($info)) {
			$info = get_file_upload_info($prefix);
			// return on failed or no upload
			if ($info === FALSE || empty($info)) {
				return FALSE;
			}//end if
		}//end if

		// make the info array look like a result from getExistingFile()
		if (!isset($info['path'])) {
			$info['path'] = array_get_index($info, 'tmp_name', '');
		}//end if

		if (!isset($info['filename'])) {
			$info['filename'] = array_get_index($info, 'name', '');
		}//end if

		$setting_filename = FALSE;
		if (isset($info['setting_filename'])) {
			$setting_filename = array_get_index($info, 'setting_filename', FALSE);
		}//end if

		if (!$asset->validFile($info)) return FALSE;

		// Check for a virus
		if (isset($info['path']) && !empty($info['path'])) {
			$status = $asset->scanFile($info['path']);
			// If infected, remove and warn the user
			if (!$status) {
				trigger_localised_error('CORE0300', E_USER_WARNING, $info['filename']);
				return FALSE;
			}//end if
		}//end if

		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$name = current(make_valid_web_paths(Array($info['name'])));

		// make sure the new web path is not already in use
		if (!defined('SQ_IN_IMPORT') && $asset->name != $name) {
			$parents = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', NULL, 'minor');
			for ($i = 0; $i < count($parents); $i++) {
				$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($parents[$i]['majorid'], $parents[$i]['major_type_code']);
				if (is_null($parent)) continue;
				$bad_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($parent, Array($name), $asset->id);
				if (!empty($bad_paths)) {
					trigger_localised_error('CORE0055', E_USER_WARNING, $name);
					return FALSE;
				}//end if
			}//end if
		}//end if

		// create the destination directory, if necessary (NOTE that checking is internal for this)
		create_directory($asset->data_path);


		//// FILE VERSIONING ////

		$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
		$existing = $asset->getExistingFile();

		if ($setting_filename === TRUE) {
			$this->getNewFile($asset, $name, $info);
			$this->removeOldFile($asset, $existing, $fv);
		} else {
			if ($asset->name != $name && !empty($existing)) {
				$this->removeOldFile($asset, $existing, $fv);
			}
			$this->getNewFile($asset, $name, $info);
		}

		// if we are overwriting our current file with one that has the same name,
		// we need to add a new version of the file to the repository
		if ($asset->name == $name && !empty($existing)) {
			$file_status = $fv->upToDate($asset->data_path.'/'.$name);
			if (FUDGE_FV_MODIFIED & $file_status) {
				if (!$fv->commit($asset->data_path.'/'.$name, '')) {
					trigger_localised_error('CORE0034', E_USER_WARNING);
					return FALSE;
				}//end if
			}//end if
		} else {
			// attempt to add the file to the repository
			if (!$fv->add($asset->data_path_suffix, $asset->data_path.'/'.$name, '')) {
				trigger_localised_error('CORE0026', E_USER_WARNING);
				return FALSE;
			}//end if
		}//end else

		// make sure we have the latest version of our file
		if (!$fv->checkOut($asset->data_path_suffix.'/'.$name, $asset->data_path)) {
			trigger_localised_error('CORE0032', E_USER_WARNING);
			return FALSE;
		}//end if

		// set the name of the file to the newly uploaded one
		$asset->_tmp['uploading_file'] = TRUE;
		$name_changed = $asset->setAttrValue('name', $name);
		if (!$asset->saveAttributes()) {
			trigger_localised_error('CORE0052', E_USER_WARNING);
			return FALSE;
		}//end if
		$asset->_tmp['uploading_file'] = FALSE;

		// bug fix for # 3625. If we cloning an asset then we need not call saveWebPaths now
		// hipo_job_clone_asset will call saveWebPaths after it is sure the links have been established
		$rm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('remap_manager');
		$auto_add_remaps = $rm->attr('remap_upon_webpath_change');
		if (!defined('SQ_IN_IMPORT') && !($asset_cloned) && !($asset->saveWebPaths(Array($name),$auto_add_remaps))) {
			trigger_localised_error('CORE0054', E_USER_WARNING);
			return FALSE;
		}//end if

		if (SQ_IN_LIMBO && $name_changed && $redirect_in_limbo) {
			// if we are in limbo and the name of the file has changed, the URL will also change
			// so we need to redirect to the new limbo URL
			if (!$asset->attr('allow_unrestricted')) {
				$o->setRedirect(strip_url($asset->getUrl()).'/'.SQ_CONF_LIMBO_SUFFIX.'?redirect_header=1');
			}//end if
		}//end if

		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($asset, 'ContentsUpdated');

		return $asset->_updated();

	}//end processFileUpload()


	/**
	* Process the interface where you can choose a pre-uploaded file
	*
	* Returns TRUE if a file was processed
	*
	* @param File				$asset	the file asset
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for form elements
	* @param boolean			$create	TRUE if this is being called from createAdditional and should run
	*									even though the asset is not yet created
	*
	* @return boolean
	* @access public
	*/
	public function processFileChooser(File $asset, $o, $prefix, $create=FALSE)
	{
		// Unless explicitly told otherwise, don't do anything if the asset is not yet completely created
		if (!$create && (empty($asset->id) || array_get_index($asset->_tmp, '__creating__'))) {
			return TRUE;
		}

		if ($asset->writeAccess('attributes') && isset($_POST[$prefix.'_server_file']) && (!empty($_POST[$prefix.'_server_file']))) {
			$info = $this->getChosenFileInfo($prefix);
			if ($this->processFileUpload($asset, $o, $prefix, $info)) {
				unlink($info['tmp_name']);
				return TRUE;
			}
		}
		return FALSE;

	}//end processFileChooser()


	/**
	* Find out if there are any pre-uploaded files we can use, returns TRUE if there is
	*
	* @param Asset	$asset	The asset that might use the files
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	protected function havePreUploadedFiles(File $asset, $prefix)
	{
		$preuploads = $this->getPreuploadOptions($asset);
		return !empty($preuploads);

	}//end havePreUploadedFiles()


//--        HELPERS        --//


	/**
	* Get a list of pre-uploaded files available for use
	*
	* Subclasses can override this to restrict file types
	* Return value format: Array(value => value to be used with combo_box)
	*
	* @param Asset	$asset	The asset that might use the files
	*
	* @return array
	* @access protected
	*/
	protected function getPreuploadOptions(File $asset)
	{
		$options = Array();
		foreach (list_files(SQ_TEMP_PATH) as $filename) {
			if (substr($filename, 0, 1) == '.') continue;
			$options[$filename] = $filename;
		}
		ksort($options);

		return $options;

	}//end getPreuploadOptions()


	/**
	* Remove the old file of a file asset
	*
	* @param File				$asset		the file asset we are in
	* @param array 				&$existing	info about existing
	* @param File_Versioning	$fv		the file versioning object
	*
	* @return void
	* @access public
	*/
	public function removeOldFile(File $asset, &$existing, File_Versioning $fv)
	{
		// a new file with a new name means we need to remove the old
		// one from the repository (NOTE that this doesnt actually 'remove' the
		// files from the repository - just marks the branch as complete)
		// If we are setting filename then don't remove the file yet
		$result = $fv->remove($existing['path'], '');
		if ($result != FUDGE_FV_OK && $result != FUDGE_FV_NOT_CHECKED_OUT) {
			trigger_localised_error('CORE0042', E_USER_WARNING);
			return FALSE;
		}//end if

		// remove the old copy in the public directory (bug#1370)
		if ($asset->usePublicPath()) {
			$unrestricted = $asset->data_path_public.'/'.$existing['filename'];
			if (file_exists($unrestricted)) {
				$fv->clearOut($unrestricted);
			}//end if
		}//end if

	}//end removeOldFile()


	/**
	* Get the new file of a file asset
	*
	* @param File	&$asset	the file asset we are in
	* @param string $name	the new name for the file asset
	* @param array	&$info	the info array for the new file
	*
	* @return void
	* @access public
	*/
	public function getNewFile(File $asset, $name, Array $info)
	{
		// copy over the new uploaded file
		if (is_uploaded_file($info['tmp_name'])) {
			// a normal uploaded file
			if (!move_uploaded_file($info['tmp_name'], $asset->data_path.'/'.$name)) {
				trigger_localised_error('CORE0037', E_USER_WARNING, $info['tmp_name'], $asset->data_path.'/'.$name);
				return FALSE;
			}//end if
		} else if (isset($info['non_uploaded_file']) && $info['non_uploaded_file'] === TRUE) {
			// we have special permission to move a non-uploaded file
			if (!copy($info['tmp_name'], $asset->data_path.'/'.$name)) {
				trigger_localised_error('CORE0036', E_USER_WARNING, $info['tmp_name'], $asset->data_path.'/'.$name);
				return FALSE;
			}//end if
		} else if (isset($info['created_file']) && $info['created_file'] === TRUE) {
			// when empty file is created by matrix when no file is uploaded
			if (!rename($info['tmp_name'].'/'.$name, $asset->data_path.'/'.$name)) {
				trigger_localised_error('CORE0036', E_USER_WARNING, $info['tmp_name'], $asset->data_path.'/'.$name);
				return FALSE;
			}//end if

		} else {
			trigger_localised_error('CORE0067', E_USER_ERROR);
			return FALSE;
		}

	}//end getNewFile()


	/**
	* Get the info on the pre-uploaded file chosen by the user
	*
	* @param string	$prefix	prefix for form elements
	*
	* @return array
	* @access public
	*/
	public function getChosenFileInfo($prefix)
	{
		$info = Array();
		if (isset($_POST[$prefix.'_server_file']) && (!empty($_POST[$prefix.'_server_file']))) {
			$filename = trim($_POST[$prefix.'_server_file'], './');
			// prevent file path injection
			$filename = preg_replace('/[^A-Za-z0-9 _ .-]/', '', $filename);
			$info['name'] = $filename;
			$info['tmp_name'] = SQ_TEMP_PATH.'/'.$filename;
			$info['non_uploaded_file'] = TRUE;
		}
		return $info;

	}//end getChosenFileInfo()


	/**
	* Prints info about the current uploaded file
	*
	* @param File	$asset	the file asset
	*
	* @return void
	* @access public
	*/
	public function printExistingFileInfo(File $asset)
	{
		$existing = $asset->getExistingFile();
		if (!empty($existing)) {
			require_once SQ_FUDGE_PATH.'/general/datetime.inc';
			?>
			<div>
				<b><a href="<?php echo current_url(TRUE, TRUE).'?a='.$asset->id?>&now=<?php echo time()?>" target="_blank"><?php echo translate('Current File'); ?></a></b><br/>

				<b><?php echo translate('Info'); ?>:</b> <?php echo get_asset_tag_line($asset->id); ?><br/>

				<b><?php echo translate('Size'); ?>:</b> <?php echo easy_filesize($existing['Size'])?><br/>

				<b><?php echo translate('Updated'); ?>:</b> <?php echo readable_datetime($existing['modified'])?>

			</div>
			<?php
		}

	}//end printExistingFileInfo()


}//end class

?>
