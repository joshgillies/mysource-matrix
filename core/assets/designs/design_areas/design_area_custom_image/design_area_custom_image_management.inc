<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Custom_Image_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Design_Area_Custom_Image_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Design_Area_Custom_Image_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'image' => Array(
								'added'     => '0.0.1',
								'type'      => 'text',
								'default'   => '',
							),
							'max_width' => Array(
								'added'     => '0.0.1',
								'type'      => 'int',
								'default'   => 0,
								'description' => 'The maximum width that the uploaded image can be',
							),
							'max_height' => Array(
								'added'     => '0.0.1',
								'type'      => 'int',
								'default'   => 0,
								'description' => 'The maximum height that the uploaded image can be',
							),
							'alt' => Array(
								'added'       => '0.0.1',
								'type'        => 'text',
								'default'     => '',
								'description' => 'The text that you want to appear in the alt attribute for this image'
							),
							'extra' => Array(
								'added'     => '0.0.1',
								'type'      => 'text',
								'default'   => ''
							),
							'filename_only' => Array(
								'added'     => '0.0.1',
								'type'      => 'boolean',
								'default'   => 0
							),
						);

	}//end constructor

}//end class
?>
