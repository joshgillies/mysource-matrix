<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Colourise_Image_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Design_Area_Colourise_Image_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Design_Area_Colourise_Image_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'image' => Array(
								'added'     => '0.0.1',
								'type'      => 'text',
								'default'   => '',
							),
							'colours' => Array(
								'added'       => '0.0.1',
								'type'        => 'serialise',
								'default'     => Array(),
								'description' => 'Associative Array of colours that are going to be used in the converison process',
							),
							'filename_only' => Array(
								'added'     => '0.0.1',
								'type'      => 'boolean',
								'default'   => 0,
							),
							'type' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'selection',
								'default'		=> 'greyscale',
								'parameters'	=> Array(
									'options' => Array(
										'greyscale'			=> 'Greyscale', 
										'multiple_colours'	=> 'Multiple Colours',
									),
								),
							),
							'tolerance' => Array(
								'added'     => '0.0.1',
								'type'      => 'float',
								'default'   => 0.2,
							),
							'extra' => Array(
								'added'     => '0.0.1',
								'type'      => 'text',
								'default'   => '',
							),
						);

	}//end constructor

}//end class
?>
