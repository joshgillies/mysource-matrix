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
* $Id: design_area_custom_image.inc,v 1.8.2.1 2004/04/21 10:55:16 brobertson Exp $
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

		$image = '<'.'?php echo $DATA_PATH; ?'.'>/assets/'.$this->type().'/'.$this->id.'/'.$this->attr('image');
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
