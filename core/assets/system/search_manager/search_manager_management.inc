<?php

/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: search_manager_management.inc,v 1.3 2003/10/15 11:05:15 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Search_Manager_Management
*
* @see Asset_Management
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.3 $ - 0.1
* @package Matrix_Packages
* @subpackage __core__
*/
class Search_Manager_Management extends Asset_Management
{
	/**
	* Constructor
	*/
	function Search_Manager_Management(&$pm)
	{
		$this->Asset_Management($pm);
	}//end constructor

}//end class
?>
