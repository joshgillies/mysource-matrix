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
* $Id: content_type_image_management.inc,v 1.3 2006/12/07 00:04:16 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Content_Type_Image_Management
*
* @see Asset_Management
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.3 $
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
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
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