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
* $Id: form_question_type_datetime.inc,v 1.3.2.1 2004/03/26 16:41:54 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question/form_question.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';

/**
* Form_Question_Type_Text
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_Datetime extends Form_Question
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question_Type_Datetime($assetid=0) 
	{
		$this->Asset($assetid);

	} // end Form_Question_Type_Datetime()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$links = parent::_getAllowedLinks();
		$links[SQ_LINK_TYPE_3]['form_question_rule_type_comparison'] = Array('card' => '1', 'exclusive' => true);
		$links[SQ_LINK_TYPE_3]['form_question_rule_type_email'] = Array('card' => '1', 'exclusive' => true);
		$links[SQ_LINK_TYPE_3]['form_question_rule_type_numeric'] = Array('card' => '1', 'exclusive' => true);
		$links[SQ_LINK_TYPE_3]['form_question_rule_type_found_in'] = Array('card' => '1', 'exclusive' => true);
		$links[SQ_LINK_TYPE_3]['form_question_rule_type_regexp'] = Array('card' => '1', 'exclusive' => true);
		$links[SQ_LINK_TYPE_3]['form_question_rule_type_contain'] = Array('card' => '1', 'exclusive' => true);
		return $links;

	}//end _getAllowedLinks()


	/**
	* generates the html for this question to display it in the frontend
	*
	* @param boolean	$content_file	if TRUE, this function will generate content for the content file
	* @param string		$value			current value of the question
	* @param string		$suffix			the suffix to append to the name
	* @param boolean	$parameters		if TRUE a default set of parameters will be used for the datetime field
	*
	* @access public
	* @return string
	*/
	function getAnswer($content_file=true, $value='', $suffix='', $default = false)
	{
		$extras = $this->attr('extra');
		$name   = $this->id;


		// generate some php for the value, which can be used for both
		// auto-answer and returning to saved forms
		if ($content_file) {
			$html  = '<'.'?php $q = &$GLOBALS[\'SQ_SYSTEM\']->am->getAsset('
			.$this->id.', \''.get_class($this).'\'); '
			.' $q->active_submission =& $this->active_submission; ?'.'>';

			$value = '<'.'?php echo $q->getValue(); ?'.'>';
		} else {
			$html = '';
		}

		if ($default) {
			$parameters = $this->getDefaultParameters();
		} else {
			$elements = Array('day' => 'd', 'month' => 'm', 'year' => 'y', 'hour' => 'h', 'min' => 'i', 'sec' => 's');

			$parameters = Array();
			$parameters['show'] = Array();
			$parameters['null'] = Array();
			$parameters['style'] = Array();
			$parameters['min'] = $this->attr('min');
			$parameters['max'] = $this->attr('max');
			$parameters['allow_circa'] = $this->attr('circa');

			foreach ($elements as $attr => $t) {
				$parameters['style'][$t] = ($this->attr($attr.'_text') ? 't' : 's');	
				if ($this->attr($attr)) $parameters['show'][] = $t;
				if ($this->attr($attr.'_null')) $parameters['null'][$t] = 1;
			}
		}

		$dt_field = new Datetime_Field($this->id.$suffix, $value, $parameters);
		
		ob_start();
		$dt_field->printField();
		$html .= ob_get_contents();
		ob_end_clean();
				
		// so we need to convert them back to proper php tags
		return html_entity_decode($html);

	} // end getAnswer()


	/**
	* returns a set of default parameters for the datatime_field class
	*
	* @access public
	* @return Array(mixed)
	*/
	function getDefaultParameters()
	{
		$elements = Array('day' => 'd', 'month' => 'm', 'year' => 'y', 'hour' => 'h', 'min' => 'i', 'sec' => 's');

		$parameters = Array();
		$parameters['show'] = Array();
		$parameters['null'] = Array();
		$parameters['style'] = Array();
		$parameters['min'] = '01-00-1970 00:00:00';
		$parameters['max'] = '31-12-2035 23:59:59';
		$parameters['allow_circa'] = 0;
	
		foreach ($elements as $attr => $t) {
			$parameters['style'][$t] = 's';
			$parameters['show'][] = $t;
			$parameters['null'][$t] = 1;
		}

		return $parameters;

	} // end getDefaultParameters()
	

	/**
	* echos the current value
	*
	* @param &$object $submission the current submission object
	*
	* @access public
	* @return void
	*/
	function getValue()
	{
		if ($this->active_submission) {
			$answer = $this->active_submission->getAnswer($this->id);
			if ($answer) echo $answer;
		}
	
	} // end getValue()


	/**
	* process this question
	*
	* @param &object $submission the current submission object
	*
	* @access public
	* @return boolean
	*/
	function processForm()
	{
		if (!$this->active_submission) return false;
		if (isset($_POST[$this->id.'value'])) {
			// we need to create a variable for these value to pass by reference
			$value = '';
			$parameters = Array();
			$dt_field = new Datetime_Field($this->id, $value, $parameters);
			$dt_field->processField();
			$val = $_POST[$this->id.'value'];
			$this->active_submission->setAnswer($this->id, $val);
			$this->active_submission->setSummary($this->id, $this->attr('name'), $val, $this->section_id, $this->section_name);

			parent::processForm();
		}
		
		return true;
	
	} // end processForm()

}//end class
?>
