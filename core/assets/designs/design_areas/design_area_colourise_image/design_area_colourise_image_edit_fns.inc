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
