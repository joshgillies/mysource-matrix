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
* $Id: form_question_rule_type_file_size.inc,v 1.2 2004/12/06 14:41:39 brobertson Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';

/**
* Form_Question_Rule_Type_File_Size
*
* Purpose
*
*
* @static
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Rule_Type_File_Size extends Form_Question_Rule
{


	/**
	* Returns an array of definition variables in an array
	*
	* Since we are not instantiating the question rules, we do not have access to member variables
	* (at least not until PHP5). So this overrides the usual attribute gathering function.
	* Those methods wishing to override this function should call its parent first then modify the
	* returned array.
	*
	* @access public
	* @return array (string => mixed) an array of parameter values indexed by a parameter id
	*/
	function getAttributes()
	{
		$attrs = parent::getAttributes();
		$attrs['show_asset_finder'] = false;
		$attrs['selective_email_rule'] = false;
		return $attrs;

	}//end getAttributes()


	/**
	* Returns a list of operators for this rule type
	*
	* @access public
	* @return array (mixed => string) an array of descriptions indexed by operator id (which can be
	*                                 anything, generally string or (int)0|1 for a boolean value)
	*/
	function getOperators()
	{
		$operators = Array(
			'<=' => 'Must be no larger than ',
			'>=' => 'Must be no smaller than ',
		);
		return $operators;

	}//end getOperators()


	/**
	* processes the form question and returns true if it is valid
	*
	* Because the Form Question Rule classes will not be instantiable by Matrix NOR will be
	* instantiated manually in code, this function MUST called statically.
	*
	* @param object Form_Question	&$q_asset		reference to the question asset
	* @param string					$operator		the operator being used to compare
	* @param mixed					&$comparison	either a value or a form_question asset (based on
	*												comparison question id), being used to compare
	*
	* @see Form_Question_Rule::processForm()
	* @static
	* @access public
	* @return boolean
	*/
	function processForm(&$q_asset, $operator, &$comparison)
	{
		if (!$q_asset->active_submission) return false;
		$answer = $_FILES[$q_asset->id];

		$operators = eval('return '.__CLASS__.'::getOperators();');

		$filesize = $answer['size'];
		$comparison = strtolower(trim($comparison));
		$comp_value = floatval($comparison);

		// allow users to specify abbreviated forms of file size (kB, MB, GB)
		// the conversion used is 1024 to the kByte, rather than the decimal 1000
		if ((substr($comparison,-2) == 'kb') || (substr($comparison,-1) == 'k')) {
			$comp_value *= 1024;
		} else if ((substr($comparison,-2) == 'mb') || (substr($comparison,-1) == 'm')) {
			$comp_value *= 1024 * 1024;
		} else if ((substr($comparison,-2) == 'gb') || (substr($comparison,-1) == 'g')) {
			$comp_value *= 1024 * 1024 * 1024;
		}

		$expression = "return $filesize $operator $comp_value;";
		print $expression;
		$valid = eval($expression);

		if ($valid) {
			return true;
		} else {
			$error = $q_asset->name.' '.strtolower($operators[$operator]).' '.strtoupper(easy_filesize($comp_value));
			$q_asset->active_submission->setError($error);
			return false;
		}

	}//end processForm()


}//end class
?>
