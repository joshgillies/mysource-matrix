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
* $Id: design_area_colourise_image_edit_fns.inc,v 1.12 2003/12/12 17:03:12 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Colourise_Image_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Colourise_Image_Edit_Fns extends Design_Area_Edit_Fns
{

	/**
	* Loops through the contents array and creates the necessary objects
	*
	* @param object Asset	$asset		the asset to which we belong
	* @param array			$contents	the contents array we generated with _parseString()
	*
	* @return boolean
	* @access private
	*/
	function _processContents(&$asset, $contents)
	{
		// blank out the colours array so that we don't end up remembering old values 
		$asset->setAttrValue('colours', Array());
		return parent::_processContents($asset, $contents);

	}// end _processContents()


	/**
	* Creates the colourised image and returns the image name
	*
	* @param object Asset	$asset		the asset to which we belong
	* @param object Design	$design the design for which we are currently printing
	*
	* @return string
	* @access private
	*/
	function createImage(&$asset, &$design)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		require_once SQ_FUDGE_PATH.'/image/image.inc';

		$image_asset = $design->getAssociatedFile($asset->attr('image'));
		if (is_null($image_asset)) {
			trigger_error('Unable to colourise "'.$asset->attr('id_name').'", source file asset for "'.$asset->attr('image').'" not found', E_USER_WARNING);
			return '';
		}

		$source = $image_asset->data_path.'/'.$image_asset->attr('name');
		// check the source file exists
		if (!file_exists($source)) {
			trigger_error('Unable to colourise "'.$asset->attr('id_name').'", source file "'.$source.'" doesn\'t exist', E_USER_WARNING);
			return '';
		}

		// dest file has design id so that if we are used by customisations they each have there own file
		$dest = $asset->data_path_public.'/colourised_'.$design->id.'_'.$asset->attr('image');

		if (!create_directory($asset->data_path_public)) {
			trigger_error('Unable to colourise "'.$asset->attr('id_name').'", unable to create destination directory "'.$asset->data_path_public.'"', E_USER_WARNING);
			return '';
		}

		$colours = $asset->attr('colours');

		switch ($asset->attr('type')) {
		
			case 'greyscale' :
				if (!isset($colours['black_colour']) || !isset($colours['white_colour'])) { 
					trigger_error('Unable to colourise "'.$asset->attr('id_name').'", you need to set both the black_colour and white_colour attributes in order to greyscale colourise an image', E_USER_WARNING);
					return '';
				}
				if (Image::gradientPalette($source, $dest, $colours['black_colour'], $colours['white_colour'])) {
					return basename($dest);
				}

			break;

			case 'multiple_colours' :

				$colour_map = Array();
				$i = 1;
				while(isset($colours['from_colour_'.$i]) && isset($colours['to_colour_'.$i])) {
					$colour_map[$colours['from_colour_'.$i]] = $colours['to_colour_'.$i];
					$i++;
				}

				if (Image::remapColour($source, $dest, $colour_map, $asset->attr('tolerance'))) {
					return basename($dest);
				}

			break;

			default : 
				trigger_error('Unable to Colourise Image "'.$asset->attr('id_name').'",  type "'.$this->get_val('type').'" unknown.', E_USER_WARNING);

		}//end switch

		return '';

	}//end createImage()


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
		// remove the colours array attribute
		unset($vars['colours']);

		// Add the emulated black_colour, white_colour, to_colour_1, from_colour_2, ... vars into the vars array
#		pre_echo($asset->vars['colours']['value']);
		foreach($asset->vars['colours']['value'] as $name => $value) {
			$vars[$name] = Array('value' => $value);
		}
		foreach($asset->vars['var_references']['value'] as $name => $data) {
			if (strstr($name, '_colour')) $vars[$name] = $data;
		}
		return $vars;
	}// end _getSetableVars()

}//end class
?>
