<?php

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* File_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
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
		file_upload($prefix);

		$existing = $asset->_getExistingFile();
		if (!empty($existing)) {
			?> 
				<p style="font-size: 7pt">
				<b>Current File</b><br>
				<b>Size:</b> <?php echo easy_filesize($existing['size'])?><br>
				<b>Updated:</b> <?php echo readable_datetime($existing['modified'])?><br>
			<?php
		}
		return true;
	}//end paintFileUpload()


	/**
	* Processes the interface for uploading a file
	*
	* @param object File				&$asset	the file asset
	* @param string						$prefix	prefix for form elements
	* @param array(string)				$info	info about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	function processFileUpload(&$asset, $prefix, $info=Array())
	{
		if (empty($info)) {
			$info = get_file_upload_info($prefix);

			// return on failed or no upload
			if ($info === false || empty($info)) return false;
			if (!$info['name'] || !is_uploaded_file($info['tmp_name'])) return false;
		}

		$name = $info['name'];
		$ext = strtolower(substr(strrchr($name,'.'), 1));

		// make sure its not too large
		if ((int)$asset->attr('max_filesize') && $info['size'] > (int)$asset->attr('max_filesize')) {
			trigger_error('Uploaded file was above allowed filesize', E_USER_WARNING);
			return false;
		}

		// create the destination directory, if necessary
		$dest = $asset->data_path;
		create_directory($dest);

		// delete existing uploaded file
		$existing_files = list_files($asset->data_path);
		foreach ($existing_files as $filename) {
			if (preg_match('/^uploaded_file(.[^.]+)?$/', $filename)) {
				unlink($asset->data_path.'/'.$filename);
			}
		}

		// copy over the new uploaded file
		if (!copy($info['tmp_name'], $dest.'/uploaded_file'.(($ext) ? '.'.$ext : ''))) {
			trigger_error('Failed copying uploaded file to destination directory', E_USER_WARNING);
			return false;
		}

		// set the name of the file to the newly uploaded one
		$asset->setAttrValue('name', $name);
		
		return true;
	}//end processFileUpload()

}//end class
?>
