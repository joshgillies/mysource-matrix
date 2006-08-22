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
* $Id: page_password_reset_edit_fns.inc,v 1.2 2006/08/22 23:15:34 skim Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Page_Password_Reset_Edit_Fns
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Password_Reset_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Page_Password_Reset_Edit_Fns()
	{
		$this->Page_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = 0;

	}//end constructor


}//end class

?>