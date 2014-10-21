<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: form_question_rule_type_file_type.inc,v 1.17 2013/04/08 07:52:52 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';
/**
* Form_Question_Rule_Type_File_Type
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.17 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Type_File_Type extends Form_Question_Rule
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns a list of operators for this rule type
	*
	* @access public
	* @return array (mixed => string) an array of descriptions indexed by operator id (which can be
	*                                 anything, generally string or (int)0|1 for a boolean value)
	*/
	public static function getOperators()
	{
		return Array(
				1	=> translate('Must match one of these extensions'),

				0	=> translate('Must not match one of these extensions'),

			   );

	}//end getOperators()


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
	function evaluate($answer, $rule_data, &$q_asset)
	{


		include (SQ_FUDGE_PATH.'/standards_lists/mime_types.inc');
		//get all data we need from either current submission or incomplete submission
		if (!empty($_FILES['q'.$q_asset->id]['tmp_name'])) {
			$temp_file = $_FILES['q'.$q_asset->id]['tmp_name'];
			$answer_name = $_FILES['q'.$q_asset->id]['name'];
		}
		else {
			$extra_data = $q_asset->getExtraData();
			$temp_file = array_get_index($extra_data, 'temp_filesystem_path','');
			if(!empty($temp_file) && strpos($temp_file, '/') !== 0) $temp_file = SQ_SYSTEM_ROOT.'/'.$temp_file;
			$answer_name = basename($temp_file);
		}

		$operator = array_get_index($rule_data, 'operator', 1);
		$comparison = array_get_index($rule_data, 'value', '');
		$check_mime = array_get_index($rule_data, 'check_mime', FALSE);

		if (strrpos($answer_name, '.') === false) {
			$extension = '';
		} else {
			$extension = substr($answer_name, strrpos($answer_name, '.') + 1);
		}


		//skip test if no file is uploaded
		if ((empty($answer_name) || $answer_name == 'No file uploaded') && empty($temp_file)) return TRUE;


		//check MIME type conistency for the temp uploaded file
		//file MIME, extension MIME should be consistent, we can't rely on browser MIME type
		if ($check_mime){
			$MIME_type = file_mime_type($temp_file);
			$MIME_type_extension = isset($standards_lists_mime_types[strtolower($extension)]) ? $standards_lists_mime_types[strtolower($extension)] : '';

			//only check MIME type consistency if it has a valid MIME type
			if(!empty($MIME_type) && $MIME_type != 'text/plain' && $MIME_type != 'application/x-empty' && $MIME_type != 'application/octet-stream') {
				if(empty($MIME_type_extension)|| $MIME_type_extension != $MIME_type) return FALSE;
			}
		}



		// If the extension is empty, return as invalid, otherwise strpos will not take empty argument.
		if (empty($extension) && !empty($comparison)) {
			$valid = 0;
		} else {
			$valid = (strpos(strtolower($comparison), strtolower($extension)) !== FALSE) ? 1 : 0;
		}

		return ($valid == $operator);

	}//end evaluate()


	/**
	* writes JavaScript code for client side validation
	*
	* @param object form_question	&$q_asset		reference to the question asset
	* @param string					$operator		the operator being used to compare
	* @param mixed					&$comparison	either a value or a form_question asset (based on
	*												comparison question id), being used to compare
	*
	* @access public
	* @return boolean
	*/
	function generateJSCode(&$q_asset, $rule_data)
	{
		// Netscape and IE do not provide interfaces for accessing the
		// properties of file upload fields therefore no JS validation possible

		return '';

	}//end generateJSCode()


	/**
	* returns a default error message for this rule
	*
	* @param object Form_Question	&$q_asset	the question asset that failed
	* @param array					$rule_data	the rule data
	*
	* @return string	the error message
	* @access public
	*/
	function defaultError(&$q_asset, $rule_data)
	{
		$operator = array_get_index($rule_data, 'operator', 1);
		$comparison = array_get_index($rule_data, 'value', '');

		$comparison = str_replace("\r", '', $comparison);
		$comparison = str_replace("\n", ', ', $comparison);
		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($q_asset->_formid);
		$use_accessible_string = $form->useAccessibleFormat();
		if($use_accessible_string)
		    return translate('core_form_rule_file_type_error_'.($operator ? 'true' : 'false').'_accessible', $q_asset->getValue(), $comparison);
		else
		    return translate('core_form_rule_file_type_error_'.($operator ? 'true' : 'false'), $q_asset->getValue(), $q_asset->attr('name'), $comparison);
	}//end defaultError()


	/**
	* returns a description of the rule
	*
	* @param object Form_Question	&$q_asset	the question asset that failed
	* @param array					$rule_data	the rule data
	*
	* @return string	the error message
	* @access public
	*/
	function ruleDescription(&$q_asset, $rule_data)
	{
		$operator = array_get_index($rule_data, 'operator', 1);
		$comparison = array_get_index($rule_data, 'value', '');

		$operator_list = $this->getOperators();

		return translate('core_form_rule_file_type_desc_'.($operator ? 'true' : 'false'), $q_asset->attr('name'), $rule_data['value']);

	}//end ruleDescription()


}//end class
?>
