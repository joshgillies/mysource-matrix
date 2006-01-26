<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: web_folder_edit_fns.inc,v 1.2 2006/01/26 22:34:07 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/site/site_edit_fns.inc';

/**
* Web_Folder_Edit_Fns
*
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Web_Folder_Edit_Fns extends Site_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Web_Folder_Edit_Fns()
	{
		$this->Site_Edit_Fns();
		unset($this->static_screens['preview']);

	}//end constructor


}//end class

?>