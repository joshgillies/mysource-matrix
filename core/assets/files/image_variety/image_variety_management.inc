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
* $Id: image_variety_management.inc,v 1.8 2006/01/18 00:59:32 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Image_Variety_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Image_Variety_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Image_Variety_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'			=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'filename'		=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'Holds the current filename, because when the filename changes we need to know what the old extension was.',
										   ),
						'variety_type'	=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'selection',
											'default'		=> 'resize',
											'parameters'	=> Array(
																'options'		=> Array(
																					'resize'	=> 'Resize Current Image',
																					'upload'	=> 'Upload Alternate Image',
																				   ),
																'multiple'		=> FALSE,
																'allow_empty'	=> FALSE,
															   ),
											'description'	=> '',
										   ),
						'constrain'		=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'selection',
											'default'		=> 'width',
											'parameters'	=> Array(
																'options'		=> Array(
																					'width'		=> 'Width',
																					'height'	=> 'Height',
																					'dimension'	=> 'Both',
																					'none'		=> 'Neither',
																				   ),
																'multiple'		=> FALSE,
																'allow_empty'	=> FALSE,
															   ),
											'description'	=> '',
										   ),
						'width'			=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'int',
											'default'	=> 0,
										   ),
						'height'		=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'int',
											'default'	=> 0,
										   ),
						'dimension'		=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'int',
											'default'	=> 0,
										   ),
					  );

	}//end constructor


}//end class
?>
