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
* $Id: form_question_rule.inc,v 1.14.2.2 2005/05/02 02:44:57 gsherwood Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Form_Question_Rule
*
* Purpose
*
*
* @static
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.14.2.2 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Rule extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question_Rule($assetid=0)
	{
		$this->Asset($assetid);

	}//end constructor


	/**
	* return true if this rule can be used for form validation
	*
	* @return boolean
	* @access public
	*/
	function isValidationRule()
	{
		return true;

	}//end isValidationRule()


	/**
	* return true if this rule can be used for processing selective emails
	*
	* @return boolean
	* @access public
	*/
	function isSelectiveEmailRule()
	{
		return true;

	}//end isSelectiveEmailRule()


	/**
	* Returns a list of operators for this rule type
	*
	* @access public
	* @return array (mixed => string) an array of descriptions indexed by operator id (which can be
	*                                 anything, generally string or (int)0|1 for a boolean value)
	*/
	function getOperators()
	{
		return Array();

	}//end getOperators()


	/**
	* @abstract
	*/
	function evaluate($answer, $rule_data)
	{
		return true;

	}//end evaluate()


	/**
	* @abstract
	*/
	function generateJSCode(&$q_asset, $rule_data)
	{
		return '';

	}//end generateJSCode()


	/**
	* @abstract
	*/
	function defaultError(&$q_asset, $rule_data)
	{
		return $this->ruleDescription($q_asset, $rule_data);

	}//end defaultError()


	/**
	* @abstract
	*/
	function ruleDescription(&$q_asset, $rule_data)
	{
		return '';

	}//end ruleDescription()


}//end class

?>
