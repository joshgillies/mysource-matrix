<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: design_area_custom_image.inc,v 1.6 2003/09/26 05:26:24 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Custom_Image
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Custom_Image extends Design_Area
{

	/**
	* All allowed extensions for an image
	* @var Array(string)
	*/
	var $allowed_extensions = Array('gif', 'jpg', 'jpeg', 'png');


	/*
	* Outputs either the straight HTML or PHP code that is needed for this design area to be represented
	*
	* @param object Design	$design the design for which we are currently printing
	*
	* @return boolean
	* @access public
	*/
	function printArea(&$design) 
	{
		$image_path = $this->data_path_public.'/'.$this->attr('image');
		// return true because it just means that they haven't uploaded the file yet
		if (!file_exists($image_path)) return true;

		$image = '<'.'?php echo $DATA_PATH; ?'.'>/'.$this->type().'/'.$this->id.'/'.$this->attr('image');
		// if they only want us to echo out the filename
		if ($this->attr('filename_only')) {
			echo  $image;

		// else print out a full tag
		} else {
			$image_size = @getimagesize($image_path);
			if ($alt = $this->attr('alt')) $alt = ' alt="'.htmlspecialchars($alt).'"';
			echo '<img src="', $image, '" ', $this->attr('extra'), $alt, (($image_size) ? $image_size[3] : ''), '>';
		}

		return true;

	}//end printArea()

}//end class

?>
