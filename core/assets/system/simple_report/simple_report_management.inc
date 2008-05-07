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
* $Id: simple_report_management.inc,v 1.1 2008/05/07 05:54:04 mbrydon Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Simple Report Management
*
* @see Asset_Management
*
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Simple_Report_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
 		Asset_Management::__construct($pm);

	}//end constructor


}//end class

?>
