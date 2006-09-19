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
* $Id: form_question_type_datetime.inc,v 1.3.2.3 2006/09/19 02:38:52 rong Exp $
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
* @version $Revision: 1.3.2.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
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
	* generates the html for this question to display it in the frontend
	*
	* @access public
	* @return string
	*/
	function getHtmlField()
	{
		$name   = 'q'.$this->id;
		$html = '';
		$value = $this->getValue();

		$elements = Array('day' => 'd', 'month' => 'm', 'year' => 'y', 'hour' => 'h', 'min' => 'i', 'sec' => 's');

		$parameters = Array();
		$parameters['show'] = Array();
		$parameters['null'] = Array();
		$parameters['style'] = Array();
		$parameters['min'] = $this->attr('minimum');
		if ($parameters['min'] == '---------- --:--:--') {
			$parameters['min'] = '1900-01-01 00:00:00';
		}
		$parameters['max'] = $this->attr('maximum');
		if ($parameters['max'] == '---------- --:--:--') {
			$parameters['max'] = '2099-12-31 23:59:59';
		}
		$parameters['extras'] = $this->attr('extras');

		$show_params = $this->attr('show');
		$text_params = $this->attr('text');
		$null_params = $this->attr('null');

		$parameters['allow_circa'] = $show_params['c'];

		foreach ($elements as $attr => $t) {
			$parameters['style'][$t] = ($text_params[$t] ? 't' : 's');
			if ($show_params[$t]) $parameters['show'][] = $t;
			// we will allow null if this field is required then check for blank value
			$parameters['null'][] = $t;
		}

		$dt_field =& new Datetime_Field($name, $value, $parameters);

		ob_start();
		$dt_field->printField();
		$html .= ob_get_contents();
		ob_end_clean();

		// so we need to convert them back to proper php tags
		return html_entity_decode($html);

	}//end getHtmlField()


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
	* Look at user-specified and internal rules to see if this question currently has a valid value
	*
	* @access public
	* @return boolean
	*/
	function hasValidValue()
	{
		$answer = $this->getValue();
		if (!is_iso8601($answer)) return false;
		return parent::hasValidValue();

	}//end hasValidValue()


	/**
	* returns the answer for this question for the summary, generally overriden by children
	*
	* @access public
	* @return string
	*/
	function getSummary()
	{


		$allow_circa = false;
		$show_params = $this->attr('show');
		if ($show_params['c']) $allow_circa = true;

		if ($allow_circa) {
			list($date, $time, $circa) = explode(' ', $this->getValue());
		} else {
			list($date, $time) = explode(' ', $this->getValue());
			$circa = 0;
		}

		$val = Array();

		// break apart the date into correct portions
		list($val['y'], $val['m'], $val['d']) = sscanf($date, '%04s-%02s-%02s');
		list($val['h'], $val['i'], $val['s']) = sscanf($time, '%02s:%02s:%02s');

		// if blank, then fill date up (using bits and pieces from the date
		// 2000-01-01 00:00:00 - year 2000 so leap year dates appear properly)
		foreach($val as $val_key => $val_time) {
			if (preg_match('|^-*$|', $val[$val_key])) {
				switch($val_key) {
					case 'y':
						$val[$val_key] = 2000;
					break;

					case 'm':
					case 'd':
						$val[$val_key] = 1;
					break;

					case 'h':
					case 'i':
					case 's':
						$val[$val_key] = 0;
					break;
				}
			}
		}

		// Extra operation is required to handle UNIX Timestamp issues of mktime() function.
		// Simply we don't want to pass the year to caculate the UNIX timestamp
		$matches = Array();
		$new_format = $this->attr('print_format');
		preg_match_all('/y|Y/', $new_format, $matches, PREG_OFFSET_CAPTURE);

		if (!empty($matches)) {
			$matches = array_reverse(array_pop($matches));
			foreach ($matches as $match) {
				$pos = $match[1];
				$year_str = ($match[0] == 'Y') ? $val['y'] : substr($val['y'], -2);
				if ($pos == 0) {
					$new_format = $year_str.substr($new_format, 1);
				} else if (($pos > 0) && (substr($new_format, ($pos - 1), 1) != "\\")) {
					$new_format = substr($new_format, 0, $pos).$year_str.substr($new_format, $pos + 1);
				}
			}
		}
		$temp_date = mktime($val['h'], $val['i'], $val['s'], $val['m'], $val['d'], 2000);
		return ($circa ? 'circa ' : '').date($new_format, $temp_date);

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
		if ($show_params['c']) $allow_circa = true;

		if ($allow_circa) {
			list($date, $time, $circa) = explode(' ', $answer);
		} else {
			list($date, $time) = explode(' ', $answer);
			$circa = 0;
		}

		echo '<datetime_q id="'.addslashes($this->id).'"  name="'.htmlspecialchars($this->attr('name')).'"'.($circa ? ' circa="1"' : '').'>';

		$iso = $date.'T'.$time;
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
		$name = 'q'.$this->id;
		$value = $_POST[$name.'value'];
		$show = $_POST[$name.'show'];

		if (strpos($show, 'y') === false) {
			$val .= '----';
		} else {
			$val .= str_pad($_POST[$name.'value']['y'], 4, '0', STR_PAD_LEFT);
		}

		$val .= '-';

		if (strpos($show, 'm') === false) {
			$val .= '--';
		} else {
			$val .= str_pad($_POST[$name.'value']['m'], 2, '0', STR_PAD_LEFT);
		}

		$val .= '-';

		if (strpos($show, 'd') === false) {
			$val .= '--';
		} else {
			$val .= str_pad($_POST[$name.'value']['d'], 2, '0', STR_PAD_LEFT);
		}

		$val .= ' ';

		if (strpos($show, 'h') === false) {
			$val .= '--';
		} else {
			$val .= str_pad($_POST[$name.'value']['h'], 2, '0', STR_PAD_LEFT);
		}

		$val .= ':';

		if (strpos($show, 'i') === false) {
			$val .= '--';
		} else {
			$val .= str_pad($_POST[$name.'value']['i'], 2, '0', STR_PAD_LEFT);
		}

		$val .= ':';

		if (strpos($show, 's') === false) {
			$val .= '--';
		} else {
			$val .= str_pad($_POST[$name.'value']['s'], 2, '0', STR_PAD_LEFT);
		}

		$this->setValue($val);

		return true;

	}//end populate()


}//end class
?>
