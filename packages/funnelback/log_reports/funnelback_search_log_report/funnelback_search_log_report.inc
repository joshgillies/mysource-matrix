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
* $Id: funnelback_search_log_report.inc,v 1.2 2012/08/30 00:58:43 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/report/report.inc';

/**
* Funnelback_Search_Log_Report
*
* Purpose
*	A report that lists searches by month, week, or day, and by url?
*
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>	
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage funnelback
*/
class Funnelback_Search_Log_Report extends Report
{

	/**
	 * Name of the report this log relies on
	 */
	var $logname;


	/**
	* Constructor
	*
	* @param int    $assetid    the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->logname = 'funnelback';
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


}//end class

?>
