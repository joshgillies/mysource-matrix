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
* $Id: page_link_manager_edit_fns.inc,v 1.1 2006/02/02 06:49:02 rhoward Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing_edit_fns.inc';

/**
* Page_Link_Manager_Edit_Fns
*
* Purpose
*
*
* @author  David Heppell <dheppell@squiz.net>
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Page_Link_Manager_Edit_Fns extends Page_Asset_Listing_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Page_Link_Manager_Edit_Fns()
	{
		$this->Page_Edit_Fns();

	}//end constructor


}//end class


?>
