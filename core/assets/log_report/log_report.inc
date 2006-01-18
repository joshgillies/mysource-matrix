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
* $Id: log_report.inc,v 1.2 2006/01/18 01:22:50 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/report/report.inc';

/**
* Log_Report
*
* Purpose
*   A varient of Report that uses logs taken from the log_manager
*
*
* @author Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Log_Report extends Report
{


	/**
	* Name of Log this report relies on
	* Other reports should overwrite this
	*/
	var $logname = NULL;


	/**
	* Requests log data from log manager and processes it into the report, saving the state
	*
	* Returns FALSE if regeneration failed, TRUE otherwise
	*
	* @return boolean
	* @access private
	*/
	function _regenerateReport()
	{
		return FALSE;

	}//end _regenerateReport()


}//end class

?>