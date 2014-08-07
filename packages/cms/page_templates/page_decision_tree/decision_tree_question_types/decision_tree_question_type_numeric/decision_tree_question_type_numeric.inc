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
* $Id: decision_tree_question_type_numeric.inc,v 1.2 2012/08/30 00:58:08 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_decision_tree/decision_tree_question/decision_tree_question.inc';

/**
* Decision_Tree_Question_Type_Numeric
*
*
* @author  Mohamed Haidar <mhaidar@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Decision_Tree_Question_Type_Numeric extends Decision_Tree_Question
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
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		return $page_links;

	}//end _getAllowedLinks()
	
	
	/**
	* generates the html for this question to display it in the frontend
	*
	* @return string
	* @access public
	*/
	function getHtmlField()
	{
		
		$width  = $this->attr('width');
		$max    = $this->attr('max');
		$extras = $this->attr('extra');
		$name   = 'q'.$this->id;

		// prepare a valid value for the field id
		$extras = $extras.' id="'.$name.'"';

		ob_start();
		text_box($name, '', $width, $max, FALSE, $extras);
		$html = ob_get_contents();
		ob_end_clean();

		return $html;

	}//end getHtmlField()
	
	/**
	* generates the html for the submit button of this questions text field
	*
	* @return string
	* @access public
	*/
	function getHtmlButton()
	{
		
		$name   = 'q'.$this->id.'_button';
		$value = $this->attr('button_value');
		$on_click = "get_next_step(document.getElementById('q$this->id').value, $this->id)";
		
		ob_start();
		normal_button($name, $value, $on_click, '');
		$html = ob_get_contents();
		ob_end_clean();
		
		return $html;
		
	}//end getHtmlButton()
	
	
	/**
	* Returns a list of operators for this rule type
	*
	* @access public
	* @return array (mixed => string) an array of descriptions indexed by operator id (which can be
	*                                 anything, generally string or (int)0|1 for a boolean value)
	*/
	public function getOperators()
	{
		$operators = Array(
						'INCL' 	  => translate('all_inclusive'),
						'L_INCL'  => translate('left_inclusive'),
						'R_INCL'  => translate('right_inclusive'),
						'N_INCL'  => translate('not_inclusive'),
					 );

		return $operators;

	}//end getOperators()
	
	
	/**
	* processes a test on two static values
	*
	* Rule data is an array with the following fields:
	* [operator] => @see getOperators()
	* [value_1] => comparison static value 1
	* [value_2] => comparison static value 2
	*
	* @param mixed	$answer		the answer being passed to us
	* @param string	$rule_data	the rule's data
	*
	* @return boolean
	* @access public
	*/
	function evaluate($answer, $rule_data)
	{
		$answer = trim($answer);
		if (strlen($answer) == 0 || !is_numeric($answer)) return FALSE;
		
		if (empty($rule_data)) return FALSE;
		if ((strlen($rule_data['value_1']) == 0 && strlen($rule_data['value_2']) == 0) || empty($rule_data['logic'])) return FALSE;
		
		switch ($rule_data['logic']) {
			case 'INCL':
				if (strlen($rule_data['value_1']) == 0){
					$valid = ($answer <= $rule_data['value_2']);
				} else if (strlen($rule_data['value_2']) == 0){
					$valid = ($answer >= $rule_data['value_1']);
				} else {
					$valid = ($answer >= $rule_data['value_1'] && $answer <= $rule_data['value_2']);
				}	
			break;

			case 'L_INCL':
				if (strlen($rule_data['value_1']) == 0){
					$valid = ($answer < $rule_data['value_2']);
				} else if (strlen($rule_data['value_2']) == 0){
					$valid = ($answer >= $rule_data['value_1']);
				} else {
					$valid = ($answer >= $rule_data['value_1'] && $answer < $rule_data['value_2']);
				}
			break;

			case 'R_INCL':
				if (strlen($rule_data['value_1']) == 0){
					$valid = ($answer <= $rule_data['value_2']);
				} else if (strlen($rule_data['value_2']) == 0){
					$valid = ($answer > $rule_data['value_1']);
				} else {
					$valid = ($answer > $rule_data['value_1'] && $answer <= $rule_data['value_2']);
				}
			break;
			
			case 'N_INCL':
				if (strlen($rule_data['value_1']) == 0){
					$valid = ($answer < $rule_data['value_2']);
				} else if (strlen($rule_data['value_2']) == 0){
					$valid = ($answer > $rule_data['value_1']);
				} else {
					$valid = ($answer > $rule_data['value_1'] && $answer < $rule_data['value_2']);
				}
			break;
			
			default:
				$valid = FALSE;
		}

		return $valid;

	}//end evaluate()

	
	/**
	* Prints a readable description of the passed in rule data
	*
	* Rule data is an array with the following fields:
	* [operator] => @see getOperators()
	* [value_1] => comparison static value 1
	* [value_2] => comparison static value 2
	*
	* @param string	$rule_data	the rule's data
	*
	* @return boolean
	* @access public
	*/
	function printDescription($rule_data)
	{	
	
		if ((strlen($rule_data['value_1']) == 0 && strlen($rule_data['value_2']) == 0) || empty($rule_data['logic'])) return;
		switch ($rule_data['logic']) {
			case 'INCL':
				if (strlen($rule_data['value_1']) == 0){
					echo "<= $rule_data[value_2]";
				} else if (strlen($rule_data['value_2']) == 0){
					echo ">= $rule_data[value_1]";
				} else {
					echo (">= $rule_data[value_1] AND <= $rule_data[value_2]");
				}	
			break;

			case 'L_INCL':
				if (strlen($rule_data['value_1']) == 0){
					echo "< $rule_data[value_2]";
				} else if (strlen($rule_data['value_2']) == 0){
					echo ">= $rule_data[value_1]";
				} else {
					echo (">= $rule_data[value_1] AND < $rule_data[value_2]");
				}	
			break;

			case 'R_INCL':
				if (strlen($rule_data['value_1']) == 0){
					echo "<= $rule_data[value_2]";
				} else if (strlen($rule_data['value_2']) == 0){
					echo "> $rule_data[value_1]";
				} else {
					echo ("> $rule_data[value_1] AND <= $rule_data[value_2]");
				}	
			break;
			
			case 'N_INCL':
				if (strlen($rule_data['value_1']) == 0){
					echo "< $rule_data[value_2]";
				} else if (strlen($rule_data['value_2']) == 0){
					echo "> $rule_data[value_1]";
				} else {
					echo ("> $rule_data[value_1] AND < $rule_data[value_2]");
				}	
			break;
			
			default:
				echo '';
		}

	}//end printDescription()

}//end class
?>
