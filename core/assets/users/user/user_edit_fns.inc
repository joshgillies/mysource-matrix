<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: user_edit_fns.inc,v 1.7 2003/10/14 05:24:58 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* User_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class User_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function User_Edit_Fns() 
	{
		$this->Asset_Edit_Fns();

		$this->static_screens['details']['lock_type'] = 'attributes';
	}

}//end class
?>
