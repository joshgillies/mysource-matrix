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
* $Id: suite_manager_management.inc,v 1.2 2012/08/30 00:59:25 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Suite_Manager_Management
*
* @see Asset_Management
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>
* @version $Revision: 1.2 $ - 0.1
* @package MySource_Matrix_Packages
* @subpackage squiz_suite
*/
class Suite_Manager_Management extends Asset_Management
{


	/**
	* Constructor
	*/
	function Suite_Manager_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array();

	}//end constructor


}//end class
?>
