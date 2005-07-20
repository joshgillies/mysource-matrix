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
* $Id: form_question_rule.inc,v 1.1 2005/07/20 00:02:42 lwright Exp $
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
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
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
