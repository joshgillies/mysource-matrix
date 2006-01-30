<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: search_log_report.inc,v 1.3 2006/01/30 04:34:33 sdanis Exp $
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
* @version $Revision: 1.3 $
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