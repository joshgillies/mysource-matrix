<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: design_area_colourise_image_management.inc,v 1.21 2007/12/06 04:28:48 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Colourise_Image_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.21 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Colourise_Image_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'image'			=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'filename of the Associated File image that we are to colourise',
										   ),
						'colours'		=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'serialise',
											'default'		=> Array(),
											'description'	=> 'Associative Array of colours that are going to be used in the converison process',
										   ),
						'filename_only'	=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'boolean',
											'default'		=> 0,
											'description'	=> 'whether to print just the filename or the full img tag',
											'parameters'	=> Array(
																'allow_empty'	=> FALSE,
															   ),
										   ),
						'type'			=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'selection',
											'default'		=> 'greyscale',
											'parameters'	=> Array(
																'options'		=> Array(
																					'greyscale'			=> 'Greyscale',
																					'multiple_colours'	=> 'Multiple Colours',
																				   ),
																'allow_empty'	=> FALSE,
																'multiple'		=> FALSE,
															   ),
										   ),
						'tolerance'		=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'float',
											'default'	=> 0.2,
										   ),
						'extra'			=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'Any extra HTML that you want to end up in the img tag',
										   ),
					  );

	}//end constructor


}//end class
?>
