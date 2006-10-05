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
* $Id: tool_clear_squid_cache_edit_fns.inc,v 1.1.1.1 2006/10/05 05:37:23 bcaldwell Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool_edit_fns.inc';

/**
* Tool_Clear_Squis_Cache_Edit_Fns
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage squid
*/
class Tool_Clear_Squid_Cache_Edit_Fns extends Tool_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Tool_Clear_Squid_Cache_Edit_Fns()
	{
		$this->Tool_Edit_Fns();

	}//end constructor


}//end class

?>