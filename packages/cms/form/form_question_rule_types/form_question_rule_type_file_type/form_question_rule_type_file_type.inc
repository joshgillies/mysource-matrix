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
* $Id: form_question_rule_type_file_type.inc,v 1.1 2004/11/09 04:16:17 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';

/**
* Form_Question_Rule_Type_File_Type
*
* Purpose
*
*
* @static
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Rule_Type_File_Type extends Form_Question_Rule
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question_Rule_Type_File_Type($assetid=0) 
	{
		$this->Form_Question_Rule($assetid);
	
	}//end Form_Question_Rule_Type_File_Type()


	/**
	* Returns a list of operators for this rule type
	*
	* @access public
	* @return array (mixed => string) an array of descriptions indexed by operator id (which can be
	*                                 anything, generally string or (int)0|1 for a boolean value)
	*/
	function getOperators() 
	{
		return Array(1 => 'Must be one of these extensions...',
			0 => 'Must not be one of these extensions...');
	
	}//end getOperators()
	
	
	/**
	* processes the form question and returns true if it is valid
	*
	* Because the Form Question Rule classes will not be instantiable, this function MUST called
	* statically.
	* 
	* @param object Form_Question	&$q_asset		reference to the question asset
	* @param string					$operator		the operator being used to compare
	* @param mixed					&$comparison	either a value or a form_question asset (based on
	*												comparison question id), being used to compare
	*
	* @see Form_Question_Rule::processForm()
	* @access public
	* @return boolean
	*/
	function processForm(&$q_asset, $operator, &$comparison)
	{
		if (!$q_asset->active_submission) return false;
		$file_info = $_FILES[$q_asset->id];
		$answer = $file_info['name'];
		if (strrpos($answer, '.') === false) {
			$extension = '';
		} else {
			$extension = substr($answer, 1 + strrpos($answer, '.'));
		}
		
		$valid = (strpos(strtolower($comparison), strtolower($extension)) !== false) ? 1 : 0;

		if ($valid == $operator) {
			return true;
		} else {
			// tailor the error response depending
			$error = $q_asset->name.' must '. (($operator) ? '' : 'not') .' have one of the following extensions: \''.$comparison.'\'';
			$q_asset->active_submission->setError($error);
			return false;
		}
	

	}//end processForm()


	/**
	* processes a test on a single static value
	*
	* Used when we are using rules outside of validating a form - for example selective emails.
	*
	* Because the Form Question Rule classes will not be instantiable by Matrix NOR will be
	* instantiated manually in code, this function MUST called statically.
	*
	* The value being tested in this case should be the file name of the uploaded file.
	* 
	* @param mixed	$answer			the answer being passed to us
	* @param string	$operator		the operator being used to compare
	* @param mixed	&$comparison	either a value or a form_question asset (based on
	*								comparison question id), being used to compare
	*
	* @static
	* @access public
	* @return boolean true only if rule succeeds
	*/
	function valueTest($answer, $operator, &$comparison)
	{
		if (strrpos($answer, '.') === false) {
			$extension = '';
		} else {
			$extension = substr($answer, strrpos($answer, '.'));
		}
		
		$valid = (strpos(strtolower($comparison), strtolower($answer)) !== false) ? 1 : 0;

		return ($valid == $operator);
	

	}//end valueTest()


}//end class
?>
