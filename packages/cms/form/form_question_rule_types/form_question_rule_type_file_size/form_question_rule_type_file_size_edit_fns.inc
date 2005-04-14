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
* $Id: form_question_rule_type_file_size_edit_fns.inc,v 1.1.2.1 2005/04/14 01:43:56 lwright Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule_edit_fns.inc';

/**
* Form_Question_Rule_Type_File_Size_Edit_Fns
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.1.2.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Rule_Type_File_Size_Edit_Fns extends Form_Question_Rule_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Rule_Type_File_Size_Edit_Fns()
	{
		$this->Form_Question_Rule_Edit_Fns();

	}//end constructor


	function paintRule(&$asset, &$o, $prefix, $rule_data, $write_access)
	{
		$operator_list = $asset->getOperators();

		if (!$write_access) {
			$o->openField('Rule Details');
			echo 'Uploaded file  '.strtolower($operator_list[array_get_index($rule_data, 'operator', '<=')]);

			echo ' '.$rule_data['value'].' '.$rule_data['multiplier'];
			$o->closeField();

		} else {
			if (!isset($rule_data['multiplier'])) {
				$comparison = strtolower($rule_data['value']);
				if ((substr($comparison,-2) == 'kb') || (substr($comparison,-1) == 'k')) {
					$rule_data['multiplier'] = 'KB';
					$rule_data['value'] = (float)$rule_data['value'];
				} else if ((substr($comparison,-2) == 'mb') || (substr($comparison,-1) == 'm')) {
					$rule_data['multiplier'] = 'MB';
					$rule_data['value'] = (float)$rule_data['value'];
				} else if ((substr($comparison,-2) == 'gb') || (substr($comparison,-1) == 'g')) {
					$rule_data['multiplier'] = 'GB';
					$rule_data['value'] = (float)$rule_data['value'];
				}
			}

			$o->openField('Operator');
			// list of operators
			combo_box($prefix.'[operator]', $operator_list, false, array_get_index($rule_data, 'operator', '<='));
			$o->closeField();

			$o->openField('File Size');
			// give a text box to write the comparison value in
			text_box($prefix.'[value]', array_get_index($rule_data, 'value', ''), 3);
			combo_box($prefix.'[multiplier]', Array('' => 'bytes', 'KB' => 'KB', 'MB' => 'MB', 'GB' => 'GB'), false, array_get_index($rule_data, 'multiplier', ''));
			$o->closeField();

			$o->sectionNote('Please note that your PHP configuration specifies a maximum file upload size of <b>'.strtoupper(easy_filesize($this->_maxUploadSize())).'</b>. If you enter a larger value than this then be aware that the configuration setting will override it. Comparison values for the file size rule this can be entered in shorthand notation: \'K\' or \'KB\' for kilobytes, \'M\' or \'MB\' for megabytes. MySource Matrix regards each KB as 1024 bytes, each MB as 1024 KB.');
		}

		return $write_access;

	}//end paintRule()


	/**
	* Process the rule's interface
	*
	* @return boolean
	* @access public
	*/
	function processRule(&$asset, &$o, $prefix, &$rule_data)
	{
		if (!isset($_POST[$prefix])) {
			return false;
		}

		$multipliers = Array(0 => '', 1 => 'KB', 2 => 'MB', 3 => 'GB');

		$operator = array_get_index($_POST[$prefix], 'operator', null);
		$comparison = array_get_index($_POST[$prefix], 'value', null);
		$multiplier = strtoupper(trim(array_get_index($_POST[$prefix], 'multiplier', '')));

		// expand the comparison size and check whether it is above maximum
		// upload size
		$comparison_size = (float)$comparison * pow(2, 10 * array_search($multiplier, $multipliers));

		if ($comparison_size > $this->_maxUploadSize()) {
			trigger_localised_error('CMS0003', E_USER_NOTICE, easy_filesize($this->_maxUploadSize()));
			$comparison_size = $this->_maxUploadSize();
		}

		// then fold back whatever the comparison size is now and store
		$final_multi = 0;
		while (($comparison_size >= 1024) && ($final_multi < 3)) {
			$comparison_size /= 1024;
			$final_multi++;
		}

		$rule_data['operator'] = $operator;
		$rule_data['value'] = $comparison_size;
		$rule_data['multiplier'] = $multipliers[$final_multi];

		return true;

	}//end processRule()


	/**
	* calculate the max upload size based on PHP configuration
	*
	* @return int	the maximum upload size in bytes
	* @access private
	*/
	function _maxUploadSize()
	{
		$upload_size = strtolower(ini_get('upload_max_filesize'));

		// allow users to specify abbreviated forms of file size (kB, MB, GB)
		// the conversion used is 1024 to the kByte, rather than the decimal 1000
		if (substr($upload_size,-1) == 'k') {
			$upload_size = floatval($upload_size) * 1024;
		} else if (substr($upload_size,-1) == 'm') {
			$upload_size = floatval($upload_size) * 1024 * 1024;
		}

		$post_size = strtolower(ini_get('post_max_size'));

		// allow users to specify abbreviated forms of file size (kB, MB, GB)
		// the conversion used is 1024 to the kByte, rather than the decimal 1000
		if (substr($post_size,-1) == 'k') {
			$post_size = floatval($post_size) * 1024;
		} else if (substr($post_size,-1) == 'm') {
			$post_size = floatval($post_size) * 1024 * 1024;
		}

		$mem_limit_size = strtolower(ini_get('memory_limit'));

		// allow users to specify abbreviated forms of file size (kB, MB, GB)
		// the conversion used is 1024 to the kByte, rather than the decimal 1000
		if (substr($mem_limit_size,-1) == 'k') {
			$mem_limit_size = floatval($mem_limit_size) * 1024;
		} else if (substr($mem_limit_size,-1) == 'm') {
			$mem_limit_size = floatval($mem_limit_size) * 1024 * 1024;
		}

		return min($upload_size, $post_size, $mem_limit_size);
	}//end _maxUploadSize()


}//end class
?>
