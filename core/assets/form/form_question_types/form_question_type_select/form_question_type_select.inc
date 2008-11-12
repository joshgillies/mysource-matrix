<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: form_question_type_select.inc,v 1.16 2008/11/12 06:55:44 lwright Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question/form_question.inc';
require_once SQ_FUDGE_PATH.'/csv/csv.inc';


/**
* Form_Question_Type_Select
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.16 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Type_Select extends Form_Question
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	* @param array	$data		the section's entry for this question (contains all question data)
	*
	*/
	function Form_Question_Type_Select($assetid=0,$data=Array())
	{
		$this->Form_Question($assetid,$data);

	}//end constructor


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
		return Array('select_limit', 'selection');

	}//end getAllowedRules()


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
	* generates the html for this question to display it in the frontend or backend
	*
	* @return string
	* @access public
	*/
	function getHtmlField()
	{
		$name = 'q'.$this->id;

		// prepare a valid value for the field id
		$extras = 'id="'.str_replace(':', '_', $name).'" '.$this->attr('extra');

		if ($this->attr('tabindex')) {
			$extras .= ' tabindex="'.$this->attr('tabindex').'"';
		}

		ob_start();
		combo_box($name, $this->getOptions(), $this->attr('multiple'), $this->getValue(), $this->attr('height'), $extras);
		$html = ob_get_contents();
		ob_end_clean();

		return $html;

	}//end getHtmlField()


	/**
	* returns the options currently in the dropdown list
	*
	* @return array
	* @access public
	*/
	function getOptions()
	{
		$options = $this->attr('options');

		if (count($options) > 0) {
			$decoded_options = Array();
			// remove empty options
			foreach (array_keys($options) as $k) {
				//$options[$k] = html_entity_decode($options[$k]);
				if ($options[$k] != '') {
					$decoded_options[html_entity_decode($k)] = html_entity_decode($options[$k]);
				}
			}
			$options = $decoded_options;

			// if allow empty is true, add the '-- Leave Empty --' as the first option
			if ($this->attr('allow_empty')) {
				$empty = Array(html_entity_decode($this->attr('empty_key')) => html_entity_decode($this->attr('empty_text')));
				$options = array_merge($empty, $options);
			}
		}

		return $options;

	}//end getOptions()


	/**
	* returns a summary for this question
	*
	* @param string	$answer	The answer the print the summary for (optional)
	*
	* @return mixed array|string
	* @access public
	*/
	function getSummary($answer=NULL)
	{
		if (is_null($answer)) $answer = $this->value;

		if (is_array($answer)) {
			$ans_str = Array();
			foreach ($answer as $a) {
				// if the option is '-- Leave Empty --', we should return empty string
				$answer_by_offset = $this->getAnswerByOffset($a);
				if ($answer_by_offset != $this->attr('empty_text')) {
					$ans_str[] = $this->getAnswerByOffset($a);
				} else {
					$ans_str[] = '';
				}
			}
			return implode(', ', $ans_str);
		}

		// if the option is '-- Leave Empty --', we should return empty string
		$answer_by_offset = $this->getAnswerByOffset($answer);
		if ($answer_by_offset != $this->attr('empty_text')) {
			return $this->getAnswerByOffset($answer);
		} else {
			return '';
		}

	}//end getSummary()


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

		echo '<select_q id="'.addslashes($this->id).'"  name="'.htmlentities($this->attr('name')).'">';
		if (!is_array($answer)) $answer = Array($answer);

		foreach ($answer as $a) {
			echo '<option value="'.$a.'">'.htmlentities($this->getAnswerByOffset($a)).'</option>';
		}
		echo '</select_q>';

		$contents = ob_get_contents();
		ob_end_clean();

		return $contents;

	}//end getXML()


	/**
	* returns TRUE if this particuar option is selected
	*
	* @param string	$option	the value to check
	*
	* @return boolean
	* @access public
	*/
	function isSelected($option)
	{
		$options = $this->getValue();
		if (!is_array($options)) {
			$options = Array($options);
		}

		return in_array($option, $options);

	}//end isSelected()


	/**
	* returns TRUE if this particuar option is an --Leave Empty-- option
	*
	* @param string	$option	the value to check
	*
	* @return boolean
	* @access public
	*/
	function isEmptyOption($option)
	{
	$option_name = $this->getAnswerByOffset($option);
		return $option_name == $this->attr('empty_text');

	}//end isEmptyOption()


	/**
	* Get option name
	*
	* Get the name for one particular option, specified by option code.
	* Assumes option id is valid
	*
	* @param string	$option_code	unique option id for this question
	*
	* @return array
	* @access public
	*/
	function getOptionName($option_code)
	{
		return $this->getAnswerByOffset($option_code);

	}//end getOptionName()


	/**
	* Replace options with an imported CSV file.
	*
	* If 1 column: value only - key is sequentially assigned (from 0)
	* If 2 columns: key, value
	* If 3 columns: key, value, default [1|0]
	*
	* @param string	$filename	The (temp) file name to import
	*
	* @return void
	*/
	public function importOptionsFromCSV($filename)
	{
		$csv = new CSV($filename);
		$csv->import();

		$values = $csv->values;
		$options = Array();
		$defaults = Array();

		if (!empty($values)) {
			// We will assume that the number of columns in the first row
			// is representative of the entire list
			$num_columns = count($values[0]);
			$index = 0;

			foreach ($values as $row) {
				if ($num_columns === 1) {
					// If we haven't provided a first column, then key and value
					// are the same
					$value = htmlentities($row[0], ENT_NOQUOTES);
					$options[$index] = $value;
					$index++;
				} else {
					$key = htmlentities($row[0], ENT_NOQUOTES);
					$value = htmlentities($row[1], ENT_NOQUOTES);
					$options[$key] = $value;

					if ($num_columns === 3) {
						if ($row[2] == 1) {
							$defaults[] = $key;
						}
					}
				}
			}

			$this->setAttrValue('options', $options);
			if ($num_columns === 3) {
				if (!$this->attr('multiple')) {
					if (count($defaults) > 1) {
						trigger_localised_error('CORE0286', E_USER_NOTICE);
					}
					if (count($defaults) > 0) {
						$defaults = Array($defaults[0]);
					} else {
						$defaults = Array();
					}
				}

				$this->setAttrValue('default', $defaults);
			} else {
				// What happens if it gets overwritten?
				$current_default = $this->attr('default');
				if (!is_array($current_default)) {
					$current_default = Array($current_default);
				}
				foreach ($current_default as $default_key) {
					// Remove any default options that are not choices in this
					// particular situation
					if (!isset($options[$default_key])) {
						unset($current_default[$default_key]);
					}
				}
				$this->setAttrValue('default', $current_default);
			}
			$this->saveAttributes();
		}

	}//end importOptionsFromCSV()


}//end class
?>
