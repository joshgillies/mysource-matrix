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
* $Id: tool_asset_sorter_edit_fns.inc,v 1.2 2006/12/06 00:13:03 bcaldwell Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool_edit_fns.inc';

/**
* Tool_Asset_Sorter_Edit_Fns
*
* Purpose
*	Functions to print and process the backend interface of the system's Tool_Asset_Sorter
*
* @author Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Tool_Asset_Sorter_Edit_Fns extends Tool_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Tool_Asset_Sorter_Edit_Fns()
	{
		$this->Tool_Edit_Fns();

	}//end constructor


}//end class
?>