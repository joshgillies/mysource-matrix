<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: site_management.inc,v 1.8 2003/10/22 05:10:35 dwong Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Site_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Site_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Site_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->components['name']['default_weighting'] = '1';

	}//end constructor

}//end class
?>
