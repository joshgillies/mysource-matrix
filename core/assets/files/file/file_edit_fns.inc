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
* $Id: file_edit_fns.inc,v 1.40 2004/06/08 02:37:57 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* File_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
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
			require_once SQ_FUDGE_PATH.'/general/datetime.inc';
			?> 
				<b><a href="<?php echo current_url(true, true).'?a='.$asset->id?>&now=<?php echo time()?>" target="_blank">Current File</a></b><br/>
				<b>Size:</b> <?php echo easy_filesize($existing['size'])?><br/>
				<b>Updated:</b> <?php echo readable_datetime($existing['modified'])?><br/>
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
		if ($asset->writeAccess('attributes')) {
			file_upload($prefix);
		}
		$this->_printExistingFileInfo($asset);
		return true;

	}//end paintFileUpload()


	/**
	* Processes the interface for uploading a file
	* If you want to process an upload of a file that was not uploaded through PHP
	* (possible because of FTP or it was in a tar archive), ensure you pass the information
	* about the file in the $info array.
	* The field required are:
	*     name              => the filename without path (eg file.txt)
	*     tmp_name          => the location of the file on the file system
	*     non_uploaded_file => must equal TRUE
	* 
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

		// assetid will not be set if we are creating
		if (!$asset->id) return true;

		if (empty($info)) {
			$info = get_file_upload_info($prefix);
			// return on failed or no upload
			if ($info === false || empty($info)) return false;
		}

		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_names = make_valid_web_paths(Array($info['name']));
		$name = array_shift($valid_names);

		// make sure the new web path is not already is use
		if ($asset->name != $name) {
			$parents = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', null, 'minor');
			for($i = 0; $i < count($parents); $i++) {
				$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parents[$i]['majorid'], $parents[$i]['major_type_code']);
				if (is_null($parent)) continue;
				$bad_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($parent, Array($name), $asset->id);
				if (!empty($bad_paths)) {
					trigger_error('Failed updating web path, file not uploaded. Path already exists?', E_USER_WARNING);
					return false;
				}
			}
		}

		// create the destination directory, if necessary (NOTE that checking is internal for this)
		create_directory($asset->data_path);


		//// FILE VERSIONING ////

		$fv = &$GLOBALS['SQ_SYSTEM']->getFileVersioning();
		$existing = $asset->_getExistingFile();

		if ($asset->name != $name && !empty($existing)) {
			// a new file with a new name means we need to remove the old
			// one from the repository (NOTE that this doesnt actually 'remove' the
			// files from the repository - just marks the branch as complete)
			if (!$fv->remove($existing['path'], '', false)) {
				trigger_error('Failed removing file from repository', E_USER_WARNING);
				return false;
			}
		}

		// copy over the new uploaded file
		if (is_uploaded_file($info['tmp_name'])) {
			// a normal uploaded file
			if (!move_uploaded_file($info['tmp_name'], $asset->data_path.'/'.$name)) {
				trigger_error('Failed copying uploaded file ('.$info['tmp_name'].') to destination directory ('.$asset->data_path.'/'.$name.')', E_USER_WARNING);
				return false;
			}
		} else if (isset($info['non_uploaded_file']) && $info['non_uploaded_file'] === true) {
			// we have special permission to move a non-uploaded file
			if (!copy($info['tmp_name'], $asset->data_path.'/'.$name)) {
				trigger_error('Failed copying non-uploaded file ('.$info['tmp_name'].') to destination directory ('.$asset->data_path.'/'.$name.')', E_USER_WARNING);
				return false;
			}
		}

		// if we are overwriting our current file with one that has the same name,
		// we need to add a new version of the file to the repository
		if ($asset->name == $name && !empty($existing)) {
			$file_status = $fv->upToDate($asset->data_path.'/'.$name);
			if (FUDGE_FV_MODIFIED & $file_status) {
				if (!$fv->commit($asset->data_path.'/'.$name, '', false)) {
					trigger_error('Failed committing file version', E_USER_WARNING);
					return false;
				}
			}
		} else {
			// attempt to add the file to the repository
			if (!$fv->add($asset->data_path_suffix, $asset->data_path.'/'.$name, '', false)) {
				trigger_error('Failed adding file version', E_USER_WARNING);
				return false;
			}
		}

		// make sure we have the latest version of our file
		if (!$fv->checkOut($asset->data_path_suffix.'/'.$name, $asset->data_path)) {
			trigger_error('Failed checking out latest version of file', E_USER_WARNING);
			return false;
		}

		// set the name of the file to the newly uploaded one
		$asset->_tmp['uploading_file'] = true;
		$name_changed = $asset->setAttrValue('name', $name);
		if (!$asset->saveAttributes()) {
			trigger_error('Failed saving attributes for file', E_USER_WARNING);
			return false;
		}
		$asset->_tmp['uploading_file'] = false;

		if (!$asset->saveWebPaths(Array($name))) {
			trigger_error('Failed saving web paths', E_USER_WARNING);
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
		$thumb_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_3, 'thumbnail', true, 'thumbnail');
		if (!empty($thumb_link)) {
			$thumb = &$GLOBALS['SQ_SYSTEM']->am->getAsset($thumb_link['minorid'], $thumb_link['minor_type_code'], true);
			$existing = $thumb->_getExistingFile();
			if (!empty($existing)) {
				require_once SQ_FUDGE_PATH.'/general/datetime.inc';
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

		if ($asset->writeAccess('links')) asset_finder($prefix.'_thumbid', '', Array('thumbnail' => 'D'));

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
		if (!$asset->writeAccess('links')) return false;
		if (!isset($_POST[$prefix.'_thumbid']['assetid'])) return false;

		$am = &$GLOBALS['SQ_SYSTEM']->am;
		
		$thumbid = (int) $_POST[$prefix.'_thumbid']['assetid'];

		if (isset($_POST[$prefix.'_delete_thumbnail'])) {
			// they want to remove the thumbnail
			$thumb_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_3, 'thumbnail', true, 'thumbnail');
			if (!$asset->deleteLink($thumb_link['linkid'])) return false;
		}

		if (!$thumbid) return true;

		$thumb = &$am->getAsset($thumbid, '', true);
		if (!$thumb->id || get_class($thumb) != 'thumbnail') {
			return false;
		}
		
		// get the current thumbnail if there is one
		$thumb_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_3, 'thumbnail', true, 'thumbnail');
		
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
