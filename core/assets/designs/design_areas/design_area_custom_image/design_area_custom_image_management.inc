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
* $Id: design_area_custom_image_management.inc,v 1.12 2007/12/06 04:28:48 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Custom_Image_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Custom_Image_Management extends Asset_Management
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
											'added'		=> '0.0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'max_width'		=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'int',
											'default'		=> 0,
											'description'	=> 'The maximum width that the uploaded image can be',
										   ),
						'max_height'	=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'int',
											'default'		=> 0,
											'description'	=> 'The maximum height that the uploaded image can be',
										   ),
						'alt'			=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'The text that you want to appear in the alt attribute for this image',
										   ),
						'extra'			=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'Any extra HTML that you want to end up in the img tag',
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
					  );

	}//end constructor


}//end class
?>
