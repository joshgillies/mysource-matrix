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
* $Id: image_variety_management.inc,v 1.10.8.1 2013/07/31 04:35:21 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Image_Variety_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.10.8.1 $
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
																					'centre_weighted_square' => 'Square (Centre Weighted)',
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
