<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: page_calendar_management.inc,v 1.2 2004/07/30 06:35:12 lwright Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Calendar_Management
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.2 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Calendar_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param object Package_Manager	&$pm	Reference to the package manager object for this package
	*
	*/
	function Page_Calendar_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['root_nodes']			= Array(
														'added'     => '0.1',
														'type'      => 'serialise',
														'default'   => Array(),
														);
		
	}//end Page_Calendar_Management()


}//end class

?>