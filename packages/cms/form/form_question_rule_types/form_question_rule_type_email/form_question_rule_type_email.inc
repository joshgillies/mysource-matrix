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
* $Id: form_question_rule_type_email.inc,v 1.17.2.4 2006/05/17 02:03:30 sdanis Exp $
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
* @version $Revision: 1.17.2.4 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Rule_Type_Email extends Form_Question_Rule
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question_Rule_Type_Email($assetid=0)
	{
		$this->Form_Question_Rule($assetid);

	}//end constructor


	/**
	* Returns a list of operators for this rule type
	*
	* @access public
	* @return array (mixed => string) an array of descriptions indexed by operator id (which can be
	*                                 anything, generally string or (int)0|1 for a boolean value)
	*/
	function getOperators()
	{
		return Array(
				1	=> translate('cms_form_rule_email_true'),
				0	=> translate('cms_form_rule_email_false'),
			   );

	}//end getOperators()


	/**
	* writes JavaScript code for client side validation
	*
	* @param object Form_Question	&$q_asset		reference to the question asset
	* @param Array					$rule_data		the rule data being used
	*
	* @access public
	* @return boolean
	*/
	function generateJSCode(&$q_asset, $rule_data)
	{
		// have we printed out the validation function yet?
		// (since this is static across all class instances, this will work)
		static $printed_fn = false;

		$operator = array_get_index($rule_data, 'operator', 1);

		// regex code - same as valid_email() in fudge/general/www.inc
		$regex_code = '/^[A-Za-z0-9](([_\.\-\']?[A-Za-z0-9]+)*)@([0-9a-zA-Z]([0-9A-Za-z-]*)+\.)+[a-zA-Z]{2,}$/';

		ob_start();			// buffer this please

			if (!$printed_fn) {
				$printed_fn = true;

					?>
					function sq_form_validate_email(answer)
					{
						<?php
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
					<?php

			}

			?>
				if (<?php echo ($operator == 1) ? '!' : ''; ?>sq_form_validate_email(form.elements["q<?php echo $q_asset->id; ?>"].value)) {
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
	* @param mixed	$answer			the answer being passed to us
	* @param string	$rule_data		the rule's data
	*
	* @access public
	* @return boolean true only if rule succeeds
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
	* @param object Form_Question	&$q_asset	the question asset that failed
	* @param array					$rule_data	the rule data
	*
	* @return string	the error message
	* @access public
	*/
	function ruleDescription(&$q_asset, $rule_data)
	{
		$error_tail = 'false';
		if (!isset($rule_data['operator']) || $rule_data['operator']) {
			$error_tail = 'true';
		}
		$error_str = translate('cms_form_rule_email_desc_'.$error_tail, $q_asset->attr('name'));

		return $error_str;

	}//end ruleDescription()


}//end class
?>
