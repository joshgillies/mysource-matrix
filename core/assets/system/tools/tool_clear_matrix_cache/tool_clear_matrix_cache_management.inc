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
* $Id: tool_clear_matrix_cache_management.inc,v 1.2 2012/08/30 01:09:12 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Tool_Clear_Matrix_Cache_Management
*
* @author  Rayn Ong <rong@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Tool_Clear_Matrix_Cache_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Tool_Clear_Matrix_Cache_Management(&$pm)
	{
		$this->Asset_Management($pm);


	}//end constructor


}//end class

?>
