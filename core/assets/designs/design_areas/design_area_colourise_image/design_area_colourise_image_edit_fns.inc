<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Colourise_Image_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Design_Area_Colourise_Image_Edit_Fns extends Design_Area_Edit_Fns
{

	/**
	* Creates the colourised image and returns the image name
	*
	* @param object Asset	$asset		the asset to which we belong
	* @param object Design	$design the design for which we are currently printing
	*
	* @return string
	* @access private
	*/
	function createImage(&$asset, &$design) {
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		require_once SQ_FUDGE_PATH.'/image/image.inc';

		$source = $design->filePath($asset->attr('image'));
		$dest   = $asset->data_path.'/colourised_'.$design->id.'_'.$asset->attr('image');

		create_directory($asset->data_path);

		// check the source file exists
		if (!file_exists($source)) {
			trigger_error('Unable to colourise "'.$asset->attr('id_name').'", source file "'.$source.'" doesn\'t exist', E_USER_WARNING);
			return '';
		}

		switch ($asset->attr('type')) {
		
			case 'greyscale' :
				if (Image::gradientPalette($source, $dest, $asset->attr('black_colour'), $asset->attr('white_colour'))) {
					return basename($dest);
				}

			break;

			case 'multiple_colours' :

				$colours = Array();
				$i = 1;
				while(@$asset->attr('from_colour_'.$i) && @$asset->attr('to_colour_'.$i)) {
					$colours[$asset->attr('from_colour_'.$i)] = $asset->attr('to_colour_'.$i);
					$i++;
				}

				if (Image::remapColour($source, $dest, $colours, $asset->attr('tolerance'))) {
					return basename($dest);
				}

			break;

			default : 
				trigger_error('Unable to Colourise Image "'.$asset->attr('id_name').'",  type "'.$this->get_val('type').'" unknown.', E_USER_WARNING);

		}//end switch

		return '';

	}//end createImage()

}//end class
?>
