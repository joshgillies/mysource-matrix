<?php

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Form_Question_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Form_Question_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}// end Form_Question_Edit_Fns()


	/**
	* paint the current rules
	*
	* @param object Form_Section		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRules(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess()) return false;

		$rules = &$asset->getRules();

		if (empty($rules)) {
			echo 'This Question Has No Rules';
			return false;
		}
		
		echo '<table>';
		for (reset($rules); null !== ($k = key($rules)); next($rules)) {
			$rule =& $rules[$k];
			// let the rule know what the id of the question is
			$rule->my_question_id = $asset->id;
			$edit = $rule->getEditFns();
			$edit->paintInlineBackend($rule, $o, $rule->getPrefix());
		}
		echo '</table>';

		return true;

	} // end paintRules()


	/**
	* get the rules to process their parameters
	*
	* @param object Form_Section		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processRules(&$asset, &$o, $prefix)
	{
		if ($asset->getRuleCount() == 0) return false;

		$rules = &$asset->getRules();
		for (reset($rules); null !== ($k = key($rules)); next($rules)) {
			$rule =& $rules[$k];
			$edit = $rule->getEditFns();
			$edit->processInlineBackend($rule, $o, $rule->getPrefix());
		}
		return true;

	} // end processRules()


	/**
	* Paint the dropdowns to add rules to this question
	*
	* @param object Form_Section		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean false if no questions in this section, true otherwise
	* @access public
	*/
	function paintAddRule(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess()) return false;

		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$links = $asset->_getAllowedLinks();
		$type_codes = Array();
		foreach	($links[SQ_LINK_TYPE_3] as $type_code => $link) {
			// make sure that this is a rule
			if (substr($type_code, 0, 18) != 'form_question_rule') continue;
			$type_codes[] = $type_code;
		}
		$desc = $am->getTypeInfo($type_codes, 'description');
		combo_box('rule', array_merge(Array(' ' => '------ Please Select ------'), $desc), false, '');
		
		return true;

	}// end paintAddRule()


	/**
	* Process the newly added questions
	*
	* @param object Form_Section		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddRule(&$asset, &$o, $prefix)
	{
		if (!trim($_POST['rule'])) return false;
		
		$rule = $_POST['rule'];

		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$am->includeAsset($rule);
			
		$rule  = new $rule();
		$copy_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_3, 'dependant' => 1, 'exclusive' => 1);
		if (!$rule->create($copy_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');		

		return true;

	} // end processAddRule()

}// end class
?>
