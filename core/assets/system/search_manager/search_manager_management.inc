<?php

/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: search_manager_management.inc,v 1.2 2003/10/08 02:12:04 dwong Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Search_Manager_Management
*
* @see Asset_Management
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.2 $ - 0.1
* @package Matrix_Packages
* @subpackage __core__
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

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
