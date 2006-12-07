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
* $Id: page_multiple_page_page_edit_fns.inc,v 1.3 2006/12/07 00:04:21 emcdonald Exp $
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
* @version $Revision: 1.3 $
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