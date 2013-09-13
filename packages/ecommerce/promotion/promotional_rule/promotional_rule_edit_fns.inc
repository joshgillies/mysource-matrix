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
* $Id: promotional_rule_edit_fns.inc,v 1.3 2013/09/13 07:57:53 cupreti Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Promotional_Rule_Edit_Fns
*
* Purpose
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Promotional_Rule_Edit_Fns extends Asset_Edit_Fns
{
	
	
	/**
	 * Constructor
	 */
	function __construct()
	{
		parent::__construct();

	}//end constructor
	
	
	/**
	 * Paints the promotional rule's interface
	 *
	 * @param object	&$asset			The asset to paint
	 * @param object	&$o				The backend outputter
	 * @param string	$prefix			String prefix for this interface/asset
	 * @param array		$rule_data		The rule data
	 * @param boolean	$write_access	Whether or not the user has write access
	 *
	 * @return boolean
	 * @access public
	 */
	function paintRule(&$asset, &$o, $prefix, $rule_data, $write_access)
	{		
		// Setup default values if there is no rule data
		if (is_null($rule_data)) {
			$rule_data = $asset->getDefaultRuleData();
		}
		
		// Paint the rule restrictions
		$restriction_type_codes = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('promotional_rule_restriction');
		$restriction_types = Array('' => '-- select --');
		foreach($restriction_type_codes as $type_code) {
			$restriction_types[$type_code] = ucwords(str_replace('_', ' ', substr($type_code, strlen('promotional_rule_restriction'))));
		}
		if ($write_access) {
			$o->openField('Add New Condition');
				combo_box($prefix.'[new_restriction]', $restriction_types, FALSE);
			$o->closeField();
		}

		$restrictions = array_get_index($rule_data, 'restrictions', Array());

		$count = 0;
		$o->openField('Conditions');
		if (!empty($restrictions)) {
			echo '<table class="sq-backend-table">';
			foreach($restrictions as $i => $restriction) {
				$restriction_type_code = $restriction['restriction_type_code'];
				if (!$restriction_type_code) {
					continue;
				}
				echo '<tr>';
				echo '<td class="sq-backend-table-cell">'.'Condition #'.(++$count).' '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($restriction_type_code, 'name').'</td>';
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($restriction_type_code);
				$restrictions_asset = new $restriction_type_code();
				$restrictions_edit_fns = $restrictions_asset->getEditFns();

				// Paint each promotional restriction in a section
				echo '<td class="sq-backend-table-cell">';
				if ($write_access) {
					check_box($prefix."_restriction[delete][$i]");
					echo 'Delete? '; 
				}
				echo '</td></tr>';
				
				echo '<tr><td colspan="2">';
				$restrictions_edit_fns->paintRuleRestriction($restrictions_asset, $o, $prefix.'_restrictions_'.$i, $restriction['restrcition_data'], $write_access);
				echo '</td></tr>';
			}
			echo '</table>';
		} else {
			echo 'No conditions set';
		}
		$o->closeField();

		// Child should take it from here
		
	}//end paintRule()
	
	
	/**
	 * Process the promotional rule's interface
	 *
	 * @param object	&$asset		The asset to paint
	 * @param object	&$o			The backend outputter
	 * @param string	$prefix		String prefix for this interface/asset
	 * @param array		&$rule_data	The rule data
	 *
	 * @return boolean
	 * @access public
	 */
	function processRule(&$asset, &$o, $prefix, &$rule_data)
	{
		$restrictions = array_get_index($rule_data, 'restrictions', Array());

		// Delete if any selected for deletion
		$to_delete = isset($_POST[$prefix.'_restriction']['delete']) ? $_POST[$prefix.'_restriction']['delete'] : Array();
		foreach($restrictions as $i => $val) {
			if (isset($to_delete[$i])) {
				unset($restrictions[$i]);
			}
		}

		$new_restriction = array_get_index($_POST[$prefix], 'new_restriction', '');
		$new_restriction_index = FALSE;
		if (!empty($new_restriction)) {
			$restriction_type_codes = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('promotional_rule_restriction');
			if (in_array($new_restriction, $restriction_type_codes)) {
				$restrictions[] = Array('restriction_type_code' => $new_restriction, 'restrcition_data' => NULL);
				end($restrictions);
				$new_restriction_index = key($restrictions);
			}
		}

		$success = TRUE;
		foreach($restrictions as $i => &$restriction) {
			$restriction_type_code = $restriction['restriction_type_code'];
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($restriction_type_code);
			$restriction_asset = new $restriction_type_code();
			$restriction_asset_edit_fns = $restriction_asset->getEditFns();
			
			if ($new_restriction_index !== $i && !$restriction_asset_edit_fns->processRuleRestriction($restriction_asset, $o, $prefix.'_restrictions_'.$i, $restriction['restrcition_data'])) {
				$success = FALSE;
				break;
			}
		}//end foreach

		if ($success) {
			$rule_data['restrictions'] = $restrictions;
		}

		// Child should take it from here
		
	}//end processRule()
	
	
}//end class

?>
