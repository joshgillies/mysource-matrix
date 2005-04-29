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
* $Id: tool_search_replace_edit_fns.inc,v 1.2 2005/04/29 05:38:41 gsherwood Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool_edit_fns.inc';

/**
* Tool_Search_Replace_Edit_Fns
*
* Purpose
*	Functions to print and process the backend interface of the system's Tool_Search_Replace
*
* @author Nathan de Vries <ndvries@squiz.net>
* @author David Heppell <dheppell@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Tool_Search_Replace_Edit_Fns extends Tool_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Tool_Search_Replace_Edit_Fns()
	{
		$this->Tool_Edit_Fns();

	}//end constructor


}//end class
