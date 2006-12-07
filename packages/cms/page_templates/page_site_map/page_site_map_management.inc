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
* $Id: page_site_map_management.inc,v 1.16 2006/12/07 00:04:23 emcdonald Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Site_Map_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.16 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Site_Map_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Page_Site_Map_Management(&$pm)
	{
		$this->Asset_Management($pm);


		$this->vars = Array(
						'name'					=> Array (
													'added'		=> '0.0.1',
													'type'		=> 'text',
													'default'	=> '',
												   ),
						'stalks'				=> Array(
													'added'			=> '0.0.1',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'top_level'				=> Array(
													'added'			=> '0.0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'columns'				=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'int',
													'default'	=> 1,
												   ),
						'levels'				=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'int',
													'default'	=> 0,
												   ),
						'exclude_list'			=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
												   ),
						'include_types'			=> Array(
													'added'		=> '0.2',
													'type'		=> 'serialise',
													'default'	=> Array(),
												   ),
						'include_dependants'	=> Array(
													'added'			=> '0.3',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'icons'					=> Array(
													'added'			=> '0.0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'show_type_2'			=> Array (
													'added'			=> '0.0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'map_format'			=> Array (
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> 'default',
													'parameters'	=> Array(
																		'options'		=> Array(
																							'default'			=> 'Default Format',
																							'unordered_list'	=> 'Unordered List',
																						   ),
																		'multiple'		=> FALSE,
																		'allow_empty'	=> FALSE,
																	   ),
													'description'	=> 'This will determine the format of the Site Map, such as an unordered list',
												   ),
						'display_formats'		=> Array(
													'added'		=> '0.2',
													'type'		=> 'serialise',
													'default'	=> Array(),
												   ),
					  );

	}//end constructor


}//end class

?>