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
* $Id: design_area_constant_button_management.inc,v 1.15 2007/12/06 04:28:48 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Constant_Button_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.15 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Constant_Button_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'assetid'			=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'int',
												'default'		=> 0,
												'description'	=> 'The asset that the button is to link to (has priority over the url attribute)',
											   ),
						'url'				=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'The URL that the button is to link to',
											   ),
						'new_window'		=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'description'	=> 'Whether to open in a new window or not',
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'new_window_width'	=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'int',
												'default'		=> 0,
												'description'	=> 'Width of the new window to be created',
											   ),
						'new_window_height'	=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'int',
												'default'		=> 0,
												'description'	=> 'Width of the new window to be created',
											   ),
					  );

	}//end constructor


}//end class
?>
