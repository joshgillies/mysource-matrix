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
* @package Resolve_Packages
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
	}//end constructor


	/**
	* Prints info about the current uploaded file
	*
	* @param object File	&$asset	the file asset
	*
	* @return void
	* @access public
	*/
	function _printExistingFileInfo(&$asset)
	{
		$existing = $asset->_getExistingFile();
		if (!empty($existing)) {
			?> 
				<p style="font-size: 7pt">
				<b><a href="<?php echo current_url().'?a='.$asset->id?>" target="_blank">Current File</a></b><br/>
				<b>Size:</b> <?php echo easy_filesize($existing['size'])?><br/>
				<?php
				if ($existing['width'] && $existing['height']) {
					?><b>Dimensions:</b> <?php echo $existing['width'].' x '.$existing['height']?> pixels<br/><?php
				}
				?>
				<b>Updated:</b> <?php echo readable_datetime($existing['modified'])?><br/>
				</p>
			<?php
		}

	}//end _printExistingFileInfo()


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
		if ($asset->writeAccess()) {
			file_upload($prefix);
		}
		$this->_printExistingFileInfo($asset);
		return true;
	}//end paintFileUpload()


	/**
	* Processes the interface for uploading a file
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
		if (!$asset->writeAccess()) return false;

		if (empty($info)) {
			$info = get_file_upload_info($prefix);
			// return on failed or no upload
			if ($info === false || empty($info)) return false;
		}

		// assetid will not be set if we are creating
		if (!$asset->id) return true;

		$name = $info['name'];

		// make sure the new web path is not already is use
		if (!@$asset->saveWebPaths(Array($name))) {
			trigger_error('Failed updating web path, file not uploaded. Path already exists?', E_USER_WARNING);
			return false;
		}

		// create the destination directory, if necessary
		create_directory($asset->data_path);

		// delete existing uploaded file if it exists
		if (is_file($asset->data_path.'/'.$asset->attr('name'))) {
			if (!unlink($asset->data_path.'/'.$asset->attr('name'))) {
				trigger_error('Failed deleting existing file', E_USER_WARNING);
				$asset->saveWebPaths(Array($asset->attr('name')));
				return false;
			}
		}

		// copy over the new uploaded file
		if (!move_uploaded_file($info['tmp_name'], $asset->data_path.'/'.$name)) {
			trigger_error('Failed copying uploaded file to destination directory', E_USER_WARNING);
			$asset->saveWebPaths(Array($asset->attr('name')));
			return false;
		}

		// set the name of the file to the newly uploaded one
		$asset->_tmp['uploading_file'] = true;
		$asset->setAttrValue('name', $name);
		unset($asset->_tmp['uploading_file']);
		
		return true;
	}//end processFileUpload()


	/**
	* Prints the interface for adding a thumbnail
	*
	* @param object File				&$asset	the file asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintThumbnail(&$asset, &$o, $prefix)
	{
		// get the current thumbnail if there is one
		$thumb_link = $asset->getLink(SQ_LINK_TYPE_3, 'thumbnail', true, 'thumbnail');
		if (!empty($thumb_link)) {
			$thumb = &$GLOBALS['SQ_SYSTEM']->am->getAsset($thumb_link['minorid'], '', true);
			$existing = $thumb->_getExistingFile();
			if (!empty($existing)) {
				?> 
					<p style="font-size: 7pt">
					<b><a href="<?php echo current_url().'?a='.$thumb->id?>" target="_blank">Current Thumbnail</a></b><br/>
					<b>Filename:</b> <?php echo $thumb->attr('name')?><br/>
					<b>Size:</b> <?php echo easy_filesize($existing['size'])?><br/>
					<b>Dimensions:</b> <?php echo $existing['width'].' x '.$existing['height']?><br/>
					<b>Updated:</b> <?php echo readable_datetime($existing['modified'])?><br/>
					<input type="checkbox" name="<?php echo $prefix?>_delete_thumbnail" value="1">&nbsp;Remove Thumbnail<br/>
					</p>
				<?php
			} else {
				?> 
					<p style="font-size: 7pt">
					<b>Current Thumbnail</b><br/>
					<b>Filename:</b> <?php echo $thumb->attr('name')?><br/>
					<b>Note:</b> <i>No file has been uploaded for this thumbnail</i>
					</p>
				<?php
			}
		} else {
			echo 'No thumbnail attached<br/>';
		}

		if ($asset->writeAccess()) asset_finder($prefix.'_thumbid', '', Array('thumbnail' => 'D'));
	}//end paintThumbnail()


	/**
	* Processes the interface for creating a thumbnail
	*
	* @param object File				&$asset	the file asset
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processThumbnail(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess()) return false;
		if (!isset($_POST[$prefix.'_thumbid'])) return false;

		$am = &$GLOBALS['SQ_SYSTEM']->am;
		
		$thumbid = (int) $_POST[$prefix.'_thumbid'];

		if (isset($_POST[$prefix.'_delete_thumbnail'])) {
			// they want to remove the thumbnail
			$thumb_link = $asset->getLink(SQ_LINK_TYPE_3, 'thumbnail', true, 'thumbnail');
			if (!$asset->deleteLink($thumb_link['linkid'])) return false;
		}

		if (!$thumbid) return true;

		$thumb = &$am->getAsset($thumbid, '', true);
		if (!$thumb->id || get_class($thumb) != 'thumbnail') {
			return false;
		}
		
		// get the current thumbnail if there is one
		$thumb_link = $asset->getLink(SQ_LINK_TYPE_3, 'thumbnail', true, 'thumbnail');
		
		// return if they are trying to set the thumb to the current thumb
		if ($thumb_link['minorid'] == $thumb->id) return true;

		if (!empty($thumb_link)) {
			if (!$asset->deleteLink($thumb_link['linkid'])) return false;
		}

		// create new link to thumbnail
		$asset->createLink($thumb, SQ_LINK_TYPE_3, 'thumbnail');

		return true;
	}//end processThumbnail

}//end class
?>
