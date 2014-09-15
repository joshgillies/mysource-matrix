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
* $Id: online_quiz_question_multichoice.inc,v 1.13 2012/08/30 00:58:09 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_online_quiz/online_quiz_question/online_quiz_question.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';


/**
* Online_Quiz_Question_Multichoice
*
* Purpose
*	A quiz question that can display a list of options for the user to select
*
*	NOTE: It is recommended that a question only be used once per user, even if the question is shared amongst different quizes
*	Simultaneously taking two tests with the same question contained in each will cause both questions to share data
*
*
* @author  Robert Howard <rhoward@squiz.net>
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Online_Quiz_Question_Multichoice extends Online_Quiz_Question
{


	/**
	* Constructor
	*
	* @param string	$assetid	the assetid of the asset to load into this object
	*
	* @return void
	* @access public
	*/
	function Online_Quiz_Question_Multichoice($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Online_Quiz_Question($assetid);

	}//end constructor


	/**
	* Set Attribute Value
	*
	* @param string	$name	name of attribute
	* @param mixed	$value	value of attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value)
	{
		switch ($name) {
			case 'max_option_selections':
				if ($value < 1) $value = 1;
			break;
		}
		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Processes the form using submitted values
	*
	* NOTE: Returns TRUE if the value is valid, otherwise returns FALSE or an array in the following format:
	* <pre>
	* Array(
	* 	'errors'	=> Array('Sample error string'),
	* 	'warnings'	=> Array('Question #12 has not been answered'),
	* )
	* </pre>
	*
	* @return boolean
	* @access public
	*/
	function processResponseForm()
	{
		$prefix = $this->getPrefix();

		switch ($this->attr('option_style')) {
			case 'list':
				if ($this->attr('max_option_selections') <= 1) {
					$value = array_get_index($_REQUEST, $prefix.'_response', NULL);
				} else {
					// Request format: Array(
					// 					0 => option_key,
					// 				   )
					$value = array_get_index($_REQUEST, $prefix.'_response', Array());
				}
			break;

			case 'radio':
				if ($this->attr('max_option_selections') <= 1) {
					$value = array_get_index($_REQUEST, $prefix.'_response', NULL);
				} else {
					// Request format: Array(
					// 					option_key => is_checked
					// 				   )
					$value = array_keys(array_get_index($_REQUEST, $prefix.'_response', Array()));
				}
			break;
		}//end switch (option_style)

		$valid = $this->validateValue($value);
		$errors = array_get_index($valid, 'errors', Array());
		if (empty($errors)) {
			$this->_value = $value;
			$this->saveQuestion();
		}

		// return the error stack
		return $valid;

	}//end processResponseForm()


	/**
	* Determine whether or not this question
	* currently has a valid value
	*
	* NOTE: Returns an array in the following format:
	* <pre>
	* Array(
	* 	'errors'	=> Array('Sample error string'),
	* 	'warnings'	=> Array('Question #12 has not been answered'),
	* )
	* </pre>
	* The 'errors' and 'warnings' keys will always be present, but empty if no errors
	* or warnings need to be returned
	*
	* @param mixed	$value	Custom value to compare with - if left NULL (or unspecified), use the current value of the question
	*
	* @return array
	* @access public
	*/
	function validateValue($value=NULL)
	{
		if (is_null($value)) $value = $this->_value;

		$errors = Array(
					'errors'	=> Array(),
					'warnings'	=> Array(),
				  );

		if (empty($value)) {
			$errors['warnings'][] = sprintf(translate('online_quiz_question_warning_empty_question'), $this->getPosition());
		}

		return $errors;

	}//end validateValue()


	/**
	* Returns the javascript used to validate the question
	*
	* @return string
	* @access public
	*/
	function getValidationJS()
	{
		static $printed_fn = FALSE;

		$prefix = $this->getPrefix();

		ob_start();
			if (!$printed_fn) {
				// the "< echo sprintf(translate('string'), '%s'); >".replace(/%s/, question) is
				// a hack to insert the question number in a translation string
				// without using js_translate (which is unsupported on the frontend)
				?>
					function <?php echo $this->type(); ?>_validate(error_stack, question, answer)
					{
						if (!answer.length) {
							 error_stack["warnings"][error_stack["warnings"].length] = "<?php echo sprintf(translate('online_quiz_question_warning_empty_question'), '%s'); ?>".replace('%s', question);
						}
					}
				<?php
				$printed_fn = TRUE;
			}

			$options = $this->attr('response_form');

			// figure out which option(s) are selected
			?>
			var selected = new Array();
			<?php

			if ($this->attr('option_style') == 'radio') {
				// checkboxes and radio buttons use the same format for their IDs
				foreach ($options as $option_key => $option) {
					?>
					if (document.getElementById("<?php echo $prefix.'_response_'.$option_key; ?>").checked) selected[selected.length] = "<?php echo $option_key; ?>";
					<?php
				}
			} else {
				?>
				select_box = document.getElementById("<?php echo $prefix.'_response'; ?>");
				selected[selected.length] = select_box.selectedIndex;
				<?php
			}
			?>
			<?php echo $this->type(); ?>_validate(error_stack, "<?php echo $this->getPosition(); ?>", selected);
			<?php

		return ob_get_clean();

	}//end getValidationJS()


	/**
	* Returns the current user response in a human-readable format
	*
	* NOTE: Arrays of answers will stay in array format, with the exception
	* that each element of the array will be rendered human-readable
	*
	* @return string
	* @access public
	*/
	function getSummary()
	{
		$value = $this->getValue();
		$options = $this->attr('response_form');

		if ($this->attr('max_option_selections') <= 1) {
			$selected_option = array_get_index($options, $value, Array());
			return array_get_index($selected_option, 'text', '');
		} else {
			$summary = Array();
			if (!empty($value)) {
				foreach ($value as $response) {
					$selected_option = array_get_index($options, $response, Array());
					$summary[]       = array_get_index($selected_option, 'text', '');
				}
			}
			return $summary;
		}

	}//end getSummary()


	/**
	* Returns the option keys of the correct answers
	*
	* @return array
	* @access public
	*/
	function _getCorrectOptionKeys()
	{
		// figure out which answers return points

		$options = $this->attr('response_form');
		$correct_keys = Array();
		foreach ($options as $option_key => $option) {
			if ($option['points'] > 0) {
				$correct_keys[] = $option_key;
			}
		}

		return $correct_keys;

	}//end _getCorrectOptionKeys()


	/**
	* Returns the correct response
	*
	* @return string
	* @access public
	*/
	function getCorrectValue()
	{
		return $this->_getCorrectOptionKeys();

	}//end getCorrectValue()


	/**
	* Returns the correct response in a human-readable format
	*
	* @return string
	* @access public
	*/
	function getCorrectSummary()
	{
		$options      = $this->attr('response_form');
		$correct_keys = $this->_getCorrectOptionKeys();
		$summary      = Array();

		foreach ($correct_keys as $key) {
			if (!empty($options[$key]['text'])) {
				$summary[] = $options[$key]['text'];
			}
		}

		return $summary;

	}//end getCorrectSummary()


	/**
	* Determine how many points the given value (or the question's current value) is worth
	*
	* @param mixed	$value	the value to use determining the number of points scored - if NULL, use the question's value
	*
	* @return int
	* @access public
	*/
	function getPoints($value=NULL)
	{
		if (is_null($value)) $value = $this->getValue();

		$total_points   = 0;

		$options         = $this->attr('response_form');
		$max_selections  = $this->attr('max_option_selections');
		$forfeit_penalty = $this->attr('forfeit_penalty');

		if (empty($options) || empty($max_selections)) {
			// invalid, nothing to select; bail without docking points
			return 0;
		}

		// if the nothing was selected, apply the forfeit penalty and bail
		if (empty($value)) return ($forfeit_penalty * -1);

		if ($max_selections == 1) {

			// if an invalid answer was selected - dock points as if they hadn't answered at all
			if (!isset($options[$value]['points'])) {
				return ($forfeit_penalty * -1);
			}

			$total_points += $options[$value]['points'];

		} else {

			// make sure only $max_selections or less options have been selected - otherwise
			// dock points as if they hadn't answered at all
			// (this prevents the user just selecting everything and getting the answer right)
			if (count($value) > $max_selections) {
				return ($forfeit_penalty * -1);
			}

			foreach ($options as $option_key => $option) {
				if (in_array($option_key, $value)) {
					$total_points += $option['points'];
				}
			}

		}

		return $total_points;

	}//end getPoints()


	/**
	* Determine how many points are available from this question in total
	*
	* @return int
	* @access public
	*/
	function getAvailablePoints()
	{
		$total_points   = 0;

		$options        = $this->attr('response_form');
		$max_selections = $this->attr('max_option_selections');

		if (empty($options) || empty($max_selections)) {
			// invalid, nothing to select
			return 0;
		}

		// order the questions by points value, then choose the
		// first $this->attr('max_option_selections') and add them up

		$option_points = Array();
		foreach ($options as $option_key => $option) {
			$option_points[] = $option['points'];
		}
		rsort($option_points, SORT_NUMERIC);

		$slice_min = 0;
		$slice_max = min(count($option_points), $max_selections);

		$points_to_count = array_slice($option_points, $slice_min, $slice_max);

		foreach ($points_to_count as $points_value) {
			if ($points_value > 0) $total_points += $points_value;
		}

		return $total_points;

	}//end getAvailablePoints()


	/**
	* Compiles the results array to be used by the quiz during marking
	*
	* The results are returned in the following format:
	* <pre>
	* Array(
	*	'assetid'			=> assetid,
	*	'value'				=> string,	// flattened raw value
	*	'summary'			=> string,	// flattened human-readible value
	*	'points'			=> int,		// earned points
	*	'available_points'	=> int,		// available points
	* )
	* </pre>
	*
	* @param boolean	$flatten	whether not to flatten the human-readable 'summary' array into a single string (TRUE by default)
	*
	* @return array
	* @access public
	*/
	function getResults($flatten=TRUE)
	{
		$value           = $this->getValue();
		$correct_value   = $this->getCorrectValue();
		$summary         = $this->getSummary();
		$correct_summary = $this->getCorrectSummary();

		$points    = $this->getPoints();
		$available = $this->getAvailablePoints();

		$results   = Array(
						'assetid'			=> $this->id,
						'value'				=> $value,
						'summary'			=> $summary,
						'correct_value'		=> $correct_value,
						'correct_summary'	=> $correct_summary,
						'points'			=> $points,
						'available_points'	=> $available,
					 );

		// flatten out the value and summary fields
		foreach (Array('value', 'correct_value') as $field) {
			$value = $results[$field];
			if ($flatten && is_array($value)) {
				$results[$field] = implode(', ', $value);
			}
		}

		foreach (Array('summary', 'correct_summary') as $field) {
			$summary = $results[$field];
			if ($flatten && is_array($summary)) {
				$results[$field] = implode("\n", $summary);
			}
		}

		return $results;

	}//end getResults()


