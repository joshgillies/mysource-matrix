<?php

require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';

/**
* Form_Question_Rule_Type_Comparison
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Rule_Type_Comparison extends Form_Question_Rule
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question_Rule_Type_Comparison($assetid=0) 
	{
		$this->Form_Question_Rule($assetid);
	
	}// end Form_Question_Rule_Type_Email()


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = parent::create($link);

		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

	} // end create()


	/**
	* returns the operators 
	*
	* @access public
	* @return Array()
	*/
	function getOperators()
	{
		$operators = Array(
			'==' => 'Must Be Equal to ',
			'!=' => 'Must Not Be Equal to ',
			'>'  => 'Must be Greater Than ',
			'<'  => 'Must be Less Than ',
			'>=' => 'Must be Greater Than or Equal To ',
			'<=' => 'Must be Less Than or Equal To ',
		);

		return $operators;

	} // getOperators


	/**
	* returns true if the question is valid, otherwise 
	* adds an error to the submission object, and return false
	* 
	* @see Form_Question_Rule::processForm()
	*
	* @param &object form_submission	$submission		the submission object for this form
	* @param	int						$questionid		the id of the question this rule is linked to 
	* @param string						$question_name	the name of the question (used for producing errors)
	*
	* @access public
	* @return boolean
	*/
	function processForm(&$submission, $questionid, $question_name)
	{
		$answer   = $submission->getAnswer($questionid);
		$operator = $this->attr('operator');
		$value	  = $this->attr('value'); 
		
		$operators = $this->getOperators();

		// the operator is in '==, !=, <, >, <=, >='
		// so the query will look something like this:
		// 12==12

		// if there is no operator, assign it to ==
		if (!$operator) $operator = '==';

		if (is_array($answer)) {
			$valid = 0;		
			foreach ($answer as $a) {
				$query = 'return \''.$a.'\''.$operator.'\''.$value.'\';';
				if (eval($query)) {
					$valid = 1;
					break;
				}				
			}
		} else {
			$query = 'return \''.$answer.'\''.$operator.'\''.$value.'\';';
			(eval($query)) ? $valid = 1 : $valid = 0; 
		}
		
		if ($valid) {
			return true;
		} else {
			// if this rule's question is a selection, we need to ask it to covert from an offset, to a value
			if ($this->isSelection()) {
				$q = $this->getMyQuestion();
				$value = $q->getAnswerByOffset($value);
			}
			$error = $question_name.' '.$operators[$operator].' \''.$value.'\'';
			$submission->setError($error);
			return false;
		}

	}// end processForm()

}// end class
?>