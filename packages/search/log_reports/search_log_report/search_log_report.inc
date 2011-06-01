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
* $Id: search_log_report.inc,v 1.7 2008/11/25 05:03:13 mbrydon Exp $
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
* @version $Revision: 1.7 $
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
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


}//end class

?>