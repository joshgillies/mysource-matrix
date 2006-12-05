<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: design_area_custom_image_edit_fns.inc,v 1.21 2006/12/05 22:42:12 emcdonald Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Custom_Image_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.21 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Custom_Image_Edit_Fns extends Design_Area_Edit_Fns
{


	/**
	* Prints the interface for uploading a file
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintImageUpload(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		file_upload($prefix);
		$o->note(translate('core_only_upload_files_of_type', $this->_getExtensionString($asset->allowed_extensions)));
		$o->note(translate('core_image_size_restricted', $asset->attr('max_width'), $asset->attr('max_height')));

		return TRUE;

	}//end paintImageUpload()


	/**
	* Processes the interface for uploading a file
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processImageUpload(&$asset, &$o, $prefix)
	{
		$info = get_file_upload_info($prefix);

		// return on failed or no upload
		if ($info === FALSE || empty($info)) {
			return FALSE;
		}

		// check that the extension is allowed
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		if (!in_array(get_file_type($info['name']) , $asset->allowed_extensions)) {
			trigger_localised_error('CORE0190', E_USER_WARNING, $this->_getExtensionString($asset->allowed_extensions));
			return FALSE;
		}

		// check that the image dimensions are not greater than our limits
		$size = getimagesize($info['tmp_name']);
		if ($size === FALSE) return FALSE;
		$max_width  = (int) $asset->attr('max_width');
		$max_height = (int) $asset->attr('max_height');
		if (($max_width && $size[0] > $max_width) || ($max_height && $size[1] > $max_height)) {
			trigger_localised_error('CORE0191', E_USER_WARNING, $asset->attr('max_width'), $asset->attr('max_height'));
			return FALSE;
		}

		create_directory($asset->data_path_public);

		$old_image = $asset->attr('image');
		$new_image = $info['name'];

		// delete existing uploaded file if it exists
		if ($old_image && $old_image != $new_image && file_exists($asset->data_path_public.'/'.$old_image)) {
			if (!unlink($asset->data_path_public.'/'.$old_image)) {
				trigger_localised_error('CORE0163', E_USER_WARNING);
				return FALSE;
			}
		}

		// copy over the new uploaded file
		if (!move_uploaded_file($info['tmp_name'], $asset->data_path_public.'/'.$info['name'])) {
			trigger_localised_error('CORE0166', E_USER_WARNING);
			return FALSE;
		}

		if (!$asset->setAttrValue('image', $new_image)) {
			return FALSE;
		}

		return TRUE;

	}//end processImageUpload()


	/**
	* Prints the interface for uploading a file
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCurrentImage(&$asset, &$o, $prefix)
	{
		$image = $asset->attr('image');
		if (empty($image) || !file_exists($asset->data_path_public.'/'.$image)) {
			echo translate('core_design_area_custom_image_no_upload');
		} else {
		?>
			<a href="<?php echo sq_web_path('data').'/assets/'.$asset->type().'/'.$asset->id.'/'.$image; ?>" target="_blank"><?php echo $image; ?></a>
		<?php
		}
		return TRUE;

	}//end paintCurrentImage()


	/**
	* Get a formatted string of extensions (eg 1,2,3 and 4)
	*
	* @param array	$extensions	array of extensions to format
	*
	* @return string
	* @access public
	*/
	function _getExtensionString($extensions)
	{
		if (empty($extensions)) return '';
		$last = array_pop($extensions);
		if (empty($extensions)) {
			return $last;
		} else {
			return implode(', ', $extensions).' and '.$last;
		}

	}//end _getExtensionString()


}//end class
?>
