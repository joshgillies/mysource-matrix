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
* $Id: design_area_custom_image.inc,v 1.17 2012/08/30 01:09:06 ewang Exp $
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
* @version $Revision: 1.17 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Custom_Image extends Design_Area
{


	/**
	* All allowed extensions for an image
	* @var Array(string)
	*/
	public $allowed_extensions = Array('gif', 'jpg', 'jpeg', 'png');


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
	* Print code that actually results in HTML output to the browser for this design area
	*
	* This function is called at the AREA tag if print != no, and at the point of any PRINT
	* tags that reference this area.
	*
	* @return void
	* @access protected
	*/
	protected function _printPaintingCode()
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