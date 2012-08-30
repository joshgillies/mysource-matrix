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
* $Id: promotional_code_edit_fns.inc,v 1.2 2012/08/30 00:58:34 ewang Exp $
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
* @version $Revision: 1.2 $
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
		$rules = $asset->attr('rules');
		if (empty($rules)) {
			$rules[] = Array('rule_type_code' => 'promotional_rule_type_quantity', 'rule_data' => NULL);
		}
		
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$write_access = $asset->writeAccess('attributes');
		
		for ($i = 0; $i < count($rules); $i++) {
			$rule = $rules[$i];
			$rule_type_code = $rule['rule_type_code'];
			$am->includeAsset($rule_type_code);
			$rule_asset = new $rule_type_code();
			$rule_edit_fns = $rule_asset->getEditFns();
			
			// Paint each promotional rule in a section
			$o->openSection($am->getTypeInfo($rule_type_code, 'name'));
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
		
		$rules = $asset->attr('rules');
		if (empty($rules)) {
			$rules[] = Array('rule_type_code' => 'promotional_rule_type_quantity', 'rule_data' => NULL);
		}
		
		$success = TRUE;
		for ($i = 0; $i < count($rules); $i++) {
			$rule =& $rules[$i];
			$rule_type_code = $rule['rule_type_code'];
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($rule_type_code);
			$rule_asset = new $rule_type_code();
			$rule_edit_fns = $rule_asset->getEditFns();
			
			if (!$rule_edit_fns->processRule($rule_asset, $o, $prefix.'_rule_'.$i, $rule['rule_data'])) {
				$success = FALSE;
				break;
			}
		}
		
		return $success && $asset->setAttrValue('rules', $rules);
		
	}//end processRules()
	
	
}//end class

?>