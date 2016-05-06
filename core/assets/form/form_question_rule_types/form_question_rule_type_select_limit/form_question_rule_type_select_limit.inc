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
* $Id: form_question_rule_type_select_limit.inc,v 1.13 2013/04/19 06:36:01 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';

/**
* Form_Question_Rule_Type_Select_Limit
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Type_Select_Limit extends Form_Question_Rule
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	* @return void
	* @access public
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
						'=='	=> 'Must select exactly this many options...',
						'!='	=> 'Must not select this many options...',
						'>'		=> 'Must select more than this many options...',
						'<'		=> 'Must select fewer than this many options...',
						'>='	=> 'Must select at least this many options...',
						'<='	=> 'Must select no more than this many options...',
					 );

		return $operators;

	}//end getOperators()


	/**
	* writes JavaScript code for client side validation
	*
	* This function assumes the skeleton of a JS validation function has been created around this code
	* with a form variable named 'form' passed (this is done when generating the content file).
	*
	* @param object form_question	&$q_asset		reference to the question asset
	* @param string					$operator		the operator being used to compare
	* @param mixed					&$comparison	either a value or a form_question asset (based on
	*												comparison question id), being used to compare
	*
	* @return boolean
	* @access public
	*/
	function generateJSCode(&$q_asset, $rule_data)
	{
		$operator = array_get_index($rule_data, 'operator', '==');
		$value = array_get_index($rule_data, 'value', '');

		$q_name = 'q'.$q_asset->id;

		if ($q_asset instanceof Form_Question_Type_Select) {
			// selection list or derivative - check multiple attribute
			$q_id = $q_name.($q_asset->attr('multiple') ? '[]' : '');
		} else if ($q_asset instanceof Form_Question_Type_Option_List) {
			// option list - always single (*sniff* :`-( )
			$q_id = $q_name;
		} else {
			// tickbox list - always multiple
			$q_id = $q_name.'[]';
		}

		$q_safe_name = str_replace(':', '_', $q_name);

		// if the validation condition is set, we need to output the part that checks whether the condition is met or not as well
		$condition_js = parent::generateConditionJSCode($rule_data);
		$additional_condition = '';
		if (strlen(trim($condition_js))) {
			$additional_condition = 'if ('.$condition_js.') {';
		}

		// since this is going to be a generic rule type applicable to both select lists and tickbox
		// lists, we need different types of magic for each possible question type.

		// For a selection list (and descendants) we need something like this.......
		ob_start();

			?>
			var selectCount = 0;

			<?php
			echo $additional_condition;
			if ($q_asset instanceof Form_Question_Type_Select) {
				?>var select = form.elements["<?php echo $q_id ?>"];
				for(i=0; i<select.options.length; i++) {
					if (select.options[i].selected) selectCount++;
				}<?php
			} else {			// tickbox (or option) list needs something else
				foreach ($q_asset->getOptions() as $key => $option) {
					?>if (document.getElementById("<?php echo $q_safe_name.'_'.$key ?>").checked) selectCount++;
					<?php
				}
			}

			?>
			if (!(selectCount <?php echo $operator ?> <?php echo $value ?>)) {
				submission_errors[submission_errors.length] = 
					'<?php echo $this->defaultError($q_asset, $rule_data, '%s'); ?>'.replace('%s', selectCount);
			}
			<?php
			if (strlen(trim($additional_condition))) echo '}';
		$contents = ob_get_contents();
		ob_end_clean();

		return $contents;

	}//end generateJSCode()


	/**
	* processes a test on a single static value
	*
	* Rule data is an array with the following fields:
	* [operator] => operator to compare with (string, ==, !=, <, <=, >, >=)
	* [value] => comparison static value
	*
	* @param mixed	$answer		the answer being passed to us
	* @param string	$rule_data	the rule's data
	*
	* @return boolean
	* @access public
	*/
	function evaluate($answer, $rule_data)
	{
		$operator = array_get_index($rule_data, 'operator', '==');
		$value = array_get_index($rule_data, 'value', '');

		$valid = FALSE;
		switch($operator) {
			case '==':
				$valid = count($answer) == $value;
				break;
			case '!=':
				$valid = count($answer) != $value;
				break;
			case '>':
				$valid = count($answer) > $value;
				break;
			case '<':
				$valid = count($answer) < $value;
				break;
			case '>=':
				$valid = count($answer) >=  $value;
				break;
			case '<=':
				$valid = count($answer) <=  $value;
				break;
		}

		return $valid;

	}//end evaluate()


	/**
	* returns a default error message for this rule
	*
	* @param object	&$q_asset	the question asset that failed
	* @param array	$rule_data	the rule data
	*
	* @return string
	* @access public
	*/
	function defaultError(&$q_asset, $rule_data, $selected_options=NULL)
	{
		$operator = array_get_index($rule_data, 'operator', '==');
		$value    = array_get_index($rule_data, 'value', '');

		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($q_asset->_formid);
		$use_accessible_string = $form->useAccessibleFormat();
		
		if ($operator == '>=' && $value == 1) {
			// Handle "required" situations differently.
			if (strlen($q_asset->attr('cust_required_error')) > 0) {
				// Custom required error.
				$error_str = $q_asset->attr('cust_required_error');
			} else if ($use_accessible_string === TRUE) {
				// Provide a default "required" string for accessible forms only.
				if ($q_asset->attr('multiple') === TRUE) {
					$error_str = translate('This question is required. Please select at least one option.');	
				} else {
					$error_str = translate('This question is required. Please select an option.');
				}
			} else {
				// Provide default error text as per below.
				$error_str = sprintf(translate('You have selected %1$s options at question "%2$s" - you must select at least %3$s'), 0, $q_asset->attr('name'), 1);
			}
		} else {
			if ($selected_options === NULL) {
				$selected_options = count($q_asset->getValue());
			}
		
			$error_text = Array(
				'=='     => translate('You have selected %2$s options at question "%1$s" - you must select %3$s'),
				'==_acc' => translate('You have selected %2$s options. You must select %3$s.'),
				'!='     => translate('You have selected %2$s options at question "%1$s" - you must not select this many'),
				'!=_acc' => translate('You have selected %2$s options. You must not select this many - you may select more or fewer.'),
				'>'      => translate('You have selected %2$s options at question "%1$s" - you must select more than %3$s'),
				'>_acc'  => translate('You have selected %2$s options. You must select more than %3$s.'),
				'<'      => translate('You have selected %2$s options at question "%1$s" - you must select fewer than %3$s'),
				'<_acc'  => translate('You have selected %2$s options. You must select fewer than %3$s.'),
				'>='     => translate('You have selected %2$s options at question "%1$s" - you must select at least %3$s'),
				'>=_acc' => translate('You have selected %2$s options. You must select at least %3$s.'),
				'<='     => translate('You have selected %2$s options at question "%1$s" - you must select no more than %3$s'),
				'<=_acc' => translate('You have selected %2$s options. You must select no more than %3$s.'),
			);

			if ($use_accessible_string) {
				$operator .= '_acc';
			}

			$error_str = sprintf(
				$error_text[$operator],
				$q_asset->attr('name'),
				$selected_options,
				$value
			);
		}
		return $error_str;

	}//end defaultError()


	/**
	* returns a description for the rule
	*
	* @param object	&$q_asset	the question asset that failed
	* @param array	$rule_data	the rule data
	*
	* @return string
	* @access public
	*/
	function ruleDescription(&$q_asset, $rule_data)
	{
		
		$operator = array_get_index($rule_data, 'operator', '==');
		$value    = array_get_index($rule_data, 'value', '');

		$description_text = Array(
			'==' => translate('Exactly %2$s options must be selected at question "%1$s"'),
			'!=' => translate('Any number of options but %2$s must be selected at question "%1$s"'),
			'>'  => translate('More than %2$s options must be selected at question "%1$s"'),
			'<'  => translate('Fewer than %2$s options must be selected at question "%1$s"'),
			'>=' => translate('At least %2$s options must be selected at question "%1$s"'),
			'<=' => translate('No more than %2$s options must be selected at question "%1$s"'),
		);

		$error_str = sprintf(
			$description_text[$operator],
			$q_asset->attr('name'),
			$value
		);

		return $error_str;

	}//end ruleDescription()


}//end class
?>
