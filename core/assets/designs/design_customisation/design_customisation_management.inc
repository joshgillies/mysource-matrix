<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: design_customisation_management.inc,v 1.4 2003/09/26 05:26:25 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Customisation_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Design_Customisation_Management(&$pm)
	{
		$this->Asset_Management($pm);
	}//end constructor

}//end class
?>
