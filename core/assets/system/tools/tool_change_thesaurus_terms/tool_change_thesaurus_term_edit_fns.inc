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
* $Id: tool_change_thesaurus_term_edit_fns.inc,v 1.2 2009/06/25 06:57:52 bpearson Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool_edit_fns.inc';

/**
* Tool_Change_Thesaurus_Term_Edit_Fns
*
* Purpose
*	Functions to print and process the backend interface of the system's Tool_Change_Thesaurus_Term
*
* @author Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Tool_Change_Thesaurus_Term_Edit_Fns extends Tool_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Tool_Change_Thesaurus_Term_Edit_Fns()
	{
		$this->Tool_Edit_Fns();

	}//end constructor


}//end class
?>
