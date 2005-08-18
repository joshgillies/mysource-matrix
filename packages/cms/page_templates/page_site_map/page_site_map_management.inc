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
* $Id: page_site_map_management.inc,v 1.11 2005/08/18 02:38:52 dheppell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Site_Map_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.11 $
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
						'name'			=> Array (
											'added'		=> '0.0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'stalks'		=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'boolean',
											'default'		=> true,
											'parameters'	=> Array(
																'allow_empty'	=> false,
															   ),
										   ),
						'top_level'		=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'boolean',
											'default'		=> false,
											'parameters'	=> Array(
																'allow_empty'	=> false,
															   ),
										   ),
						'columns'		=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'int',
											'default'	=> 1,
										   ),
						'levels'		=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'int',
											'default'	=> 0,
										   ),
						'exclude_list'	=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'icons'			=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'boolean',
											'default'		=> false,
											'parameters'	=> Array(
																'allow_empty'	=> false,
															   ),
										   ),
						'show_type_2'	=> Array (
											'added'			=> '0.0.1',
											'type'			=> 'boolean',
											'default'		=> false,
											'parameters'	=> Array(
																'allow_empty'	=> false,
															   ),
										   ),
						'map_format'	=> Array (
											'added'			=> '0.1',
											'type'			=> 'selection',
											'default'		=> 'default',
											'parameters'	=> Array(
																'options'		=> Array(
																					'default'			=> 'Default Format',
																					'unordered_list'	=> 'Unordered List',
																				   ),
																'multiple'		=> false,
																'allow_empty'	=> false,
															   ),
											'description'	=> 'This will determine the format of the Site Map, such as an unordered list',
										   ),
					  );

	}//end constructor


}//end class

?>