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
* $Id: tool_clear_matrix_cache_management.inc,v 1.1 2007/05/31 04:53:34 rong Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Tool_Clear_Matrix_Cache_Management
*
* @author  Rayn Ong <rong@squiz.net>
* @version $Revision: 1.1 $
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
