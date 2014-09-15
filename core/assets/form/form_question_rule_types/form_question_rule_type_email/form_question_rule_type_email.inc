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
* $Id: form_question_rule_type_email.inc,v 1.13 2013/04/19 06:36:00 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';
require_once SQ_FUDGE_PATH.'/general/www.inc';


/**
* Form_Question_Rule_Type_Email
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Type_Email extends Form_Question_Rule
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
	* returns an array of descriptions indexed by operator id
	* (which can be anything, generally string or (int)0|1 for a boolean value)
	*
	* @return array
	* @access public
	*/
	public static function getOperators()
	{
		return Array(
				1	=> translate('core_form_rule_email_true'),
				0	=> translate('core_form_rule_email_false'),
			   );

	}//end getOperators()


	/**
	* writes JavaScript code for client side validation
	*
	* @param object	&$q_asset	reference to the question asset
	* @param array	$rule_data	the rule data being used
	*
	* @return boolean
	* @access public
	*/
	function generateJSCode(&$q_asset, $rule_data)
	{
		ob_start();			// buffer this please

			$operator = array_get_index($rule_data, 'operator', 1);
			$function_id = preg_replace('/[^a-zA-Z0-9]/', '_', $q_asset->id);

			$local = '\da-zA-Z-_+'; // allowed in the first char
			$local_middle = $local.'.\w'; // allowed in the rest of the username
			$regex_code = '/^(['.$local.']['.$local_middle.'\']*['.$local.']@[\da-zA-Z][\'-.\w]*[\da-zA-Z]\.[a-zA-Z]{2,7})$/';

			// if the validation condition is set, we need to output the part that checks whether the condition is met or not as well
			$condition_js = parent::generateConditionJSCode($rule_data);
			$additional_condition = '';
			if (strlen(trim($condition_js))) {
				$additional_condition = 'if (!'.$condition_js.') return true;';
			}
			?>
			function sq_form_validate_email_<?php echo $function_id; ?>(answer)
			{
				<?php
					echo $additional_condition;
					if (!$q_asset->attr('is_required')) {
						?>
						if (answer.length == 0) {
							return true;
						}
						<?php
					}
				?>
				answer.replace(/,/, "");
				return answer.match(<?php echo $regex_code ?>);

			}//end sq_form_validate_email()

			if (<?php echo ($operator == 1) ? '!' : ''; ?>sq_form_validate_email_<?php echo $function_id; ?>(form.elements["q<?php echo $q_asset->id; ?>"].value)) {
				submission_errors[submission_errors.length] = "<?php
					if (empty($rule_data['custom_text'])) {
						echo addslashes($this->defaultError($q_asset, $rule_data));
					} else {
						echo addslashes($rule_data['custom_text']);
					}
				?>";
			}
			<?php

		$contents = ob_get_contents();
		ob_end_clean();

		return $contents;

	}//end generateJSCode()


	/**
	* processes a test on a single static value
	*
	* Rule data is an array with the following fields:
	* [operator] => operator to compare with (boolean: 1 or 0)
	*
	* @param mixed	$answer		the answer being passed to us
	* @param string	$rule_data	the rule's data
	*
	* @return boolean
	* @access public
	*/
	function evaluate($answer, $rule_data)
	{
		$operator = array_get_index($rule_data, 'operator', 1);

		$valid = valid_email($answer);
		return ($valid == $operator);

	}//end evaluate()


	/**
	* returns a description of the rule
	*
	* @param object	&$q_asset	the question asset that failed
	* @param array	$rule_data	the rule data
	*
	* @return string
	* @access public
	*/
	function ruleDescription(&$q_asset, $rule_data)
	{
		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($q_asset->_formid);
		$use_accessible_string = $form->useAccessibleFormat();
		if($use_accessible_string)
		    $error_str = translate('core_form_rule_email_desc_'.(array_get_index($rule_data, 'operator', 1) ? 'true' : 'false').'_accessible');
		 else
		    $error_str = translate('core_form_rule_email_desc_'.(array_get_index($rule_data, 'operator', 1) ? 'true' : 'false'), $q_asset->attr('name'));

		return $error_str;

	}//end ruleDescription()


}//end class
?>
