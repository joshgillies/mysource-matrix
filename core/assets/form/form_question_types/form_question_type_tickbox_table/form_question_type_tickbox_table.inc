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
* $Id: form_question_type_tickbox_table.inc,v 1.4 2006/06/19 01:03:45 lwright Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question/form_question.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';


/**
* Form_Question_Type_Tickbox_List
*
* Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Type_Tickbox_Table extends Form_Question
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	* @param array	$data		the section's entry for this question (contains all question data)
	*
	*/
	function Form_Question_Type_Tickbox_Table($assetid=0,$data=Array())
	{
		$this->Form_Question($assetid,$data);

	}//end constructor


	/**
	* generates the html for this question to display it in the frontend
	*
	* @access public
	* @return string
	*/
	function getHtmlField()
	{
		return $this->_getOutput('active');

	}//end getHtmlField()


	/**
	* returns a summary for this question
	*
	* @access public
	* @return mixed
	*/
	function getSummary()
	{
		return $this->_getOutput('plaintext');

	}//end getSummary()


	/**
	* Processes the question and returns its "view" depending on type
	*
	* @param string $type view type - defined in xml2html.xsl
	*
	* @access public
	* @return string
	*/
	function _getOutput($type='plaintext')
	{
		$prefix = $this->getSafePrefix();

		$question_contents = $this->attr('question_contents');
		$datapath = sq_web_path('data').'/asset_types/form_question_type_tickbox_table/files';

		$form =& $this->getMySection();
		$submission = $this->getValue();

		$to_output = '';

		switch ($type)
		{
			// plaintext part for e-mail sending and submission confirmation
			case 'plaintext':
				if (empty($question_contents )) return '';
				$header = array_shift($question_contents);
				$title  = array_shift($header);
				$output = Array();

				if (!empty($question_contents)) {
					foreach ($question_contents as $question_id=>$question) {
						$question_title = array_shift($question);
						$this_question = Array();
						foreach ($question as $option_id=>$option) {
							if ($submission && isset($submission[$question_id][$option_id])) {
								$this_question[] = $header[$option_id].' ('.$option.')';
							}
						}
						if (!empty($this_question)) {
							$output[] = $question_title.': '.implode(', ', $this_question);
						}
					}
				}
				return implode("; \n", $output);
			break;

			// editor part for dynamic JavaScript page
			case 'editor':
				$header = array_shift($question_contents);
				$to_output =	'<style type="text/css">@import url("'.$datapath.'/style.css");</style>'.
								'<script type="text/javascript" src="'.$datapath.'/InputTable.js">//blank</script>'.
								'<table border="0" cellspacing="0" cellpadding="0" id="'.$prefix.'" class="tbl"></table>'.
								'<script type="text/javascript">'.
								"var Quests = Array(\n";
				$output = Array();
				if (!empty($header)) {
					array_walk($header, 'escape_double_quotes');
					$output[] = '	Array("'.implode('", "', $header).'")';
					foreach ($question_contents as $question_id=>$question) {
						array_walk($question, 'escape_double_quotes');
						$output[] = '	Array("'.implode('", "', $question).'")';
					}
					$to_output .= implode(",\n", $output);
				}
				$to_output .= ");\n";
				$to_output .= 	'	var tb = new InputTable("'.$prefix.'", "tb");'.
								'	tb.datapath = "'.$datapath.'";'.
								'	tb.setup(Quests);'.
								'</script>';
				return $to_output;
			break;

			// active and passive part for front-end and unlocked back-end view
			default:
				$to_output =	'<table border="0" cellpadding="0" cellspacing="0" class="sq-backend-table" id="'.$prefix.'">'.
								'	<tr>'.
								'		<td class="sq-backend-table-header">';
				if (empty($question_contents) || empty($question_contents[0])) {
					$to_output .= '<div align="center">'.translate('core_form_tickbox_table_not_properly_defined');
					if ($type == 'passive') {
						$to_output .= ' '.translate('core_form_tickbox_table_incorrectly_displayed');
					}
					$to_output .= '</div>';
				} else {
					$header = array_shift($question_contents);
					$to_output .= array_shift($header).'</td>';
					foreach ($header as $key=>$title) {
						$to_output .= '<td class="sq-backend-table-header">'.$title.'</td>';
					}
					$to_output .= "</tr>\n";
					$j = 0;
					foreach ($question_contents as $question_id=>$question) {
						$j++;
						$to_output .= '<tr';
						if ($j % 2 == 0) $to_output .= ' class="even"';
						$to_output .= '>';

						$to_output .= '<td class="sq-backend-table-header">'.array_shift($question).'</td>';
						foreach ($question as $option_id=>$option) {
							$to_output .= '<td class="sq-backend-table-cell">';
							if (empty($option)) {
								$to_output .= '&mdash;';
							} else if ($type != 'passive') {
								ob_start();
									check_box($prefix.'['.$question_id.']['.$option_id.']', '1', $submission && isset($submission[$question_id][$option_id]));
									$to_output .= ob_get_contents();
								ob_end_clean();
								$to_output .= ' <span onclick="this.previousSibling.previousSibling.checked = !this.previousSibling.previousSibling.checked">'.$option.'</span>';
							} else {
								$to_output .= $option;
							}
							$to_output .= '</td>';
						}
						$to_output .= '</tr>';
					}
				}
				$to_output .= '</table>';

		}
		return $to_output;

	}//end _getOutput()


	/**
	* Look at user-specified and internal rules to see if this question currently has a valid value
	*
	* @access public
	* @return boolean
	*/
	function hasValidValue($answer)
	{
		if (is_null($answer)) {
			$answer = $this->getValue();
		}

		if ($this->attr('is_required')) {
			$req_rule = Array('rule_code' => 'select_limit', 'operator' => '>=', 'value' => '1');
			array_push($this->vars['rules']['value'], $req_rule);
		}

		$ok = parent::hasValidValue($answer);

		if ($this->attr('is_required')) {
			array_pop($this->vars['rules']['value']);
		}

		return $ok;

	}//end hasValidValue()


	/**
	* Assemble an XML representation of the question
	*
	* @access public
	* @return string
	*/
	function getXMLstring()
	{
		$prefix = $this->getSafePrefix();

		$question_contents = $this->attr('question_contents');
		$datapath = sq_web_path('data').'/asset_types/form_question_type_tickbox_table/files';
		$submission = array_get_index($this->_tmp,'submission',false);
		$to_output = '<question>';
		$to_output .= '<prefix><![CDATA['.$prefix.']]></prefix>';
		$to_output .= '<datapath><![CDATA['.$datapath.']]></datapath>';

		if (empty($question_contents )) {
			$to_output .= '</question>';
		} else {
			$header = array_shift($question_contents);
			$title  = array_shift($header);
			$to_output .= '<title><![CDATA['.$title.']]></title>';
			$to_output .= '<optiontitles>';
			foreach ($header as $key=>$item) {
				$to_output .= '<title id="'.$key.'"><![CDATA['.$item.']]></title>';
			}
			$to_output .= '</optiontitles>';

			foreach ($question_contents as $question_id=>$question) {
				$question_title = array_shift($question);
				$to_output .= '<subquestion id="'.$question_id.'">';
				$to_output .= '<title><![CDATA['.$question_title.']]></title>';

				$to_output .= '<options>';
				foreach ($question as $option_id=>$option) {
					$checked = 'checked="0"';
					if ($submission && isset($submission[$question_id][$option_id])) {
						$checked = 'checked="1"';
					}
					$to_output .= '<option id="'.$option_id.'" '.$checked.'><![CDATA['.$option.']]></option>';
				}
				$to_output .= '</options>';
				$to_output .= '</subquestion>';
			}
			$to_output .= '</question>';
		}

		return $to_output;

	}//end getXMLstring()


	/**
	* Create HTML and Javascript safe prefix
	*
	* @access public
	* @return string
	*/
	function getSafePrefix()
	{
		return $prefix = 'q'.str_replace(':', '_', $this->getPrefix());

	}//end getSafePrefix()


	/**
	* Specifies if this question is of type Selection
	*
	* @access public
	* @return boolean
	*/
	function isSelection()
	{
		return true;

	}//end isSelection()


	/**
	* returns TRUE if this particuar tickbox is selected
	*
	* @param int $value the value to check
	*
	* @access public
	* @return boolean
	*/
	function isSelected($option_code)
	{
		$value = $this->getValue();
		if (empty($value)) return false;

		// decipher the option id
		list($line,$option) = explode('_', $option_code);

		// check if the option was submitted
		return !empty($value[$line][$option]);

	}//end isSelected()


	/**
	* Get option name
	*
	* Get the name for one particular option, specified by option code.
	* Assumes option id is valid
	*
	* @param string $option_code    unique option id for this question
	*
	* @access public
	* @return Array
	*/
	function getOptionName($option_code)
	{
		$question_contents = $this->attr('question_contents');

		// decipher the option id
		list($line_id,$option_id) = explode('_', $option_code);

		$header = array_shift($question_contents);
		//first column of first row is of no importance
		array_shift($header);

		$line   = $question_contents[$line_id];
		$title  = array_shift($line);

		$option = $header[$option_id];

		$name   = $title.': '.$option.' - ('.$line[$option_id].')';

		return $name;

	}//end getOptionName()


	/**
	* Get Option Array
	*
	* @access public
	* @return Array()
	*/
	function getOptions()
	{
		$options = Array();
		$lines = $this->attr('question_contents');
		$header = array_shift($lines);
		// remove the first option - it does not carry any value
		array_shift($header);

		foreach ($lines as $id => $line) {
			$title = array_shift($line);
			foreach ($header as $o_id => $o_name) {
				$option_id = $id.'_'.$o_id;
				$options[$option_id] = $title.': '.$o_name.' - ('.$line[$o_id].')';
			}
		}

		return $options;

	}//end getOptions()


	function populate()
	{
		$prefix = $this->getSafePrefix();
		$this->setValue(array_get_index($_POST, $prefix, Array()));
		return true;

	}//end populate()


}//end class
?>
