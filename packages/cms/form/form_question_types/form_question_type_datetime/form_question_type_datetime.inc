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
* $Id: form_question_type_datetime.inc,v 1.11 2004/05/20 05:33:22 lwright Exp $
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
	* @param array	$data		the section's entry for this question (contains all question data)
	*
	*/
	function Form_Question_Type_Datetime($assetid=0,$data=Array()) 
	{
		$this->Form_Question($assetid,$data);

	}//end Form_Question_Type_Datetime()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array();

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
		//if ($content_file) {
			//$html  = '<'.'?php $q = &$GLOBALS[\'SQ_SYSTEM\']->am->getAsset('
			//.$this->id.', \''.get_class($this).'\'); '
			//.' $q->active_submission =& $this->active_submission; ?'.'>';

			 //'<'.'?php echo $q->getValue(); ?'.'>';
		//} else {
			$html = '';
			$value = $this->attr('default');
		//}

		if ($default) {
			$parameters = $this->getDefaultParameters();
		} else {
			$elements = Array('day' => 'd', 'month' => 'm', 'year' => 'y', 'hour' => 'h', 'min' => 'i', 'sec' => 's');

			$parameters = Array();
			$parameters['show'] = Array();
			$parameters['null'] = Array();
			$parameters['style'] = Array();
			$parameters['min'] = $this->attr('minimum');
			if ($parameters['min'] == '---------- --:--:--') $parameters['min'] = '1970-01-01 00:00:00';
			$parameters['max'] = $this->attr('maximum');
			if ($parameters['max'] == '---------- --:--:--') $parameters['max'] = '2030-12-31 23:59:59';

			$show_params = $this->attr('show');
			$text_params = $this->attr('text');
			$null_params = $this->attr('null');

			$parameters['allow_circa'] = $show_params['c'];

			foreach ($elements as $attr => $t) {
				$parameters['style'][$t] = ($text_params[$t] ? 't' : 's');	
				if ($show_params[$t]) $parameters['show'][] = $t;
				if ($null_params[$t]) $parameters['null'][$t] = 1;
			}
		}

		$dt_field = new Datetime_Field($this->id.$suffix, $value, $parameters);
		
		ob_start();
		$dt_field->printField();
		$html .= ob_get_contents();
		ob_end_clean();
				
		// so we need to convert them back to proper php tags
		return html_entity_decode($html);

	}//end getAnswer()


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
		$parameters['show']['c'] = 0;
	
		foreach ($elements as $attr => $t) {
			$parameters['style'][$t] = 's';
			$parameters['show'][] = $t;
			$parameters['null'][$t] = 1;
		}

		return $parameters;

	}//end getDefaultParameters()
	

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
	
	}//end getValue()


	/**
	* returns an array of the types of rules that are allowed upon this asset
	*
	* Returned array is 'short name' => 'full name'
	*
	* @access public
	* @return Array(string => string)
	*/
	function getAllowedRules() {
		return Array();
	
	}//end getAllowedRules()


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
		parent::processForm();

		if (!$this->active_submission) return false;
		if (isset($_POST[$this->id.'value'])) {
			// we need to create a variable for these value to pass by reference
			$value = '';
			$parameters = Array();
			$dt_field = new Datetime_Field($this->id, $value, $parameters);
			$dt_field->processField();
			$val = $_POST[$this->id.'value'];
			$this->active_submission[$this->id] = ts_iso8601(mktime($val['h'], $val['i'], $val['s'], $val['m'], $val['d'], $val['y'])).(!isset($val['c']) ? '' : (' '.($val['c'] ? '1' : '0')));
		}
		
		return true;
	
	}//end processForm()


	/**
	* returns the answer for this question for the summary, generally overriden by children
	*
	* @param $answer the answer of the question
	*
	* @access public
	* @return string
	*/
	function getSummary($answer)
	{
		$allow_circa = false;
		
		$show_params = $this->attr('show');
		if ($show_params['c']) {
			$allow_circa = true;
		}

		if ($allow_circa) {
			list($date, $time, $circa) = explode(' ', $answer);
		} else {
			list($date, $time) = explode(' ', $answer);
			$circa = 0;
		}

		$timestamp = iso8601_ts($date . ' ' . $time);

		return ($circa ? 'circa ' : '') . date('l, F jS, Y H:i:s', $timestamp);
	
	}//end getSummary()


	/**
	* returns an answer summary in XML format (suitable for submission logs)
	* 
	* @access public
	* @return string
	*/
	function getXML($answer)
	{
		$allow_circa = false;

		$show_params = $this->attr('show');
		if ($show_params['c']) {
			$allow_circa = true;
		}

		if ($allow_circa) {
			list($date, $time, $circa) = explode(' ', $answer);
		} else {
			list($date, $time) = explode(' ', $answer);
			$circa = 0;
		}

		echo '<datetime_q id="'.addslashes($this->id).'"  name="'.addslashes($this->attr('name')).'"'.($circa ? ' circa="1"' : '').'>';

		$iso = $date . 'T' . $time;
		echo $iso;

		echo '</datetime_q>';
	}//end getXML()
	
	
}//end class
?>
