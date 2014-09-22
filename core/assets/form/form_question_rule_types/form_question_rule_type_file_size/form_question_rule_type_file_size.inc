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
* $Id: form_question_rule_type_file_size.inc,v 1.9 2013/04/08 07:52:52 ewang Exp $
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
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Type_File_Size extends Form_Question_Rule
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
		$operators = Array(
						'<='	=> translate('Must be no larger than'),

						'>='	=> translate('Must be no smaller than'),

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
	function evaluate($answer, $rule_data, &$q_asset)
	{
		//get all data we need from either current submission or incomplete submission
		if (!empty($_FILES['q'.$q_asset->id]['size'])) {
			$filesize = $_FILES['q'.$q_asset->id]['size'];
		}
		else {
			$extra_data = $q_asset->getExtraData();
			$temp_file = array_get_index($extra_data, 'temp_filesystem_path','');
			if(!empty($temp_file) && strpos($temp_file, '/') !== 0) $temp_file = SQ_SYSTEM_ROOT.'/'.$temp_file;
			$filesize = (int) filesize ($temp_file);
		}

		$operator = array_get_index($rule_data, 'operator', '<=');

		// comparison value will be the concatenation of the value and the
		// multiplier
		$comparison = array_get_index($rule_data, 'value', '').array_get_index($rule_data, 'multiplier', '');


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
		$valid = eval($expression);

		return $valid;

	}//end evaluate()


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
		$operator = array_get_index($rule_data, 'operator', '<=');
		$comparison = array_get_index($rule_data, 'value', '').' '.array_get_index($rule_data, 'multiplier', '');

		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($q_asset->_formid);
		$use_accessible_string = $form->useAccessibleFormat();
		if($use_accessible_string)
		    $error_str = translate('core_form_rule_file_size_desc_'.$operator.'_accessible', $comparison);
		else
		    $error_str = translate('core_form_rule_file_size_desc_'.$operator, $q_asset->attr('name'), $comparison);

		return $error_str;

	}//end ruleDescription()


}//end class
?>
