<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: design_area_custom_image_edit_fns.inc,v 1.8 2003/09/26 05:26:24 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Custom_Image_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Custom_Image_Edit_Fns extends Design_Area_Edit_Fns
{

	/**
	* Prints the interface for uploading a file
	*
	* @param object Image				&$asset	the file asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintImageUpload(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess()) return false;
		file_upload($prefix);
		?>
		<div style="font-size: 7pt">
			<b>Note:</b> You may only upload files of type <?php echo $this->_getExtensionString($asset->allowed_extensions)?><br>
			<b>Note:</b> Image size is restricted to a maximum of <?php echo $asset->attr('max_width').' x '.$asset->attr('max_height'); ?> pixels
		</div>
		<?php

		return true;

	}//end paintImageUpload()


	/**
	* Processes the interface for uploading a file
	*
	* @param object Image				&$asset	the file asset
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processImageUpload(&$asset, &$o, $prefix)
	{
		$info = get_file_upload_info($prefix);

		// return on failed or no upload
		if ($info === false || empty($info)) return false;

		// check that the extension is allowed
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		if (!in_array(get_file_type($info['name']) , $asset->allowed_extensions)) {
			trigger_error('Uploaded file was not an image, You may only upload files of type '.$this->_getExtensionString($asset->allowed_extensions), E_USER_WARNING);
			return false;
		}

		// check that the image dimensions are not greater than our limits
		$size = getimagesize($info['tmp_name']);
		if ($size === false) return false;
		$max_width  = (int) $asset->attr('max_width');
		$max_height = (int) $asset->attr('max_height');
		if (($max_width && $size[0] > $max_width) || ($max_height && $size[1] > $max_height)) {
			trigger_error('Uploaded file was too large. You may only upload a file no more than '.$asset->attr('max_width').' x '.$asset->attr('max_height').' pixels', E_USER_WARNING);
			return false;
		}

		create_directory($asset->data_path_public);

		$old_image = $asset->attr('image');
		$new_image = $info['name'];

		// delete existing uploaded file if it exists
		if ($old_image && $old_image != $new_image && file_exists($asset->data_path_public.'/'.$old_image)) {
			if (!unlink($asset->data_path_public.'/'.$old_image)) {
				trigger_error('Failed deleting existing file', E_USER_WARNING);
				return false;
			}
		}

		// copy over the new uploaded file
		if (!move_uploaded_file($info['tmp_name'], $asset->data_path_public.'/'.$info['name'])) {
			trigger_error('Failed moving uploaded file to destination directory', E_USER_WARNING);
			return false;
		}
		
		if (!$asset->setAttrValue('image', $new_image)) return false;

		return true;

	}//end processImageUpload()

	/**
	* Prints the interface for uploading a file
	*
	* @param object Image				&$asset	the file asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCurrentImage(&$asset, &$o, $prefix)
	{
		$image = $asset->attr('image');
		if (empty($image) || !file_exists($asset->data_path_public.'/'.$image)) {
		?>
			No Image Uploaded
		<?php
		} else {
		?>
			<a href="<?php echo sq_web_path('data').'/assets/'.$asset->type().'/'.$asset->id.'/'.$image; ?>" target="_blank"><?php echo $image; ?></a>
		<?php
		}
		return true;

	}//end paintCurrentImage()


	/**
	* Get a formatted string of extensions (eg 1,2,3 and 4)
	*
	* @param array(string)	$extensions	array of extensions to format
	*
	* @return string
	* @access public
	*/
	function _getExtensionString($extensions)
	{
		if (empty($extensions)) return '';
		$last = array_pop($extensions);
		if (empty($extensions)) return $last;
		else return implode(', ', $extensions).' and '.$last;
	}//end _getExtensionString()


	/**
	* Returns an array of vars that are setable, used to create the parseString
	*
	* @param object Asset	$asset	the asset whose interface we are painting
	*
	* @return Array
	* @access public
	* @see Design_Area_Edit_Fns::createParseString()
	*/
	function _getSetableVars(&$asset)
	{
		$vars = parent::_getSetableVars($asset);
		// remove the image attribute
		unset($vars['image']);
		return $vars;
	}// end _getSetableVars()



}//end class
?>
