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
* $Id: page_password_reset_edit_fns.inc,v 1.3 2006/12/07 00:04:22 emcdonald Exp $
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
* @version $Revision: 1.3 $
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