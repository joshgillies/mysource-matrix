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
* $Id: form_question_rule_edit_fns.inc,v 1.3 2006/12/06 00:35:26 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Form_Question_Rule_Edit_Fns

* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Rule_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end constructor


	/**
	* Paints the rule's interface
	*
	* @param object		&$asset			The asset to paint
	* @param object		&$o				The backend outputter
	* @param string		$prefix			String prefix for this interface/asset
	* @param array		$rule_data		The rule data
	* @param boolean	$write_access	Whether or not the user has write access
	*
	* @return boolean
	* @access public
	*/
	function paintRule(&$asset, &$o, $prefix, $rule_data, $write_access)
	{

	}//end paintRule()


	/**
	* Process the rule's interface
	*
	* @param object	&$asset		The asset to paint
	* @param object	&$o			The backend outputter
	* @param string	$prefix		String prefix for this interface/asset
	* @param array	&$rule_data	The rule data
	*
	* @return boolean
	* @access public
	*/
	function processRule(&$asset, &$o, $prefix, &$rule_data)
	{

	}//end processRule()


}//end class
?>