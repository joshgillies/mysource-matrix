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
* $Id: image_variety_edit_fns.inc,v 1.4 2005/02/03 18:57:17 brobertson Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Image_Variety_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Image_Variety_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Image_Variety_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		// override static screens - only display details because the others don't apply
		$this->static_screens = Array(
									'details' => Array(
													'name'			=> 'Details',
													'force_unlock'	=> false,
													'lock_type'		=> 'none',
												),
								);

	}//end constructor


	/**
	* Paint the interface for the image variety
	*
	* @param object Image_Variety		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintInlineInterface(&$asset, &$o, $prefix)
	{
		require_once 'XML/Tree/Node.php';
		$wa = $asset->writeAccess('attributes');

		if (!$asset->id && !$wa) return false;

		$int_edit_params  = new XML_Tree_Node('int', '', Array('width' => 8));
		$bool_edit_params = new XML_Tree_Node('boolean', '', Array('true_text' => 'Yes', 'false_text' => 'No'));

		$o->openSection(($asset->name) ? $asset->name : 'New');
			if ($asset->id) {
				$o->openField('Current File');
					?>
						<a href="<?php echo current_url(true, true).'?a='.$asset->id?>&now=<?php echo time()?>" target="_blank">Click to open in a new window</a>
					<?php
				$o->closeField();
			}

			if ($asset->name == '') {
				$o->openField('Name');
					$text_edit_params = new XML_Tree_Node('text', '', Array('width' => 15));
					$name = &$asset->getAttribute('name');
					$name->setEditParams($text_edit_params);
					$name->paint($prefix.'_'.$name->name, !$wa);
				$o->closeField();
			}

			$o->openField('', 'new_line');
				$o->note('One of either the width or height attribute can be set to zero.');
				$o->note('This will mean that this dimension is not important and will force the aspect ratio of the original image to be maintained');
				$o->note('<b>OR</b>');
				$o->note('You can choose to upload a completely different image file (perhaps a cropped version of the original image).');
				$o->note('Image file must be the same type as the original image (e.g. jpg, gif or png)');
				echo '<br>';
			$o->closeField();
			$o->openField('Width');
				$width = &$asset->getAttribute('width');
				$width->setEditParams($int_edit_params);
				$width->paint($prefix.'_'.$width->name, !$wa);
			$o->closeField();
			$o->openField('Height');
				$height = &$asset->getAttribute('height');
				$height->setEditParams($int_edit_params);
				$height->paint($prefix.'_'.$height->name, !$wa);
			$o->closeField();
			$o->openField('Using Upload');
				$using_uploaded = &$asset->getAttribute('using_uploaded');
				$using_uploaded->setEditParams($bool_edit_params);
				$using_uploaded->paint($prefix.'_'.$using_uploaded->name, !$wa);
			$o->closeField();
			if ($asset->id) {
				$o->openField('New Upload ?');
					file_upload($prefix.'_upload_file');
				$o->closeField();
			}
		$o->closeSection();

		return true;

	}//end paintInlineValueInterface();


	/**
	* Process the interface for the image variety
	*
	* @param object Image_Variety		&$asset		the asset whose interface we are processing
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processInlineInterface(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return false;

		$changes_made = false;

		$vars = Array('width', 'height', 'using_uploaded');
		if ($asset->attr('name') == '') array_unshift($vars, 'name');

		foreach($vars as $attr_name) {
			$attr = &$asset->getAttribute($attr_name);
			$attr->process($prefix.'_'.$attr->name);
			if ($attr->processed && $asset->setAttrValue($attr->name, $attr->value)) {
				$changes_made = true;
			}
		}

		if ($asset->id && $asset->attr('using_uploaded')) {
			if ($this->_processUploadedFile($asset, $prefix)) {
				$changes_made = true;
			}
		}

		if (!$changes_made) return false;
		else return $asset->saveAttributes();

	}//end processInlineValueInterface();


	/**
	* Takes care of processing any uploaded file
	*
	* @param object Image_Variety		&$asset		the asset whose interface we are processing
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for form elements
	*
	* @access public
	* @return boolean
	*/
	function _processUploadedFile($asset, $prefix)
	{
		$info = get_file_upload_info($prefix.'_upload_file');
		// return on failed or no upload
		if ($info === false || empty($info)) return false;

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$fv = &$GLOBALS['SQ_SYSTEM']->getFileVersioning();

		if (!is_dir($asset->data_path) && !create_directory($asset->data_path)) return false;

		$image = &$GLOBALS['SQ_SYSTEM']->am->getAsset($asset->_imageid);
		if (is_null($image)) return false;

		$img_ext = get_file_type($image->name);
		$upload_ext = get_file_type($info['name']);

		if ($img_ext != $upload_ext) {
			trigger_error('An image uploaded for a variety MUST be of the same type as the original image.', E_USER_WARNING);
			return false;
		}

		$d_old_filename = $asset->attr('filename');
		$d_new_filename = $asset->name.'.'.$img_ext;
		$d_old_file     = $asset->data_path.'/'.$d_old_filename;
		$d_new_file     = $asset->data_path.'/'.$d_new_filename;

		// if the filename has changed then we need to remove the old file
		if ($d_old_filename != $d_new_filename) {
			$asset->deleteImage();

		} else {
			// make sure we have the latest version checked out if we are modifying the file
			$up_to_date = $fv->upToDate($d_new_file);
			if ($up_to_date & FUDGE_FV_OLD_VERSION) {
				if (!$fv->checkOut($asset->data_path_suffix.'/'.$d_new_filename, $asset->data_path)) {
					trigger_error('Update failed checking-out latest version of file', E_USER_WARNING);
					return false;
				}
			}

		}// endif

		if (commit_file_upload($prefix.'_upload_file', $d_new_file, true, 0)) return false;

		$up_to_date = $fv->upToDate($d_new_file);

		// file not in repository
		if ($up_to_date & FUDGE_FV_NOT_CHECKED_OUT) {
			if (!$fv->add($asset->data_path_suffix, $d_new_file, '', false)) return false;

			if (!$fv->checkOut($asset->data_path_suffix.'/'.$d_new_filename, $asset->data_path)) {
				trigger_error('Update failed checking-out latest version of file', E_USER_WARNING);
				return false;
			}

		} elseif ($up_to_date & FUDGE_FV_MODIFIED) {
			if (!$fv->commit($d_new_file, '', false)) return false;

		}

		// if the filename has changed then we need to update the look ups
		if ($d_old_filename != $d_new_filename) {
			if (!$asset->setAttrValue('filename', $d_new_filename)) return false;
			if (!$asset->updateLookups($image->usePublicPath())) return false;
		}

		return true;

	}// end _processUploadedFile()


}//end class

?>
