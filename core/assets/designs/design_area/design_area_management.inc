<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: design_area_management.inc,v 1.8 2003/09/26 05:26:24 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Design_Area_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('id_name' => Array(
								'added'     => '0.0.1',
								'type'      => 'text',
								'default'   => ''
								),
							'contents' => Array(
								'added'     => '0.0.1',
								'type'      => 'serialise',
								'default'   => Array(),
								'description' => 'Holds the processed contents array'
								),
							'var_references' => Array(
								'added'     => '0.0.1',
								'type'      => 'serialise',
								'default'   => Array(),
								'description' => 'Holds the names for all variables that are "references" to values in other'
								),
							'specialised' => Array(
								'added'     => '0.0.1',
								'type'      => 'boolean',
								'default'   => 0
								)
							);

	}//end constructor

}//end class
?>
