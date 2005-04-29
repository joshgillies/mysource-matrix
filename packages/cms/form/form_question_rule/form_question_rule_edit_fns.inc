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
* $Id: form_question_rule_edit_fns.inc,v 1.15 2005/04/29 05:40:29 gsherwood Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Form_Question_Rule_Edit_Fns

* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.15 $
* @package MySource_Matrix_Packages
* @subpackage cms
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
	* @return boolean
	* @access public
	*/
	function paintRule(&$asset, &$o, $prefix, $rule_data, $write_access)
	{
	}//end paintRule()


	/**
	* Process the rule's interface
	*
	* @return boolean
	* @access public
	*/
	function processRule(&$asset, &$o, $prefix, &$rule_data)
	{
	}//end processRule()


}//end class
?>