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
* $Id: design_area_custom_image_edit_fns.inc,v 1.25 2012/08/30 01:09:06 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';

/**
* Design_Area_Custom_Image_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.25 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Custom_Image_Edit_Fns extends Design_Area_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor()


	/**
	* Prints the interface for uploading a file
	*
	* @param Design_Area_Custom_Image	$asset	the design area asset
	* @param Backend_Outputter			$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintImageUpload(Design_Area_Custom_Image $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		file_upload($prefix);
		$o->note(sprintf(translate('<b>NOTE:</b> You may only upload files of type %s.'), make_readable_list($asset->allowed_extensions)));

		$o->note(sprintf(translate('<b>NOTE:</b> Image size is restricted to a maximum of %1$s x %2$s pixels.'), $asset->attr('max_width'), $asset->attr('max_height')));


		return TRUE;

	}//end paintImageUpload()


	/**
	* Processes the interface for uploading a file
	*
	* @param Design_Area_Custom_Image	$asset	the design area asset
	* @param Backend_Outputter			$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function processImageUpload(Design_Area_Custom_Image $asset, Backend_Outputter $o, $prefix)
	{
		$info = get_file_upload_info($prefix);

		// return on failed or no upload
		if ($info === FALSE || empty($info)) {
			return FALSE;
		}

		// check that the extension is allowed
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		if (!in_array(get_file_type($info['name']) , $asset->allowed_extensions)) {
			trigger_localised_error('CORE0190', sprintf(translate('Uploaded file not an image. You may only upload files of type %s'), make_readable_list($asset->allowed_extensions)), E_USER_WARNING);
			return FALSE;
		}

		// check that the image dimensions are not greater than our limits
		$size = getimagesize($info['tmp_name']);
		if ($size === FALSE) return FALSE;
		$max_width  = (int) $asset->attr('max_width');
		$max_height = (int) $asset->attr('max_height');
		if (($max_width && $size[0] > $max_width) || ($max_height && $size[1] > $max_height)) {
			trigger_localised_error('CORE0191', sprintf(translate('Uploaded file was too large. File cannot exceed %1$s x %2$s pixels'), $asset->attr('max_width'), $asset->attr('max_height')), E_USER_WARNING);
			return FALSE;
		}

		create_directory($asset->data_path_public);

		$old_image = $asset->attr('image');
		$new_image = $info['name'];

		// delete existing uploaded file if it exists
		if ($old_image && $old_image != $new_image && file_exists($asset->data_path_public.'/'.$old_image)) {
			if (!unlink($asset->data_path_public.'/'.$old_image)) {
				trigger_localised_error('CORE0163', translate('Failed deleting existing file'), E_USER_WARNING);
				return FALSE;
			}
		}

		// copy over the new uploaded file
		if (!move_uploaded_file($info['tmp_name'], $asset->data_path_public.'/'.$info['name'])) {
			trigger_localised_error('CORE0166', translate('Failed moving uploaded file to destination directory'), E_USER_WARNING);
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
	* @param Design_Area_Custom_Image	$asset	the design area asset
	* @param Backend_Outputter			$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintCurrentImage(Design_Area_Custom_Image $asset, Backend_Outputter $o, $prefix)
	{
		$image = $asset->attr('image');
		if (empty($image) || !file_exists($asset->data_path_public.'/'.$image)) {
			echo translate('No Image Uploaded');

		} else {
		?>
			<a href="<?php echo sq_web_path('data').'/assets/'.$asset->type().'/'.get_asset_hash($asset->id).'/'.$asset->id.'/'.$image; ?>" target="_blank"><?php echo $image; ?></a>
		<?php
		}
		return TRUE;

	}//end paintCurrentImage()


}//end class
?>
