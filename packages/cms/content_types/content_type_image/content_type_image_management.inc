<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: content_type_image_management.inc,v 1.1 2005/12/21 06:02:23 dmckee Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Content_Type_Image_Management
*
* @see Asset_Management
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Image_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Content_Type_Image_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'display_settings'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'on_click'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'none',
												'parameters'	=> Array(
																	'multiple'		=> false,
																	'allow_empty'	=> false,
																	'options'		=> Array(
																						'none'				=> 'No Action',
																						'new_window'		=> 'Open Image in New Window',
																						'current_window'	=> 'Open Image in Current Window',
																						'inline'			=> 'Open Inline Image Preview',
																					   ),
																   ),
											   ),
					  );

	}//end constructor


}//end class

?>