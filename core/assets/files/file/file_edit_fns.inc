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
				</p>
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

		// delete existing uploaded file if it exists
		if (is_file($asset->data_path.'/'.$asset->attr('name'))) {
			if (!unlink($asset->data_path.'/'.$asset->attr('name'))) {
				trigger_error('Failed deleting existing file', E_USER_WARNING);
				return false;
			}
		}

		// copy over the new uploaded file
		if (!copy($info['tmp_name'], $dest.'/'.$name)) {
			trigger_error('Failed copying uploaded file to destination directory', E_USER_WARNING);
			return false;
		}

		// set the name of the file to the newly uploaded one
		$asset->_tmp['uploading_file'] = true;
		$asset->setAttrValue('name', $name);
		unset($asset->_tmp['uploading_file']);
		
		return true;
	}//end processFileUpload()


	function paintThumbnail(&$asset, &$o, $prefix)
	{
		// get the current thumbnail if there is one
		$thumb_link = $asset->getLink(SQ_LINK_USES, 'thumbnail', true, 'thumbnail');
		if (!empty($thumb_link)) {
			$thumb = $GLOBALS['SQ_SYSTEM']->am->getAsset($thumb_link['minorid'], '', true);
			$existing = $thumb->_getExistingFile();
			if (!empty($existing)) {
				?> 
					<p style="font-size: 7pt">
					<b>Current Thumbnail</b><br>
					<b>Filename:</b> <?php echo $thumb->attr('name')?><br>
					<b>Size:</b> <?php echo easy_filesize($existing['size'])?><br>
					<b>Updated:</b> <?php echo readable_datetime($existing['modified'])?><br>
					</p>
				<?php
			} else {
				?> 
					<p style="font-size: 7pt">
					<b>Current Thumbnail</b><br>
					<b>Filename:</b> <?php echo $thumb->attr('name')?><br>
					<b>Note:</b> <i>No file has been uploaded for this thumbnail</i>
					</p>
				<?php
			}
		}

		?>
		<input type="text" name="<?php echo $prefix?>_thumbid" value="">
		<?php
	}

	
	function processThumbnail(&$asset, $prefix)
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		
		$thumbid = (int) $_POST[$prefix.'_thumbid'];
		if (!$thumbid) return true;

		$thumb = $am->getAsset($thumbid, '', true);
		if (!$thumb->id || get_class($thumb) != 'thumbnail') {
			return false;
		}
		
		// get the current thumbnail if there is one
		$thumb_link = $asset->getLink(SQ_LINK_USES, 'thumbnail', true, 'thumbnail');
		
		// return if they are trying to set the thumb to the current thumb
		if ($thumb_link['minorid'] == $thumb->id) return true;

		if (!empty($thumb_link)) {
			if (!$asset->deleteLink($thumb_link['linkid'])) return false;
		}

		// create new link to thumbnail
		$asset->createLink($thumb, SQ_LINK_USES, 'thumbnail');

		return true;
	}

}//end class
?>
