<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: page_multiple_page_page_edit_fns.inc,v 1.7 2012/08/30 00:58:09 ewang Exp $
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
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage CMS
*/
class Page_Multiple_Page_Page_Edit_Fns extends Bodycopy_Edit_Fns
{ 		
	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		
		$this->static_screens['tagging'] = Array(
												'name'          => 'Tagging',
												'force_unlock'  => TRUE,
												'lock_type'     => 'menu',
											);

	}//end constructor


}//end class

?>
