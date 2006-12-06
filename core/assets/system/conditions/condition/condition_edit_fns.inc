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
* $Id: condition_edit_fns.inc,v 1.6 2006/12/06 04:23:57 emcdonald Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Condition_Edit_Fns
*
* Edit functions for the conditional parent
*
* @see Asset_Edit_Fns
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::Asset_Edit_Fns
	*/
	function Condition_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

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
		return Array();

	}//end processEditInterface()


}//end class
?>