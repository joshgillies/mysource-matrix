<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: log_report.inc,v 1.3 2006/12/06 01:07:34 emcdonald Exp $
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
* @version $Revision: 1.3 $
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