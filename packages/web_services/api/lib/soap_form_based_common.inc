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
* $Id: soap_form_based_common.inc,v 1.5 2012/10/17 23:30:00 akarelia Exp $
*
*/



/**
* SOAP_Form_Based_Common
* This class will have all the common functions used for SOAP Form Actions and SOAP Form Section Action
*
* @author  Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage web_service
*/


class SOAP_Form_Based_Common
{

	/**
	* This function replace all the keywords for each of the field
	*
	* @return string
	* @access public
	*/
	public static function getKeywordReplacementsFromForm($content, &$form)
	{
		$matches = Array();
		$found = preg_match_all('/%(response_(\d+_)?q\d+[^%]*)%/U', $content, $set_matches, PREG_SET_ORDER);
		$matches = array_merge($matches, $set_matches);
		$found = preg_match_all('/%(question_name_(\d+_)?q\d+[^%]*)%/U', $content, $set_matches, PREG_SET_ORDER);
		$matches = array_merge($matches, $set_matches);
		$found = preg_match_all('/%(form_submission_[^%]*)%/U', $content, $set_matches, PREG_SET_ORDER);
		$matches = array_merge($matches, $set_matches);

		foreach ($matches as $match) {
			if (empty($match)) continue;
			$count = 0;
			do {
				// Apply keyword modifiers, if any
				$full_keyword = $match[1];
				$keyword = parse_keyword($full_keyword, $modifiers);

				$replacement = $form->_getThankYouKeywordReplacement($keyword);
				if ($replacement == '%'.$keyword.'%') {
					break;
				}

				if (!empty($modifiers)) {
					$replace_keywords = Array(
						'assetid' 	=> $form->id,
						'call_fns'	=> Array ('_getThankYouKeywordReplacement', 'getKeywordReplacement'),
					);
					apply_keyword_modifiers($replacement, $modifiers, $replace_keywords);
				}
				$content = preg_replace('/%'.str_replace('^', '\^', $match[1]).'%/U', str_replace('$', '\$', $replacement), $content, 1, $count);

			} while ($count > 0);
		}//end foreach

		// Carryout global keywords replacement
		replace_global_keywords($content);

		return $content;

	}//end getKeywordReplacementsFromForm()


	/**
	* This function replace the keyword %file_upload_1111_q1% with the uploaded file content
	*
	* @return string
	* @access public
	*/
	public static function getFileContentReplacementsFromForm($content, &$form)
	{
		preg_match_all('|%file_upload_([0-9]*_)?q([0-9]+)[^%]*%|U', $content, $matches);

		foreach($matches[1] as $key => $section_match) {
			$question_match = $matches[2][$key];
			if (empty($section_match)) {
				// One asset ID
				$section_assetid = $form->id;
				$question_assetid = $question_match;
			} else {
				$section_assetid = str_replace('_', '', $section_match);
				$question_assetid = $question_match;
			}
			$question_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($section_assetid.':q'.$question_assetid);
			if(empty($question_asset) || get_class($question_asset) != 'Form_Question_Type_File_Upload') return $content;
			// is the file an asset or just raw file in data dir?

			$replacement = '';
			if(isset($question_asset->extra_data['filesystem_path']) && !empty($question_asset->extra_data['filesystem_path'])) {
				$file_path  = $question_asset->extra_data['filesystem_path'];
				if(strpos($file_path, '/') !== 0) $file_path = SQ_SYSTEM_ROOT.'/'.$file_path;
				$replacement = file_get_contents($file_path);
			}
			else {
				$uploaded_assetid = array_get_index($question_asset->extra_data, 'existing_file_assetid', array_get_index($question_asset->extra_data, 'new_file_assetid', NULL));
				if (!empty($uploaded_assetid)) {
					$file = $GLOBALS['SQ_SYSTEM']->am->getAsset($uploaded_assetid);
					$info = $file->getExistingFile();
					if(isset($info['path']))
						$replacement = file_get_contents($info['path']);
				}
			}

			// Apply keyword modifiers, if any
			$full_keyword = trim($matches[0][$key], '%');
			$keyword = parse_keyword($full_keyword, $modifiers);
			if (!empty($modifiers)) {
				apply_keyword_modifiers($replacement, $modifiers);

				$keyword = $full_keyword;
			}

			if (empty($section_match)) {
				$content = str_replace('%'.$keyword.'%', $replacement, $content);
			} else {
				$content = str_replace('%'.$keyword.'%', $replacement, $content);
			}
		}//end foreach

		return $content;

	}//end getFileContentReplacementsFromForm()


	/**
	* Get the array of validation rules for
	* the value in SOAP response
	*
	* @access public
	* @return array
	*/
	public static function validationRules()
	{
		return Array(
					'must_contain' => 'Must contain',
					'must_contain_casesensitive' => 'Must contain (Case sensitive)',
					'must_not_contain' => 'Must NOT contain',
					'must_not_contain_casesensitive' => 'Must NOT contain (Case sensitive)',
					'equals' => 'Must be equal to',
					'equals_not' => 'Must NOT be equal to',
					'less_than' => 'Must be less than',
					'less_than_or_equal' => 'Must be less than or equal to',
					'more_than' => 'Must be more than',
					'more_than_or_equal' => 'Must be more than or equal to',
				);

	}// end validationRules()


	/**
	* Validate the value based on the applied rule
	*
	* @param string $value Value to validate
	* @param array  $rule  Rule to check against
	*
	* @access public
	* @return boolean
	*/
	public static function validateValue($value, $rule)
	{
		$valid = FALSE;
		$rule_type = array_get_index($rule, 'type', '');
		$rule_value = array_get_index($rule, 'value', '');

		switch ($rule_type) {

			case 'must_contain':
				$valid = stripos($value, $rule_value) !== FALSE;
			break;

			case 'must_not_contain':
				$valid = stripos($value, $rule_value) === FALSE;
			break;

			case 'must_contain_casesensitive':
				$valid = strpos($value, $rule_value) !== FALSE;
			break;

			case 'must_not_contain_casesensitive':
				$valid = strpos($value, $rule_value) === FALSE;
			break;

			case 'equals':
				$valid = $value == $rule_value;
			break;

			case 'equals_not':
				$valid = !($value == $rule_value);
			break;

			case 'less_than':
				$valid = $value < $rule_value;
			break;

			case 'more_than':
				$valid = $value > $rule_value;
			break;

			case 'less_than_or_equal':
				$valid = $value <= $rule_value;
			break;

			case 'more_than_or_equal':
				$valid = $value >= $rule_value;
			break;
		}//end switch

		return $valid;

	}// end validateValue()


	/**
	* Converts xml object to array
	* Replace file content keyword
	*
	* @return array
	* @access public
	*/
	public static function toArray($data, &$form)
	{
		if (is_object($data)) $data = get_object_vars($data);

		// Don't play with empty object or array
		if (empty($data) && (is_array($data) || is_object($data))) return '';

		if(is_array($data)) {
			$result = Array();
			foreach ($data as $index => $element) {
				$result[$index] = self::toArray($element, $form);
			}
			return $result;
		}else {
			// Also replace file content keyword with actual binary file
			return (self::getFileContentReplacementsFromForm($data, $form));
		}

	}//end toArray


}//end class

?>