//--        Keyword Replacements        --//


	/**
	* Get the replacement for the "response_form" keyword
	*
	* To be overridden by the children
	*
	* @param text	 $option_format used by Page_Online_Quiz Interactive Mode, control print format of radio/checkbox options.
	* @param boolean $feedback_mode used by Page_Online_Quiz Interactive Mode, will disable fields and add special classes & tags
	*
	* @return string
	* @access public
	*/
	function getResponseFormKeywordReplacement($option_format='', $feedback_mode=FALSE)
	{
		$prefix = $this->getPrefix();

		$options = $this->attr('response_form');
		$use_html_options = $this->attr('use_html_options');

		$value = $this->getValue();
		$correct_value = $this->getCorrectValue();

		ob_start();
			$class = 'sq-'.str_replace('_', '-', $this->type());
			$class = $feedback_mode ? $class.' '.$class.'-feedback' : $class;
			?><div class="<?php echo $class; ?>"><?php

			if (empty($options)) {
				echo translate('online_quiz_question_multichoice_empty_options');
			} else {
				switch ($this->attr('option_style')) {
					case 'list':
						$selection_contents = Array();
						foreach ($options as $option_key => $option) {
							$selection_contents[$option_key] = htmlentities($option['text']);
						}
						$extras = ($feedback_mode) ? 'disabled="disabled"' : '';
						combo_box($prefix.'_response', $selection_contents, ($this->attr('max_option_selections') > 1), $value, 0, $extras);
						if ($feedback_mode) {
							foreach ($options as $option_key => $option) {
								$response_supplement = $this->getResponseSupplement($option_key);
								if (!empty($response_supplement)){
									if (in_array($option_key, $correct_value)){
										?><div class="online_quiz_response_supplement_correct"><?php echo $response_supplement ?></div><?php
									} else {
										?><div class="online_quiz_response_supplement_incorrect"><?php echo $response_supplement ?></div><?php
									}
								}
							}
						}
					break;

					case 'radio':
					default:
						?><ul><?php
						$option_number = 1;
						foreach ($options as $option_key => $option) {

							if ($feedback_mode){
								$is_selected = ((is_array($value) && in_array($option_key, $value)) || $option_key == $value);
								if (in_array($option_key, $correct_value)){
									if ($is_selected){
										?><li class="online_quiz_correct_answer online_quiz_user_selection"><?php
									} else {
										?><li class="online_quiz_correct_answer"><?php
									}
								} else {
									if ($is_selected){
										?><li class="online_quiz_incorrect_answer online_quiz_user_selection"><?php
									} else {
										?><li class="online_quiz_incorrect_answer"><?php
									}
								}
							} else {
								?><li><?php
							}

							$extras = 'id="'.$prefix.'_response_'.$option_key.'"';
							if ($feedback_mode) $extras .= ' disabled="disabled"';

							ob_start();
							if ($this->attr('max_option_selections') <= 1) {
								radio_button($prefix.'_response', $option_key, ($value == $option_key), '', $extras);
							} else {
								if (!is_array($value)) $value = Array();
								check_box($prefix.'_response['.$option_key.']', '1', (in_array($option_key, $value)), '', $extras);
							}
							$option_input_field = ob_get_clean();

							ob_start();
							if ($use_html_options) {
								$matches = Array();
								preg_match_all('|\./\?a=([0-9]+)|', $option['text'], $matches);
								$urls = $GLOBALS['SQ_SYSTEM']->am->getAssetURL(array_unique($matches[1]));
								foreach ($urls as $assetid => $url) {
									$option['text'] = preg_replace('|\./\?a='.$assetid.'([^0-9])|', $url.'\\1', $option['text']);
								}

								label($option['text'], $prefix.'_response_'.$option_key);
							} else {
								label(htmlentities($option['text']), $prefix.'_response_'.$option_key);
							}
							$option_label = ob_get_clean();

							ob_start();
							if ($feedback_mode) {
								$response_supplement = $this->getResponseSupplement($option_key);
								if (!empty($response_supplement)){
									if (in_array($option_key, $correct_value)){
										?><div class="online_quiz_response_supplement_correct"><?php echo $response_supplement ?></div><?php
									} else {
										?><div class="online_quiz_response_supplement_incorrect"><?php echo $response_supplement ?></div><?php
									}
								}
							}
							$option_response_supplement = ob_get_clean();

							if (!empty($option_format)){
								$option_format_output = $option_format;
								$option_format_output = str_replace('%option_number%', $option_number, $option_format_output);
								$option_format_output = str_replace('%option_input_field%', $option_input_field, $option_format_output);
								$option_format_output = str_replace('%option_label%', $option_label, $option_format_output);
								$option_format_output = str_replace('%option_text%', $option['text'], $option_format_output);
								$option_format_output = str_replace('%option_response_supplement%', $option_response_supplement, $option_format_output);
								echo $option_format_output;
							} else {
								echo $option_input_field.$option_label.$option_response_supplement;
							}
							?></li><?php
							$option_number++;
						}
						?></ul><?php
					break;
				}//end switch (option_style)
			}//end else (empty($options))

			?></div><?php

		return ob_get_clean();

	}//end getResponseFormKeywordReplacement()


	/**
	* Get the Response Supplement content for the selected option
	*
	* @param  string $selected_option override the selected option with another key
	*
	* @return string
	* @access public
	*/
	function getResponseSupplement($selected_option=NULL)
	{
		$response_supplement = '';
		$option = Array();

		$response_options = $this->attr('response_form');
		if (empty($selected_option)) {
			$selected_option = $this->getValue();
		}
		//we can only provide the response supplement for one selection
		if (!is_array($selected_option) && isset($response_options[$selected_option])) {
			$option = $response_options[$selected_option];
		}

		if (isset($option['response_supplement'])) {
			$response_supplement = $option['response_supplement'];

			// replace internal links to pages and images with their full URLs
			$matches = Array();
			preg_match_all('|\./\?a=([0-9]+)|', $response_supplement, $matches);
			$urls = $GLOBALS['SQ_SYSTEM']->am->getAssetURL(array_unique($matches[1]));
			foreach ($urls as $assetid => $url) {
				$response_supplement = preg_replace('|\./\?a='.$assetid.'([^0-9])|', $url.'\\1', $response_supplement);
			}
		}

		return $response_supplement;

	}//end getResponseSupplement()


}//end class
?>
