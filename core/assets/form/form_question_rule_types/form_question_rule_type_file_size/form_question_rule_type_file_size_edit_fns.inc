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
* $Id: form_question_rule_type_file_size_edit_fns.inc,v 1.6 2013/04/19 06:36:00 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Form_Question_Rule_Type_File_Size_Edit_Fns
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
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
		$multiplier_map = Array(
							''		=> 'bytes',
							'KB'	=> 'KB',
							'MB'	=> 'MB',
							'GB'	=> 'GB',
						  );

		if (!$write_access) {
			$o->openField(translate('core_form_rule_details'));
			echo translate('core_form_rule_file_size_edit_'.array_get_index($rule_data, 'operator', '<='), array_get_index($rule_data, 'value', '0').' '.$multiplier_map[array_get_index($rule_data, 'multiplier', '')]);
			$o->closeField();

		} else {
			$o->openField(translate('operator'));
			// list of operators
			combo_box($prefix.'[operator]', $operator_list, false, array_get_index($rule_data, 'operator', '<='));
			$o->closeField();

			$o->openField(translate('file_size'));
			// give a text box to write the comparison value in
			int_text_box($prefix.'[value]', array_get_index($rule_data, 'value', ''), false, 3);
			combo_box($prefix.'[multiplier]', $multiplier_map, false, array_get_index($rule_data, 'multiplier', ''));
			$o->closeField();

			$o->sectionNote(translate('core_form_rule_file_size_config_note', strtoupper(easy_filesize($this->_maxUploadSize()))));
		}
		parent::paintRule($asset, $o, $prefix, $rule_data, $write_access);
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
		if (!isset($_POST[$prefix])) return false;
		parent::processRule($asset, $o, $prefix, $rule_data);

		$multipliers = Array(0 => '', 1 => 'KB', 2 => 'MB', 3 => 'GB');

		$operator = array_get_index($_POST[$prefix], 'operator', null);
		$comparison = array_get_index($_POST[$prefix], 'value', null);
		$multiplier = strtoupper(trim(array_get_index($_POST[$prefix], 'multiplier', '')));

		// expand the comparison size and check whether it is above maximum
		// upload size
		$comparison_size = (float)$comparison * pow(2, 10 * array_search($multiplier, $multipliers));

		if ($comparison_size > $this->_maxUploadSize()) {
			// something like "File size exceeds system-imposed limit of xx MB, resetting to this limit"
			// TRANSLATE: CMS0003 => CORE0234
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
