<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: design_area_declared_vars_management.inc,v 1.7 2003/09/26 05:26:24 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Declared_Vars_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Declared_Vars_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Design_Area_Declared_Vars_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'declared_vars'   => Array(
								'added'       => '0.0.1',
								'type'        => 'serialise',
								'default'     => Array(),
								'description' => 'Holds the names for all variables that are "references" to values in other'
							),
						);

	}//end constructor

}//end class
?>
