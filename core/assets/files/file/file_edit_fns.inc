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
* $Id: file_edit_fns.inc,v 1.60.2.5 2006/01/19 01:09:40 skim Exp $
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
* @version $Revision: 1.60.2.5 $
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
	* Prints the interface for uploading a file
	*
	* @param object File				&$asset	the file asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
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
				$max_size = min($post_size, $mem_limit_size);
			}
			?><br/><span style="font-size: 10px;"><?php echo translate('core_cannot_upload_file_larger_than', strtoupper(easy_filesize($max_size))); ?></span><?php
		}
		return true;

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
	* @param object File				&$asset	the file asset
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for form elements
	* @param array(string)				$info	info about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	function processFileUpload(&$asset, &$o, $prefix, $info=Array())
	{
		if (!$asset->writeAccess('attributes')) return false;
		if (empty($info) && isset($_POST[$prefix.'_server_file']) && !empty($_POST[$prefix.'_server_file'])) {
			return false;
		}

		// assetid will not be set if we are creating
		if (!$asset->id) return true;

		if (empty($info)) {
			$info = get_file_upload_info($prefix);
			// return on failed or no upload
			if ($info === false || empty($info)) {
				return false;
			}
		}

		// make the info array look like a result from getExistingFile()
		if (!isset($info['path'])) {
			$info['path'] = array_get_index($info, 'tmp_name', '');
		}
		if (!isset($info['filename'])) {
			$info['filename'] = array_get_index($info, 'name', '');
		}
		if (!$asset->validFile($info)) return false;

		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$name = current(make_valid_web_paths(Array($info['name'])));

		// make sure the new web path is not already is use
		if ($asset->name != $name) {
			$parents = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', null, 'minor');
			for ($i = 0; $i < count($parents); $i++) {
				$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parents[$i]['majorid'], $parents[$i]['major_type_code']);
				if (is_null($parent)) continue;
				$bad_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($parent, Array($name), $asset->id);
				if (!empty($bad_paths)) {
					trigger_localised_error('CORE0055', E_USER_WARNING);
					return false;
				}
			}
		}

		// create the destination directory, if necessary (NOTE that checking is internal for this)
		create_directory($asset->data_path);


		//// FILE VERSIONING ////

		$fv = &$GLOBALS['SQ_SYSTEM']->getFileVersioning();
		$existing = $asset->getExistingFile();

		if ($asset->name != $name && !empty($existing)) {
			// a new file with a new name means we need to remove the old
			// one from the repository (NOTE that this doesnt actually 'remove' the
			// files from the repository - just marks the branch as complete)
			$result = $fv->remove($existing['path'], '', false);
			if ($result != FUDGE_FV_OK && $result != FUDGE_FV_NOT_CHECKED_OUT) {
				trigger_localised_error('CORE0042', E_USER_WARNING);
				return false;
			}
		}

		// copy over the new uploaded file
		if (is_uploaded_file($info['tmp_name'])) {
			// a normal uploaded file
			if (!move_uploaded_file($info['tmp_name'], $asset->data_path.'/'.$name)) {
				trigger_localised_error('CORE0037', E_USER_WARNING, $info['tmp_name'], $asset->data_path.'/'.$name);
				return false;
			}
		} else if (isset($info['non_uploaded_file']) && $info['non_uploaded_file'] === true) {
			// we have special permission to move a non-uploaded file
			if (!copy($info['tmp_name'], $asset->data_path.'/'.$name)) {
				trigger_localised_error('CORE0036', E_USER_WARNING, $info['tmp_name'], $asset->data_path.'/'.$name);
				return false;
			}
		} else {
			trigger_localised_error('CORE0067', E_USER_ERROR);
			return false;
		}

		// if we are overwriting our current file with one that has the same name,
		// we need to add a new version of the file to the repository
		if ($asset->name == $name && !empty($existing)) {
			$file_status = $fv->upToDate($asset->data_path.'/'.$name);
			if (FUDGE_FV_MODIFIED & $file_status) {
				if (!$fv->commit($asset->data_path.'/'.$name, '', false)) {
					trigger_localised_error('CORE0034', E_USER_WARNING);
					return false;
				}
			}
		} else {
			// attempt to add the file to the repository
			if (!$fv->add($asset->data_path_suffix, $asset->data_path.'/'.$name, '', false)) {
				trigger_localised_error('CORE0026', E_USER_WARNING);
				return false;
			}
		}

		// make sure we have the latest version of our file
		if (!$fv->checkOut($asset->data_path_suffix.'/'.$name, $asset->data_path)) {
			trigger_localised_error('CORE0032', E_USER_WARNING);
			return false;
		}

		// set the name of the file to the newly uploaded one
		$asset->_tmp['uploading_file'] = true;
		$name_changed = $asset->setAttrValue('name', $name);
		if (!$asset->saveAttributes()) {
			trigger_localised_error('CORE0052', E_USER_WARNING);
			return false;
		}
		$asset->_tmp['uploading_file'] = false;

		if (!$asset->saveWebPaths(Array($name))) {
			trigger_localised_error('CORE0054', E_USER_WARNING);
			return false;
		}

		if (SQ_IN_LIMBO && $name_changed) {
			// if we are in limbo and the name of the file has changed, the URL will also change
			// so we need to redirect to the new limbo URL
			$o->setRedirect(strip_url($asset->getUrl()).'/'.SQ_CONF_LIMBO_SUFFIX);
		}

		return $asset->_updated();

	}//end processFileUpload()


	/**
	* Paint the interface for selecting a pre-uploaded file to use in this asset
	*
	* @param object File				&$asset	the file asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @access public
	* @return boolean	true
	*/
	function paintFileChooser(&$asset, &$o, $prefix)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$options = $this->getPreuploadOptions($asset);
		combo_box($prefix.'_server_file', Array(''=>'')+$options, false, Array());
		echo '&nbsp;&nbsp;';
		normal_button($prefix.'_simple_upload_button', translate('core_upload_new_file_button'), 'this.parentNode.parentNode.style.display=\'none\';			 document.getElementById(\''.$prefix.'_choose_server_file_button\').parentNode.parentNode.style.display=\'\'; document.getElementById(\''.$prefix.'_server_file\').value = \'\'');
		return true;

	}//end paintFileChooser()


	/**
	* Process the interface where you can choose a pre-uploaded file
	*
	* @param object File				&$asset	the file asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	* @param boolean					$create	True if this is being called from createAdditional and should run
	*											even though the asset is not yet created
	* @access public
	* @return boolean	true if a file was processed
	*/
	function processFileChooser(&$asset, &$o, $prefix, $create=false)
	{
		// Unless explicitly told otherwise, don't do anything if the asset is not yet completely created
		if (!$create && (empty($asset->id) || array_get_index($asset->_tmp, '__creating__'))) {
			return true;
		}

		if ($asset->writeAccess('attributes') && isset($_POST[$prefix.'_server_file']) && (!empty($_POST[$prefix.'_server_file']))) {
			$info = $this->getChosenFileInfo($prefix);
			if ($this->processFileUpload($asset, $o, $prefix, $info)) {
				unlink($info['tmp_name']);
				return true;
			}
		}
		return false;

	}//end processFileChooser()


	/**
	* Find out if there are any pre-uploaded files we can use
	*
	* @param object Asset	&$asset		The asset that might use the files
	* @param string			$prefix		the prefix for this asset
	*
	* @access protected
	* @return boolean	true if there are some files to use
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
	*
	* @param	&$asset		The asset that might use the files
	*
	* @access protected
	* @return array		value => value to be used with combo_box
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
	* @param string						$prefix	prefix for form elements
	*
	* @access protected
	* @return array
	*/
	function getChosenFileInfo($prefix)
	{
		$info = Array();
		if (isset($_POST[$prefix.'_server_file']) && (!empty($_POST[$prefix.'_server_file']))) {
			$info['name'] = $_POST[$prefix.'_server_file'];
			$info['tmp_name'] = SQ_TEMP_PATH.'/'.$_POST[$prefix.'_server_file'];
			$info['non_uploaded_file'] = true;
		}
		return $info;

	}//end getChosenFileInfo()


	/**
	* Prints info about the current uploaded file
	*
	* @param object File	&$asset	the file asset
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
				<b><a href="<?php echo current_url(true, true).'?a='.$asset->id?>&now=<?php echo time()?>" target="_blank"><?php echo translate('core_current_file'); ?></a></b><br/>
				<b><?php echo translate('size'); ?>:</b> <?php echo easy_filesize($existing['size'])?><br/>
				<b><?php echo translate('updated'); ?>:</b> <?php echo readable_datetime($existing['modified'])?>
			</div>
			<?php
		}

	}//end printExistingFileInfo()


}//end class

?>
