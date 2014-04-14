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
* $Id: form_question_type_datetime.inc,v 1.29 2013/07/25 23:23:48 lwright Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question/form_question.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';

/**
* Form_Question_Type_Text
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.29 $
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
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		return Array();

	}//end _getAllowedLinks()


	/**
	* generates the html for this question to display it in the frontend
	*
	* @return string
	* @access public
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

		if (($parameters['min'] == '---------- --:--:--') || ($parameters['min'] == '0001------ --:--:--')) {
			$parameters['min'] = '1900-01-01 00:00:00';
		}
		$parameters['max'] = $this->attr('maximum');

		if (($parameters['max'] == '---------- --:--:--') || ($parameters['max'] == '0001------ --:--:--')) {
			$parameters['max'] = '2099-12-31 23:59:59';
		}
		$parameters['extras'] = $this->attr('extras');

		if ($this->attr('tabindex')) {
			$tabindex = ' tabindex="'.$this->attr('tabindex').'"';
			foreach ($parameters['extras'] as $key => $extra) {
				$parameters['extras'][$key] .= $tabindex;
			}
		}

		$show_params = $this->attr('show');
		$text_params = $this->attr('text');
		$null_params = $this->attr('null');

		$parameters['allow_circa'] = $show_params['c'];
		if (!empty($show_params['a'])) $parameters['show'][] = 'a';

		foreach ($elements as $attr => $t) {
			$parameters['style'][$t] = ($text_params[$t] ? 't' : 's');
			if ($show_params[$t]) $parameters['show'][] = $t;
			// we will allow null if this field is required then check for blank value
			$parameters['null'][] = $t;
		}

		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_formid);
		if($form->useAccessibleFormat()) {
		    $parameters['print_label'] = TRUE;
		}
		// don't print duplicate JS inclusions
		$include_js = TRUE;
		if(isset($form->_tmp['datetime_js_files_included'])) {
		    $include_js = FALSE;
		}
		if($include_js) {
		    $form->_tmp['datetime_js_files_included'] = TRUE;
		    $parameters['include_js'] = TRUE;
		}
		// use js validation or not
		$parameters['use_js'] = $this->attr('use_js');
		$dt_field = new Datetime_Field(str_replace(':', '_', $name).'_', $value, $parameters);

		ob_start();
		$dt_field->printField();
		$html .= ob_get_contents();
		ob_end_clean();

		// so we need to convert them back to proper php tags
		return $html;

	}//end getHtmlField()


	/**
	* returns a set of default parameters for the datatime_field class
	*
	* @return array
	* @access public
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
		$parameters['show']['a'] = 0;

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
	* @return array
	* @access public
	*/
	function getAllowedRules()
	{
		return Array();

	}//end getAllowedRules()


	/**
	* Look at user-specified and internal rules to see if this question
	* currently has a valid value
	*
	* Returns TRUE if all validation rules pass
	*
	* @param mixed		$answer			Custom value to compare with - if left
	*									NULL (or unspecified), use the current
	*									value of the question
	* @param boolean	$mute_errors	If TRUE, do not add to the "Failed
	*									Rules" log, but simply return FALSE
	*
	* @return boolean
	* @access public
	*/
	function hasValidValue($answer=NULL, $mute_errors=FALSE)
	{
		if (is_null($answer)) $answer = $this->getValue();

		$required_fields = $this->attr('null');
		$show_params = $this->attr('show');

		$allow_circa = $show_params['c'];
		if ($allow_circa) {
			// Get rid of the "circa" bit while validating the date
			$answer = preg_replace('/(.*?) [0|1]$/', '$1', $answer);
		}

		$custom_error = $this->attr('cust_required_error');
		if (empty($custom_error)) {
			$custom_error = translate('core_form_datetime_invalid_date_time');
		}

		$this->failed_rules[] = $custom_error;

		if (!is_iso8601($answer)) return FALSE;

		// Ensure that required fields are filled in
		foreach ($required_fields as $field => $required) {
			if ($required != '1' && $show_params[$field] == '1') {
				if ($field == 'y' && (substr($answer, 0, 2) == '--')) {
					return FALSE;

				} else if ($field == 'm' && (substr($answer, 5, 2) == '--')) {
					return FALSE;

				} else if ($field == 'd' && (substr($answer, 8, 2) == '--')) {
					return FALSE;

				} else if ($field == 'h' && (substr($answer, 11, 2) == '--')) {
					return FALSE;

				} else if ($field == 'i' && (substr($answer, 14, 2) == '--')) {
					return FALSE;

				} else if ($field == 's' && (substr($answer, 17, 2) == '--')) {
					return FALSE;

				}
			}
		}

		// At this stage the date is in ISO 8601 format, however we still need to check value limits (eg; to avoid 30th Feb)
		$date = Array();
		list($date['y'], $date['m'], $date['d']) = sscanf($answer, '%04s-%02s-%02s');

		// For checking purposes, set year, month, and day defaults for each value if they were not mandatory and were not specified. Feb 29th will be allowed if the year is not specified
		foreach ($required_fields as $field => $required) {
			if ($required == '1' || $show_params[$field] != '1') {
				if ((($field == 'y') || ($field == 'm') || ($field == 'd')) && (substr($date[$field], 0, 2) == '--')) {
					if ($field == 'y') {
						$date['y'] = 2000;
					} else {
						$date[$field] = 1;
					}
				}
			}
		}
		$is_valid_date = checkdate($date['m'], $date['d'], $date['y']);

		if (!$is_valid_date) return FALSE;

		return parent::hasValidValue($answer, $mute_errors);

	}//end hasValidValue()


	/**
	* returns the answer for this question for the summary, generally overriden by children
	*
	* @return string
	* @access public
	*/
	function getSummary()
	{
		$allow_circa = FALSE;
		$show_params = $this->attr('show');
		if ($show_params['c']) $allow_circa = TRUE;

		if ($allow_circa) {
			$value_array = explode(' ', $this->getValue());
			$date = $value_array[0];
			$time = $value_array[1];
			$circa = isset($value_array[2]) ? $value_array[2] : 0;
		} else {
			list($date, $time) = explode(' ', $this->getValue());
			$circa = 0;
		}
		$val = Array();

		// break apart the date into correct portions
		list($val['y'], $val['m'], $val['d']) = sscanf($date, '%04s-%02s-%02s');
		list($val['h'], $val['i'], $val['s']) = sscanf($time, '%02s:%02s:%02s');

		$matches = Array();
		$new_format = $this->attr('print_format');

		$required_fields = $this->attr('null');
		
		// if blank, then fill date up (using bits and pieces from the date
		// 2000-01-01 00:00:00 - year 2000 so leap year dates appear properly)
		$non_empty_date = TRUE;

		foreach ($val as $val_key => $val_time) {
			$preg_match_result = preg_match('|^-*$|', $val[$val_key]);
			if ($required_fields[$val_key] != '1' || !$preg_match_result) {
				$non_empty_date = FALSE;
			}//end if
			if ($preg_match_result) {
				switch ($val_key) {
					case 'y':
						// If Year is allowed empty,  set new_format to [empty],xx,xx
						if ($required_fields[$val_key] == 1) {
							$new_format = $this->replaceDateFormat($new_format, $val_key);
						}
						$val[$val_key] = 2000;
					break;

					case 'm':
						// If Month is allowed empty, set new_format to xxxx,[empty],xx
					case 'd':
						// If Date is allowed empty,  set new_format to xxxx,xx,[empty]
						if ($required_fields[$val_key] == 1) {
							$new_format = $this->replaceDateFormat($new_format, $val_key);
						}
						$val[$val_key] = 1;
					break;

					case 'h':
					case 'i':
					case 's':
						if ($required_fields[$val_key] == 1) {
							$new_format = $this->replaceDateFormat($new_format, $val_key);
						}
						$val[$val_key] = 0;
					break;
				}
			}//end if preg_match_result
		}//end foreach

		// If all six date fields is allow blank and they are blank, then return blank string.
		if ($non_empty_date === TRUE) return '';
		
		// Flag set when supplied year is out of range 1970 - 2038 range
		$invalid_year = FALSE;
		$temp_date = mktime($val['h'], $val['i'], $val['s'], $val['m'], $val['d'], $val['y']);
		
		// If the supplied year doesn't returns the timestamp or date is not within range, use the year 2000
		if ($temp_date === FALSE || $val['y'] < 1970 || $val['y'] > 2038) {
			$temp_date = mktime($val['h'], $val['i'], $val['s'], $val['m'], $val['d'], 2000);
			$invalid_year = TRUE;
		}
		
		// Extra operation is required to handle UNIX Timestamp issues of mktime() function.
		// Simply we don't want to pass the year to calculate the UNIX timestamp when year is not within range

		preg_match_all('/y|Y/', $new_format, $matches, PREG_OFFSET_CAPTURE);
		if (!empty($matches) && $invalid_year) {
			$matches = array_reverse(array_pop($matches));
			foreach ($matches as $match) {
				$pos = $match[1];
				$year_str = ($match[0] == 'Y') ? $val['y'] : substr($val['y'], -2);
				if ($pos == 0) {
					$new_format = $year_str.substr($new_format, 1);
				} else if (($pos > 0) && (substr($new_format, ($pos - 1), 1) != '\\')) {
					$new_format = substr($new_format, 0, $pos).$year_str.substr($new_format, $pos + 1);
				}
			}
		}
		
		return ($circa ? 'circa ' : '').date($new_format, $temp_date);

	}//end getSummary()


	/**
	* If this function is called, it will replace the format of a date element (y,m,d,h,i,s) with blank string
	* It is only used when a date element is allowed blank and was submitted with blank value.
	*
	* @return string
	* @return public
	*/
	function replaceDateFormat($original_format, $element)
	{
		$replacement_mapping = Array (
								'y'	=> Array ('y', 'Y'),
								'm'	=> Array ('m', 'n', 'F', 'M'),
								'd' => Array ('d', 'j', 'S', 'D', 'l'),
								'h'	=> Array ('h', 'H', 'g', 'G'),
								'i' => Array ('i'),
								's'	=> Array ('s'),
							   );
		// Replace with blank string
		if (isset($replacement_mapping[$element])) {
			foreach ($replacement_mapping[$element] as $replace_string) {
				$original_format = str_replace($replace_string, '', $original_format);
			}//end foreach
		}//end if

		return $original_format;

	}//end replaceDateFormat()


	/**
	* returns an answer summary in XML format (suitable for submission logs)
	*
	* @param mixed	$answer	the answer for the question
	*
	* @return string
	* @access public
	*/
	function getXML($answer)
	{
		ob_start();

		$allow_circa = FALSE;

		$show_params = $this->attr('show');
		if ($show_params['c']) $allow_circa = TRUE;

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
		$name = str_replace(':', '_', 'q'.$this->id).'_';
		$value = array_get_index($_POST, $name.'value', '');
		$show = array_get_index($_POST, $name.'show', '');

		if (strpos($show, 'y') === FALSE) {
			$val .= '----';
		} else {
			$val .= str_pad($_POST[$name.'value']['y'], 4, '-', STR_PAD_LEFT);
		}

		$val .= '-';

		if ((isset($_POST[$name.'value']['m']) && empty($_POST[$name.'value']['m'])) || strpos($show, 'm') === FALSE) {
			$val .= '--';
		} else {
			$val .= str_pad($_POST[$name.'value']['m'], 2, '0', STR_PAD_LEFT);
		}

		$val .= '-';

		if ((isset($_POST[$name.'value']['d']) && empty($_POST[$name.'value']['d'])) || strpos($show, 'd') === FALSE) {
			$val .= '--';
		} else {
			$val .= str_pad($_POST[$name.'value']['d'], 2, '0', STR_PAD_LEFT);
		}

		$val .= ' ';

		if ((isset($_POST[$name.'value']['h']) && empty($_POST[$name.'value']['h'])) || strpos($show, 'h') === FALSE) {
			if (isset($_POST[$name.'value']['h']) && (string) $_POST[$name.'value']['h'] === '0') {
				$val .= '00';
			} else {
				$val .= '--';
			}
		} else {
			$hour = str_pad($_POST[$name.'value']['h'], 2, '0', STR_PAD_LEFT);
			if (!empty($_POST[$name.'value']['a']) && strpos($show, 'a') !== FALSE && $hour != '--'){
				$hour = date('H', strtotime($hour.' '.$_POST[$name.'value']['a']));
			}
			$val .= $hour;
		}

		$val .= ':';

		if ((isset($_POST[$name.'value']['i']) && empty($_POST[$name.'value']['i'])) || strpos($show, 'i') === FALSE) {
			if (isset($_POST[$name.'value']['i']) && (string) $_POST[$name.'value']['i'] === '0') {
				$val .= '00';
			} else {
				$val .= '--';
			}
		} else {
			$val .= str_pad($_POST[$name.'value']['i'], 2, '0', STR_PAD_LEFT);
		}

		$val .= ':';

		if ((isset($_POST[$name.'value']['s']) && empty($_POST[$name.'value']['s'])) || strpos($show, 's') === FALSE) {
			if (isset($_POST[$name.'value']['s']) && (string) $_POST[$name.'value']['s'] === '0') {
				$val .= '00';
			} else {
				$val .= '--';
			}
		} else {
			$val .= str_pad($_POST[$name.'value']['s'], 2, '0', STR_PAD_LEFT);
		}

		if (isset($_POST[$name.'value']['c'])) {
			// '1' means allow circa, '0' means no
			$val .= ((string) $_POST[$name.'value']['c'] === '1') ? ' 1' : ' 0';
		}

		$this->setValue($val);

		return TRUE;

	}//end populate()


	/**
	* Use fieldset tag to group question content?
	*
	* @return bool
	* @access public
	*/
	function useFieldset()
	{
		return TRUE;
	}//end useFieldset()
	
	
	/**
	* Get the HTML label for this form question
	*
	* @return boolean
	* @access public
	*/
	function getHtmlLabel()
	{
		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_formid);
		if($form->useAccessibleFormat()) {
		    $required_mark = '';
		    $is_required = FALSE;
		    $required_fields = $this->attr('null');
			$show_params = $this->attr('show');
			foreach ($required_fields as $field => $required) {
				if ($required != '1' && $show_params[$field] == '1') {
					$is_required = TRUE;
				}
			}
		    if($is_required) {
			$required_mark = $form->getRequiredFieldMark();
		    }
		    return '<legend class="sq-form-question-title">'.$this->attr('name').$required_mark.'</legend>';
		}
		else {
		    return $this->attr('name');
		}

	}//end getHtmlLabel()
	
	
}//end class
?>
