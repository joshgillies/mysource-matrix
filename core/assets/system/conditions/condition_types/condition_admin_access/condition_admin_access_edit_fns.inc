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
* $Id: condition_admin_access_edit_fns.inc,v 1.8 2006/12/06 04:23:57 emcdonald Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition_edit_fns.inc';

/**
* Condition_Write_Access_Edit_Fns
*
* Edit functions for the admin access conditional
*
* @see Asset_Edit_Fns
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_Admin_Access_Edit_Fns extends Condition_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::Asset_Edit_Fns
	*/
	function Condition_Admin_Access_Edit_Fns()
	{
		$this->Condition_Edit_Fns();

	}//end constructor


	/**
	* Paint the backend editing interface for this condition
	*
	* @param array		&$condition_data	Array of settings for this condition
	* @param object		&$o					Backend outputter to use
	* @param string		$prefix				Prefix for form elements
	* @param boolean	$write_access		Whether we have access to edit the field values
	*
	* @return void
	* @access public
	*/
	function paintEditInterface(&$condition_data, &$o, $prefix, $write_access)
	{

	}//end paintEditInterface()


	/**
	* Process the backend editing interface for this condition
	*
	* @param object	&$o		Backend outputter to use
	* @param string	$prefix	Prefix for form elements
	*
	* @return array
	* @access public
	*/
	function processEditInterface(&$o, $prefix)
	{

	}//end processEditInterface()


}//end class

?>