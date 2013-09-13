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
* $Id: promotional_rule_restriction_compare_value_edit_fns.inc,v 1.1 2013/09/13 08:10:06 cupreti Exp $
*
*/


require_once dirname(__FILE__).'/../../promotional_rule_restriction/promotional_rule_restriction_edit_fns.inc';

/**
* Promotional_Rule_Compare_Value_Edit_Fns
*
* Purpose
*
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Promotional_Rule_Restriction_Compare_Value_Edit_Fns extends Promotional_Rule_Restriction_Edit_Fns
{
	
	
	/**
	 * Constructor
	 */
	function __construct()
	{
		parent::__construct();

	}//end constructor
	
	
	/**
	 * Paints the promotional rule restriction's interface
	 *
	 * @param object	&$asset				The asset to paint
	 * @param object	&$o					The backend outputter
	 * @param string	$prefix				String prefix for this interface/asset
	 * @param array		$restriction_data	The rule data
	 * @param boolean	$write_access		Whether or not the user has write access
	 *
	 * @return boolean
	 * @access public
	 */
	function paintRuleRestriction(&$asset, &$o, $prefix, $restriction_data, $write_access)
	{		
		// Setup default values if there is no rule data
		if (is_null($restriction_data)) {
			$restriction_data = $asset->getDefaultRuleRestrictionData();			
		}
		
		// Paint the compare "source" field
		echo '<table class="sq-backend-table">';
		echo '<tr>';
		echo '<td width="15%">'.translate('ecom_promotional_rule_restriction_type_compare_value_source_field').'</td>';
		echo '<td>';
			if ($write_access) {
				text_box($prefix.'[source_value]', $restriction_data['source_value'], 50);
			} else {
				echo $restriction_data['source_value'];				
			}			
		echo '</td></tr>';

		// Paint the compare value selection
		echo '<td>'.translate('ecom_promotional_rule_restriction_type_compare_value_compare_field').'</td>';
		echo '<td>';
			if ($write_access) {
				text_box($prefix.'[compare_value]', $restriction_data['compare_value'], 50);
			} else {
				echo $restriction_data['compare_value'];				
			}			
		echo '</td></tr>';
		
		// Paint the comparison type selection
		echo '<td>'.translate('ecom_promotional_rule_restriction_type_compare_value_comparison_type').'</td>';
		echo '<td>';
			$comp_types = $asset->getComparisionTypes();
			if ($write_access) {
				combo_box($prefix.'[comparison_type]', $comp_types, FALSE, $restriction_data['comparison_type']);
			} else {
				echo $comp_types[$restriction_data['comparison_type']];				
			}
			echo '&nbsp;';
			check_box($prefix."[case_sensitive]", 1, $restriction_data['case_sensitive'], '', $write_access ? '' : 'disabled');
			echo 'Case sensitive?';
		echo '</td></tr>';
		
		// Add section note
		echo '<tr><td colspan="2">'.translate('ecom_promotional_rule_restriction_type_compare_value_section_note').'</td></tr>';
		echo '</table>';
		
		return TRUE;
		
	}//end paintRuleRestriction()
	
	
	/**
	 * Process the promotional rule restriction's interface
	 *
	 * @param object	&$asset				The asset to paint
	 * @param object	&$o					The backend outputter
	 * @param string	$prefix				String prefix for this interface/asset
	 * @param array		&$restriction_data	The rule data
	 *
	 * @return boolean
	 * @access public
	 */
	function processRuleRestriction(&$asset, &$o, $prefix, &$restriction_data)
	{		
		if (!isset($_POST[$prefix])) return FALSE;
	
		// Setup default values if there is no rule data
		if (is_null($restriction_data)) {
			$restriction_data = $asset->getDefaultRuleRestrictionData();
		}
		
		// Save compare values
		$restriction_data['source_value'] = array_get_index($_POST[$prefix], 'source_value', '');
		$restriction_data['compare_value'] = array_get_index($_POST[$prefix], 'compare_value', '');		
		
		// Save comparison type		
		$comparison_type = array_get_index($_POST[$prefix], 'comparison_type', '');
		if (array_key_exists($comparison_type, $asset->getComparisionTypes())) {
			$restriction_data['comparison_type'] = $comparison_type;
		}
		$restriction_data['case_sensitive'] = array_get_index($_POST[$prefix], 'case_sensitive', '0');
		
		return TRUE;
		
	}//end processRuleRestriction()
	
	
}//end class

?>