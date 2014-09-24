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
* $Id: design_area_colourise_image.inc,v 1.26 2012/08/30 01:09:06 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Colourise_Image
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.26 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Colourise_Image extends Design_Area
{


	/**
	* Constructor
	*
	* @param string	$assetid	The asset ID to load
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	public function setAttrValue($name, $value)
	{
		// if this is one of the colour attributes then let's grab the value, add it the
		// $this->colours and then save that array
		if ($name == 'black_colour' || $name == 'white_colour'
			|| stristr($name, 'from_colour') || stristr($name, 'to_colour')) {
			$new_value = $this->vars['colours']['value'];
			if (!is_array($new_value)) $new_value = Array();
			$new_value[$name] = $value;
			$name = 'colours';
			$value = $new_value;
		}

		if ($name == 'type' && $value != 'multiple_colours') {
			$value = 'greyscale';
		}

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Easy way to get the value of an attribute
	*
	* @param string	$name	Name of the attribute to get
	*
	* @return mixed array|string
	* @access public
	*/
	public function attr($name)
	{
		if (empty($this->vars['colours']['value'][$name])) {
			return parent::attr($name);
		} else {
			return $this->vars['colours']['value'][$name];
		}

	}//end attr()


	/**
	* Print code that actually results in HTML output to the browser for this design area
	*
	* @return void
	* @access protected
	*/
	protected function _printPaintingCode()
	{
		if (!isset($this->_tmp['image'])) {
			trigger_error('no image');
			return;
		}
		$image = 'assets/'.$this->type().'/'.$this->id.'/'.$this->_tmp['image'];

		if ($this->attr('filename_only')) {
			// they only want us to echo out the filename
			echo '<', '?php echo $DATA_PATH; ?>/', $image;
		} else {
			// print out a full tag
			echo '<img src="<', '?php echo $DATA_PATH; ?', '>/', $image, '" ', $this->attr('extra'), ' />';
		}

	}//end _printPaintingCode()


	/**
	* Return whether this DA has been given sufficient details to be able to print stuff
	*
	* This function can be overriden by children to enforce required attributes etc
	*
	* @return boolean
	* @access protected
	*/
	protected function _canInit()
	{
		$image_asset = $GLOBALS['DESIGN_BEING_GENERATED']->getAssociatedFile($this->attr('image'));
		if (is_null($image_asset)) {
			trigger_localised_error('CORE0184', sprintf(translate('Unable to colourise "%1$s", source file asset for "%2$s" not found'), $this->attr('id_name'), $this->attr('image')), E_USER_WARNING);
			return FALSE;
		}

		// check the source file exists
		$source = $image_asset->data_path.'/'.$image_asset->attr('name');
		if (!file_exists($source)) {
			trigger_localised_error('CORE0183', sprintf(translate('Unable to colourise "%1$s", source file "%2$s" doesn\'t exist'), $this->attr('id_name'), $source), E_USER_WARNING);
			return FALSE;
		}

		if ($this->attr('type') == 'greyscale') {
			$colours = $this->attr('colours');
			if (!isset($colours['black_colour']) || !isset($colours['white_colour'])) {
				trigger_localised_error('CORE0186', sprintf(translate('Unable to colourise "%s", you need to set both the black_colour and white_colour attributes in order to greyscale colourise an image'), $this->attr('id_name')), E_USER_WARNING);
				return FALSE;
			}
		}

		return TRUE;

	}//end _canInit()


	/**
	* Print code that initialises this design area and should always be printed in place of the AREA tag
	*
	* This version doesn't actually print any code but it prepares the colourised image for future use
	*
	* @return void
	* @access protected
	*/
	protected function _printInitCode()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		require_once SQ_FUDGE_PATH.'/image/image_manip.inc';

		$image_asset = $GLOBALS['DESIGN_BEING_GENERATED']->getAssociatedFile($this->attr('image'));
		$source = $image_asset->data_path.'/'.$image_asset->attr('name');

		// dest file has design id so that if we are used by customisations they each have their own file
		$dest = $this->data_path_public.'/colourised_'.$GLOBALS['DESIGN_BEING_GENERATED']->id.'_'.$this->attr('image');

		if (!create_directory($this->data_path_public)) {
			trigger_localised_error('CORE0185', sprintf(translate('Unable to colourise "%1$s", unable to create destination directory "%2$s"'), $this->attr('id_name'), $this->data_path_public), E_USER_WARNING);
			return;
		}

		$colours = $this->attr('colours');

		switch ($this->attr('type')) {

			case 'greyscale' :
				if (Image_Manip::gradientPalette($source, $dest, $colours['black_colour'], $colours['white_colour'])) {
					$this->_tmp['image'] = basename($dest);
				}
			break;

			case 'multiple_colours' :
				$colour_map = Array();
				$i = 1;
				while (isset($colours['from_colour_'.$i]) && isset($colours['to_colour_'.$i])) {
					$colour_map[$colours['from_colour_'.$i]] = $colours['to_colour_'.$i];
					$i++;
				}
				if (Image_Manip::remapColour($source, $dest, $colour_map, $this->attr('tolerance'))) {
					$this->_tmp['image'] = basename($dest);
				}
			break;

			default :
				trigger_localised_error('CORE0182', sprintf(translate('Unable to Colourise Image "%1$s",  type "%2$s" unknown.'), $this->attr('id_name'), $this->get_val('type')), E_USER_WARNING);

		}//end switch

	}//end _printInitCode()


	/**
	* Get the names of attributes for this DA that cannot be set in the parse file
	*
	* Design Area attributes that are not mentioned in the parse file are normally set to defaults
	* when parsing the design.  Attributes returned by this function are exempted from that process.
	*
	* @return array
	* @access public
	*/
	public function getProtectedAttrs()
	{
		$res = parent::getProtectedAttrs();
		$res[] = 'colours';
		return $res;

	}//end getProtectedAttrs()


}//end class
?>
