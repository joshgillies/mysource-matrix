<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: page_site_map_management.inc,v 1.1 2003/10/07 04:01:07 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Site_Map_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
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
							'name' => Array (
									'added'		=> '0.0.1',
									'type'		=> 'text',
									'default'	=> '',
							), // end name
							'stalks' => Array(
									'added'		=> '0.0.1',
									'type'		=> 'boolean',
									'default'	=> true,
							), // end stalks
							'top_level'	=> Array(
									'added'		=> '0.0.1',
									'type'		=> 'boolean',
									'default'	=> false,
							), // end top_level
							'columns' => Array(
									'added'		=> '0.0.1',
									'type'		=> 'int',
									'default'	=> 1,
							), // end levels
							'levels' => Array(
									'added'		=> '0.0.1',
									'type'		=> 'int',
									'default'	=> 0,
							),
							'exclude_list'	=>	Array(
									'added'		=> '0.0.1',
									'type'		=> 'serialise',
									'default'	=> Array(),
							),
						);

	}//end constructor

}//end class

?>