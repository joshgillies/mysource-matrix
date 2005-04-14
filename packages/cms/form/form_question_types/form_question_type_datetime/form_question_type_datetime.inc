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
* $Id: form_question_type_datetime.inc,v 1.17.2.1 2005/04/14 01:43:57 lwright Exp $
*
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
* @version $Revision: 1.17.2.1 $
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

	}//end constructor


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
	* returns the current value of the question, or the default value if no current value exists
	*
	* @access public
	* @return mixed type depends on the subclass of question
	*/
	function getValue()
	{
		if (!$this->active_submission) {
			return null;
		}

		if (isset($this->active_submission[$this->id])) {
			return $this->active_submission[$this->id];
		} else {
			return $this->attr('default');
		}

	}//end getValue()


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
	function getAnswer($content_file=true, $value='', $suffix='', $default=false)
	{
		$extras = $this->attr('extra');
		$name   = $this->id;

		$html = '';
		$value = $this->getValue();

		if ($default) {
			$parameters = $this->getDefaultParameters();
		} else {
			$elements = Array('day' => 'd', 'month' => 'm', 'year' => 'y', 'hour' => 'h', 'min' => 'i', 'sec' => 's');

			$parameters = Array();
			$parameters['show'] = Array();
			$parameters['null'] = Array();
			$parameters['style'] = Array();
			$parameters['min'] = $this->attr('minimum');
			if ($parameters['min'] == '---------- --:--:--') $parameters['min'] = '1900-01-01 00:00:00';
			$parameters['max'] = $this->attr('maximum');
			if ($parameters['max'] == '---------- --:--:--') $parameters['max'] = '2099-12-31 23:59:59';

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
	* returns an array of the types of rules that are allowed upon this asset
	*
	* Returned array is 'short name' => 'full name'
	*
	* @access public
	* @return Array(string => string)
	*/
	function getAllowedRules()
	{
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
	function evaluate($answer=null)
	{
		if (isset($_POST[$this->id.'value'])) {
			// we need to create a variable for these value to pass by reference
			$value = '';
			$parameters = Array();
			$dt_field = new Datetime_Field($this->id, $value, $parameters);
			$dt_field->processField();
			$val = $_POST[$this->id.'value'];
			$value = sprintf('%04s-%02s-%02s %02s:%02s:%02s', $val['y'], $val['m'], $val['d'], $val['h'], $val['i'], $val['s']).(!isset($val['c']) ? '' : (' '.($val['c'] ? '1' : '0')));

			$ok = parent::evaluate($value);
		}

		return $ok;

	}//end evaluate()


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

		$val = Array();

		// break apart the date into correct portions
		list($val['y'], $val['m'], $val['d']) = sscanf($date, '%04s-%02s-%02s');

		// build the date string based on what wacky combinations we have
		$show_date_str = (($val['y'] == '----') ? '' : 'y').(($val['m'] == '--') ? '' : 'm').(($val['d'] == '--') ? '' : 'd');

		switch ($show_date_str) {
			case 'ymd':
				$temp_date = mktime(0,0,0,$val['m'],$val['d'],2000);
				$show_date = date('F jS', $temp_date).', '.$val['y'];
			break;

			case 'yd':
				$temp_date = mktime(0,0,0,1,$val['d'],2000);
				$show_date = date('jS', $temp_date).' day of month, '.$val['y'];
			break;

			case 'md':
				$temp_date = mktime(0,0,0,$val['m'],$val['d'],2000);
				$show_date = date('F jS', $temp_date);
			break;

			case 'ym':
				$temp_date = mktime(0,0,0,$val['m'],1,2000);
				$show_date = date('F', $temp_date).', '.$val['y'];
			break;

			case 'y':
				$show_date = 'Year '.$val['y'];
			break;

			case 'm':
				$temp_date = mktime(0,0,0,$val['m'],1,2000);
				$show_date = date('F', $temp_date);
			break;

			case 'd':
				$temp_date = mktime(0,0,0,1,$val['d'],2000);
				$show_date = date('jS', $temp_date).' day of month';
			break;

			default:
				$show_date = '';
			break;

		}

		// break apart the date into correct portions
		list($val['h'], $val['i'], $val['s']) = sscanf($time, '%02s-%02s-%02s');

		// do similar with the time string
		$show_time_str = (($val['h'] == '--') ? '' : 'h').(($val['i'] == '--') ? '' : 'i').(($val['s'] == '--') ? '' : 's');

		switch ($show_time_str) {
			case 'his':
				$show_time = $time;
			break;

			case 'hi':
				$show_time = substr($time,5);
			break;

			case 'hs':
				$show_time = date('F jS', $temp_date);
			break;

			case 'is':
				$show_time = $val['i'].' min '.$val['s'].' sec';
			break;

			case 'h':
				$show_time = $val['h'].' hour'.($val['h'] == 1 ? '' : 's');
			break;

			case 'i':
				$show_time = $val['i'].' minute'.($val['i'] == 1 ? '' : 's');
			break;

			case 's':
				$show_time = $val['s'].' seconds'.($val['s'] == 1 ? '' : 's');
			break;

			default:
				$show_date = '';
			break;

		}

		// so we can extract the correct month and day-suffix- using 2000 as it's a leap year (so we can extract for Jan 29).

		return ($circa ? 'circa ' : '') . trim($show_date).(empty($show_date) || empty($show_time) ? '' : ', ').trim($show_time);

	}//end getSummary()


	/**
	* returns an answer summary in XML format (suitable for submission logs)
	*
	* @param mixed	$answer	the answer for the question
	*
	* @access public
	* @return string the summary XML
	*/
	function getXML($answer)
	{
		ob_start();

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

		echo '<datetime_q id="'.addslashes($this->id).'"  name="'.htmlspecialchars($this->attr('name')).'"'.($circa ? ' circa="1"' : '').'>';

		$iso = $date . 'T' . $time;
		echo $iso;

		echo '</datetime_q>';

		$contents = ob_get_contents();
		ob_end_clean();

		return $contents;

	}//end getXML()


	/**
	* Populates this question's response from the form data
	*
	* @return boolean
	* @access public
	*/
	function populate()
	{
		$val = '';
		$value = $_POST[$this->id.'value'];
		$show = $_POST[$this->id.'show'];

		if (strpos($show, 'y') === false) {
			$val .= '----';
		} else {
			$val .= str_pad($_POST[$this->id.'value']['y'], 4, '0', STR_PAD_LEFT);
		}

		$val .= '-';

		if (strpos($show, 'm') === false) {
			$val .= '--';
		} else {
			$val .= str_pad($_POST[$this->id.'value']['m'], 2, '0', STR_PAD_LEFT);
		}

		$val .= '-';

		if (strpos($show, 'd') === false) {
			$val .= '--';
		} else {
			$val .= str_pad($_POST[$this->id.'value']['d'], 2, '0', STR_PAD_LEFT);
		}

		$val .= ' ';

		if (strpos($show, 'h') === false) {
			$val .= '--';
		} else {
			$val .= str_pad($_POST[$this->id.'value']['h'], 2, '0', STR_PAD_LEFT);
		}

		$val .= ':';

		if (strpos($show, 'i') === false) {
			$val .= '--';
		} else {
			$val .= str_pad($_POST[$this->id.'value']['i'], 2, '0', STR_PAD_LEFT);
		}

		$val .= ':';

		if (strpos($show, 's') === false) {
			$val .= '--';
		} else {
			$val .= str_pad($_POST[$this->id.'value']['s'], 2, '0', STR_PAD_LEFT);
		}

		$this->active_submission[$this->id] = $val;

		return true;

	}//end populate()


}//end class
?>
