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
* $Id: search_log_report.inc,v 1.1 2005/04/13 04:38:46 rhoward Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/log_report/log_report.inc';

/**
* Search_Log_Report
*
* Purpose
*	A report that lists searches by month, week, or day, and by assetid
*
*
* @author Andrei Railean <arailean@squiz.net>
* @author Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Log_Report extends Log_Report
{

	/**
	* Name of Log this report relies on
	*/
	var $logname = 'search';


	/**
	* Constructor
	*
	* @param int    $assetid    the asset id to be loaded
	*
	*/
	function Search_Log_Report($assetid=0)
	{
		$this->_ser_attrs = true;
		parent::Log_Report($assetid);

	}//end constructor


}//end class

?>