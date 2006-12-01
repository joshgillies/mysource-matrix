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
* $Id: file_edit_fns.inc,v 1.70.2.4 2006/12/01 00:35:55 skim Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* File_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.70.2.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class File_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* All available protocols
	* @var Array(string)
	*/
	var $protos = Array('http', 'https');


	/**
	* Constructor
	*
	*/
	function File_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
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
			$asset->setAttrValue('name', $_REQUEST[$prefix.'_filename']);
		}

		return $process;

	}//end processFilename()


	/**
	* Prints the interface for uploading a file
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintFileUpload(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			file_upload($prefix);
			if ($this->havePreUploadedFiles($asset, $prefix)) {
				echo '&nbsp;&nbsp;';
				normal_button($prefix.'_choose_server_file_button', translate('core_upload_server_file_button'), 'this.parentNode.parentNode.style.display=\'none\';			 document.getElementById(\''.$prefix.'_simple_upload_button\').parentNode.parentNode.style.display=\'\'');
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
			?><br/><span style="font-size: 10px;"><?php echo translate('core_cannot_upload_file_larger_than', strtoupper(easy_filesize($max_size))); ?></span><?php
		}//end if

		return TRUE;

	}//end paintFileUpload()


	/**
	* Processes the interface for uploading a file
	*
	* If you want to process an upload of a file that was not uploaded through PHP
	* (possible because of FTP or it was in a tar archive), ensure you pass the information
	* about the file in the $info array.
	* The fields required are:
	*     name              => the filename without path (eg file.txt)
	*     tmp_name          => the location of the file on the file system
	*     non_uploaded_file => must equal TRUE
	*
	* @param object		&$asset				the file asset
	* @param object		&$o					the Backend Outputter object
	* @param string		$prefix				prefix for form elements
	* @param array		$info				info about the uploaded file
	* @param boolean	$redirect_in_limbo	redirect when the file name changes in limbo
	*
	* @return boolean
	* @access public
	*/
	function processFileUpload(&$asset, &$o, $prefix, $info=Array(), $redirect_in_limbo=TRUE)
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
			}
		}

		// make the info array look like a result from getExistingFile()
		if (!isset($info['path'])) {
			$info['path'] = array_get_index($info, 'tmp_name', '');
		}
		if (!isset($info['filename'])) {
			$info['filename'] = array_get_index($info, 'name', '');
		}
		if (!$asset->validFile($info)) return FALSE;

		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$name = current(make_valid_web_paths(Array($info['name'])));

		// make sure the new web path is not already is use
		if ($asset->name != $name) {
			$parents = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', NULL, 'minor');
			for ($i = 0; $i < count($parents); $i++) {
				$parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset($parents[$i]['majorid'], $parents[$i]['major_type_code']);
				if (is_null($parent)) continue;
				$bad_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($parent, Array($name), $asset->id);
				if (!empty($bad_paths)) {
					trigger_localised_error('CORE0055', E_USER_WARNING);
					return FALSE;
				}
			}
		}

		// create the destination directory, if necessary (NOTE that checking is internal for this)
		create_directory($asset->data_path);

		//// FILE VERSIONING ////

		$fv =& $GLOBALS['SQ_SYSTEM']->getFileVersioning();
		$existing = $asset->getExistingFile();

		if ($asset->name != $name && !empty($existing)) {
			// a new file with a new name means we need to remove the old
			// one from the repository (NOTE that this doesnt actually 'remove' the
			// files from the repository - just marks the branch as complete)
			$result = $fv->remove($existing['path'], '');
			if ($result != FUDGE_FV_OK && $result != FUDGE_FV_NOT_CHECKED_OUT) {
				trigger_localised_error('CORE0042', E_USER_WARNING);
				return FALSE;
			}

			// remove the old copy in the public directory (bug#1370)
			if ($asset->usePublicPath()) {
				$unrestricted = $asset->data_path_public.'/'.$existing['filename'];
				if (file_exists($unrestricted)) {
					$fv->clearOut($unrestricted);
				}
			}
		}

		// copy over the new uploaded file
		if (is_uploaded_file($info['tmp_name'])) {
			// a normal uploaded file
			if (!move_uploaded_file($info['tmp_name'], $asset->data_path.'/'.$name)) {
				trigger_localised_error('CORE0037', E_USER_WARNING, $info['tmp_name'], $asset->data_path.'/'.$name);
				return FALSE;
			}
		} else if (isset($info['non_uploaded_file']) && $info['non_uploaded_file'] === TRUE) {
			// we have special permission to move a non-uploaded file
			if (!copy($info['tmp_name'], $asset->data_path.'/'.$name)) {
				trigger_localised_error('CORE0036', E_USER_WARNING, $info['tmp_name'], $asset->data_path.'/'.$name);
				return FALSE;
			}
		} else {
			trigger_localised_error('CORE0067', E_USER_ERROR);
			return FALSE;
		}

		// if we are overwriting our current file with one that has the same name,
		// we need to add a new version of the file to the repository
		if ($asset->name == $name && !empty($existing)) {
			$file_status = $fv->upToDate($asset->data_path.'/'.$name);
			if (FUDGE_FV_MODIFIED & $file_status) {
				if (!$fv->commit($asset->data_path.'/'.$name, '')) {
					trigger_localised_error('CORE0034', E_USER_WARNING);
					return FALSE;
				}
			}
		} else {
			// attempt to add the file to the repository
			if (!$fv->add($asset->data_path_suffix, $asset->data_path.'/'.$name, '')) {
				trigger_localised_error('CORE0026', E_USER_WARNING);
				return FALSE;
			}
		}

		// make sure we have the latest version of our file
		if (!$fv->checkOut($asset->data_path_suffix.'/'.$name, $asset->data_path)) {
			trigger_localised_error('CORE0032', E_USER_WARNING);
			return FALSE;
		}

		// set the name of the file to the newly uploaded one
		$asset->_tmp['uploading_file'] = TRUE;
		$name_changed = $asset->setAttrValue('name', $name);
		if (!$asset->saveAttributes()) {
			trigger_localised_error('CORE0052', E_USER_WARNING);
			return FALSE;
		}
		$asset->_tmp['uploading_file'] = FALSE;

		if (!$asset->saveWebPaths(Array($name))) {
			trigger_localised_error('CORE0054', E_USER_WARNING);
			return FALSE;
		}

		if (SQ_IN_LIMBO && $name_changed && $redirect_in_limbo) {
			// if we are in limbo and the name of the file has changed, the URL will also change
			// so we need to redirect to the new limbo URL
			$o->setRedirect(strip_url($asset->getUrl()).'/'.SQ_CONF_LIMBO_SUFFIX);
		}

		$em =& $GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($asset, 'ContentsUpdated');

		return $asset->_updated();

	}//end processFileUpload()


	/**
	* Paint the interface for selecting a pre-uploaded file to use in this asset
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintFileChooser(&$asset, &$o, $prefix)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$options = $this->getPreuploadOptions($asset);
		combo_box($prefix.'_server_file', Array(''=>'')+$options, FALSE, Array());
		echo '&nbsp;&nbsp;';
		normal_button($prefix.'_simple_upload_button', translate('core_upload_new_file_button'), 'this.parentNode.parentNode.style.display=\'none\';			 document.getElementById(\''.$prefix.'_choose_server_file_button\').parentNode.parentNode.style.display=\'\'; document.getElementById(\''.$prefix.'_server_file\').value = \'\'');
		return TRUE;

	}//end paintFileChooser()


	/**
	* Process the interface where you can choose a pre-uploaded file
	*
	* Returns TRUE if a file was processed
	*
	* @param object		&$asset	the file asset
	* @param object		&$o		the Backend Outputter object
	* @param string		$prefix	prefix for form elements
	* @param boolean	$create	TRUE if this is being called from createAdditional and should run
	*							even though the asset is not yet created
	*
	* @return boolean
	* @access public
	*/
	function processFileChooser(&$asset, &$o, $prefix, $create=FALSE)
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
	* @param object	&$asset	The asset that might use the files
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function havePreUploadedFiles(&$asset, $prefix)
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
	* @param object	&$asset	The asset that might use the files
	*
	* @return array
	* @access protected
	*/
	function getPreuploadOptions(&$asset)
	{
		$options = Array();
		foreach (list_files(SQ_TEMP_PATH) as $filename) {
			if (substr($filename, 0, 1) == '.') continue;
			$options[$filename] = $filename;
		}
		return $options;

	}//end getPreuploadOptions()


	/**
	* Get the info on the pre-uploaded file chosen by the user
	*
	* @param string	$prefix	prefix for form elements
	*
	* @return array
	* @access protected
	*/
	function getChosenFileInfo($prefix)
	{
		$info = Array();
		if (isset($_POST[$prefix.'_server_file']) && (!empty($_POST[$prefix.'_server_file']))) {
			$info['name'] = $_POST[$prefix.'_server_file'];
			$info['tmp_name'] = SQ_TEMP_PATH.'/'.$_POST[$prefix.'_server_file'];
			$info['non_uploaded_file'] = TRUE;
		}
		return $info;

	}//end getChosenFileInfo()


	/**
	* Prints info about the current uploaded file
	*
	* @param object	&$asset	the file asset
	*
	* @return void
	* @access public
	*/
	function printExistingFileInfo(&$asset)
	{
		$existing = $asset->getExistingFile();
		if (!empty($existing)) {
			require_once SQ_FUDGE_PATH.'/general/datetime.inc';
			?>
			<div>
				<b><a href="<?php echo current_url(TRUE, TRUE).'?a='.$asset->id?>&now=<?php echo time()?>" target="_blank"><?php echo translate('core_current_file'); ?></a></b><br/>
				<b><?php echo translate('size'); ?>:</b> <?php echo easy_filesize($existing['size'])?><br/>
				<b><?php echo translate('updated'); ?>:</b> <?php echo readable_datetime($existing['modified'])?>
			</div>
			<?php
		}

	}//end printExistingFileInfo()


}//end class

?>
