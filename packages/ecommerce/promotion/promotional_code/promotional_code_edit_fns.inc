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
* $Id: promotional_code_edit_fns.inc,v 1.3 2013/09/09 08:35:57 cupreti Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Promotional_Code_Edit_Fns
*
* Purpose
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Promotional_Code_Edit_Fns extends Asset_Edit_Fns
{
	
	
	/**
	 * Constructor
	 */
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor
	
	
	/**
	 * Paints the applied assets selection
	 * 
	 * @param object &$asset	asset being painted
	 * @param object &$o		backend outputter
	 * @param string $prefix	prefix for the html doc element name
	 * 
	 * @return boolean
	 * @access public
	 */
	function paintAppliedAssets(&$asset, &$o, $prefix)
	{
		$link_value = $asset->attr('code');
		
		return $this->paintNoticeLinkedAsset($asset, $link_value, Array('asset' => 'D'), 'applied_assets', TRUE);
		
	}//end paintAppliedAssets()
	
	
	/**
	 * Process the applied assets selection
	 * 
	 * @param object &$asset	asset being painted
	 * @param object &$o		backend outputter
	 * @param string $prefix	prefix for the html doc element name
	 * 
	 * @return void
	 * @access public
	 */
	function processAppliedAssets(&$asset, &$o, $prefix)
	{
		// The 'code' attribute value might be changed already, so we will try to get the old value
		$link_value = $asset->getOldAttrValue('code');
		
		return $this->processNoticeLinkedAsset($asset, $link_value, 'applied_assets', TRUE);
		
	}//end processAppliedAssets()
	
	
	/**
	 * Paints the promotional rules
	 * 
	 * @param object &$asset	asset being painted
	 * @param object &$o		backend outputter
	 * @param string $prefix	prefix for the html doc element name
	 * 
	 * @return boolean
	 * @access public
	 */
	function paintRules(&$asset, &$o, $prefix)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$write_access = $asset->writeAccess('attributes');

		$rule_type_codes = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('promotional_rule');
		$rule_types = Array('' => '-- select --');
		foreach($rule_type_codes as $type_code) {
			$rule_types[$type_code] = ucwords(str_replace('_', ' ', substr($type_code, 22)));
		}
		if ($write_access) {
			$o->openField('Add New Rule');
				combo_box($prefix.'[new_rule]', $rule_types, FALSE);
			$o->closeField();
		}

		$rules = $asset->attr('rules');
		if (empty($rules)) {
			$o->openField('No promotional rules set');
			$o->closeField();
		}

		$count = 0;
		foreach($rules as $i => $rule) {
			$rule_type_code = $rule['rule_type_code'];
			$am->includeAsset($rule_type_code);
			$rule_asset = new $rule_type_code();
			$rule_edit_fns = $rule_asset->getEditFns();

			// Paint each promotional rule in a section
			$o->openSection('Rule #'.(++$count).' '.$am->getTypeInfo($rule_type_code, 'name'));
			if ($write_access) {
				$o->openField('Delete Rule?');
					check_box($prefix."[delete][$i]");
				$o->closeField();
			}
			$rule_edit_fns->paintRule($rule_asset, $o, $prefix.'_rule_'.$i, $rule['rule_data'], $write_access);
			$o->closeSection();
		}

	}//end paintRules()
	
	
	/**
	 * Process the promotional rules
	 * 
	 * @param object &$asset	asset being painted
	 * @param object &$o		backend outputter
	 * @param string $prefix	prefix for the html doc element name
	 * 
	 * @return void
	 * @access public
	 */
	function processRules(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		
		// Get existing rules
		$rules = $asset->attr('rules');

		// Delete if any selected for deletion
		$to_delete = array_get_index($_POST[$prefix], 'delete', Array());
		foreach($to_delete as $i => $val) {
			if (isset($rules[$i])) {
				unset($rules[$i]);
			}
		}

		$new_rule = array_get_index($_POST[$prefix], 'new_rule', '');
		$new_rule_index = FALSE;
		if (!empty($new_rule)) {
			$rule_type_codes = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('promotional_rule');
			if (in_array($new_rule, $rule_type_codes)) {
				$rules[] = Array('rule_type_code' => $new_rule, 'rule_data' => NULL);
				end($rules);
				$new_rule_index = key($rules);
			}
		}
		
		$success = TRUE;
		foreach($rules as $i => &$rule) {
			$rule_type_code = $rule['rule_type_code'];
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($rule_type_code);
			$rule_asset = new $rule_type_code();
			$rule_edit_fns = $rule_asset->getEditFns();
			
			if ($new_rule_index !== $i && !$rule_edit_fns->processRule($rule_asset, $o, $prefix.'_rule_'.$i, $rule['rule_data'])) {
				$success = FALSE;
				break;
			}
		}
		
		return $success && $asset->setAttrValue('rules', $rules);
		
	}//end processRules()
	
	
}//end class

?>
