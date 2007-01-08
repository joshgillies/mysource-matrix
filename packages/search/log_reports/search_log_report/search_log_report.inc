<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: search_log_report.inc,v 1.5 2007/01/08 04:00:57 emcdonald Exp $
*
*/

check_ssv_licence();
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
* @version $Revision: 1.5 $
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
		$this->_ser_attrs = TRUE;
		parent::Log_Report($assetid);

	}//end constructor


}//end class

?>