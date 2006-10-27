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
* $Id: design_area_custom_image.inc,v 1.14 2006/10/27 04:25:42 tbarrett Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Custom_Image
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.14 $
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


	/**
	* Print code that actually results in HTML output to the browser for this design area
	*
	* This function is called at the AREA tag if print != no, and at the point of any PRINT
	* tags that reference this area.
	*
	* @return void
	* @access protected
	*/
	function _printPaintingCode()
	{
		$image_path = $this->data_path_public.'/'.$this->attr('image');
		// return true because it just means that they haven't uploaded the file yet
		if (!file_exists($image_path)) return;

		$image = '<'.'?php echo $DATA_PATH; ?'.'>/'.$this->data_path_suffix.'/'.$this->attr('image');
		// if they only want us to echo out the filename
		if ($this->attr('filename_only')) {
			echo  $image;

		// else print out a full tag
		} else {
			$image_size = @getimagesize($image_path);
			if ($alt = $this->attr('alt')) {
				$alt = htmlspecialchars($alt);
			}
			echo '<img src="', $image, '" ', $this->attr('extra');
			if ($alt) echo ' alt="', $alt, '"';
			echo (($image_size) ? ' '.$image_size[3] : ''), ' />';
		}

	}//end _printPaintingCode()


}//end class
?>