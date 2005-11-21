<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | Squiz.net Commercial Module Licence                                |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: page_multiple_page_page_edit_fns.inc,v 1.2 2005/11/21 04:34:05 dmckee Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/bodycopy/bodycopy/bodycopy_edit_fns.inc';

/**
* Page_Multiple_Page_Page_Edit_Fns
*
* Purpose
*
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage CMS
*/
class Page_Multiple_Page_Page_Edit_Fns extends Bodycopy_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Page_Multiple_Page_Page_Edit_Fns()
	{
		$this->Bodycopy_Edit_Fns();

	}//end constructor


}//end class

?>