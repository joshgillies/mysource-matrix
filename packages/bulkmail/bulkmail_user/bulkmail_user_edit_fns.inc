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
* $Id: bulkmail_user_edit_fns.inc,v 1.4 2005/11/28 22:42:01 rong Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Bulkmail_User_Edit_Fns
*
* Purpose
*
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_User_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Bulkmail_User_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end constructor


}//end class
?>