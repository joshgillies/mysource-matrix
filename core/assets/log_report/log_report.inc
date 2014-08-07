<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: log_report.inc,v 1.5 2012/08/30 01:09:08 ewang Exp $
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
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Log_Report extends Report
{


	/**
	* Name of Log this report relies on
	* Other reports should overwrite this
	*/
	public $logname = NULL;


	/**
	* Constructor
	*
	* @param string	$assetid	The asset ID of an asset to load
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Requests log data from log manager and processes it into the report, saving the state
	*
	* Returns FALSE if regeneration failed, TRUE otherwise
	*
	* @return boolean
	* @access private
	*/
	protected function _regenerateReport()
	{
		return FALSE;

	}//end _regenerateReport()


}//end class

?>