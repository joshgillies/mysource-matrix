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
* $Id: form_question_edit_fns.inc,v 1.13 2004/04/28 02:10:45 lwright Exp $
* $Name: not supported by cvs2svn $
*/


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
		// override static screens - only display details because the others don't apply
		$this->static_screens = Array(	'details'		=> Array(
															'name'			=> 'Details',
															'force_unlock'	=> true,
															'lock_type'		=> 'none',
															),
									);

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

		/*$rules = &$asset->getRules();
		for (reset($rules); null !== ($k = key($rules)); next($rules)) {
			$rule =& $rules[$k];
			$edit = $rule->getEditFns();
			$edit->processInlineBackend($rule, $o, $rule->getPrefix());
		}*/
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
		if (!$asset->writeAccess('links')) return false;

		/*$am = &$GLOBALS['SQ_SYSTEM']->am;
		$links = $asset->_getAllowedLinks();
		$type_codes = Array();
		foreach	($links[SQ_LINK_TYPE_3] as $type_code => $link) {
			// make sure that this is a rule
			if (substr($type_code, 0, 18) != 'form_question_rule') continue;
			$type_codes[] = $type_code;
		}
		$desc = $am->getTypeInfo($type_codes, 'description');
		combo_box('rule', array_merge(Array(' ' => '------ Please Select ------'), $desc), false, '');*/
		
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
		
		/*$rule = $_POST['rule'];

		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$am->includeAsset($rule);
			
		$rule  = new $rule();
		$copy_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_3, 'dependant' => 1, 'exclusive' => 1);
		if (!$rule->create($copy_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');		*/

		return true;

	} // end processAddRule()

}// end class
?>
