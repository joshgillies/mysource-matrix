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
* $Id: promotional_condition_compare_value_edit_fns.inc,v 1.1 2013/09/14 01:43:20 cupreti Exp $
*
*/


require_once dirname(__FILE__).'/../../promotional_condition/promotional_condition_edit_fns.inc';

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
class Promotional_Condition_Compare_Value_Edit_Fns extends Promotional_Condition_Edit_Fns
{


	/**
	 * Constructor
	 */
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	 * Paints the promotional condition's interface
	 *
	 * @param object	&$asset				The asset to paint
	 * @param object	&$o					The backend outputter
	 * @param string	$prefix				String prefix for this interface/asset
	 * @param array		$condition_data		The condition data
	 * @param boolean	$write_access		Whether or not the user has write access
	 *
	 * @return boolean
	 * @access public
	 */
	function paintCondition(&$asset, &$o, $prefix, $condition_data, $write_access)
	{
		// Setup default values if there is no condition data
		if (is_null($condition_data)) {
			$condition_data = $asset->getDefaultConditionData();
		}

		// Paint the compare "source" field
		$o->openField(translate('Source Value'));

			if ($write_access) {
				text_box($prefix.'[source_value]', $condition_data['source_value'], 50);
			} else {
				echo $condition_data['source_value'];
			}
		$o->closeField();

		// Paint the compare value selection
		$o->openField(translate('Compare Value'));

			if ($write_access) {
				text_box($prefix.'[compare_value]', $condition_data['compare_value'], 50);
			} else {
				echo $condition_data['compare_value'];
			}
		$o->closeField();

		// Paint the comparison type selection
		$o->openField(translate('Comparison Type'));

			$comp_types = $asset->getComparisionTypes();
			if ($write_access) {
				combo_box($prefix.'[comparison_type]', $comp_types, FALSE, $condition_data['comparison_type']);
			} else {
				echo $comp_types[$condition_data['comparison_type']];
			}
			echo '&nbsp;';
			check_box($prefix."[case_sensitive]", 1, $condition_data['case_sensitive'], '', $write_access ? '' : 'disabled');
			echo 'Case sensitive?';
		$o->closeField();

		// Add section note
		$o->sectionNote(translate('This condition is based on the value comparison. "Source Value" and "Compare Value" can be considered as first and second operands respectivly, and "Compare Type" as an operator. If the comparison operation returns TRUE, this condition will be considered successful. Globals keywords can be used in "Source Value" and "Compare Value" fields'));


		return TRUE;

	}//end paintCondition()


	/**
	 * Process the promotional condition condition's interface
	 *
	 * @param object	&$asset				The asset to paint
	 * @param object	&$o					The backend outputter
	 * @param string	$prefix				String prefix for this interface/asset
	 * @param array		&$condition_data	The condition data
	 *
	 * @return boolean
	 * @access public
	 */
	function processCondition(&$asset, &$o, $prefix, &$condition_data)
	{
		if (!isset($_POST[$prefix])) return FALSE;

		// Setup default values if there is no condition data
		if (is_null($condition_data)) {
			$condition_data = $asset->getDefaultConditionData();
		}

		// Save compare values
		$condition_data['source_value'] = array_get_index($_POST[$prefix], 'source_value', '');
		$condition_data['compare_value'] = array_get_index($_POST[$prefix], 'compare_value', '');

		// Save comparison type
		$comparison_type = array_get_index($_POST[$prefix], 'comparison_type', '');
		if (array_key_exists($comparison_type, $asset->getComparisionTypes())) {
			$condition_data['comparison_type'] = $comparison_type;
		}
		$condition_data['case_sensitive'] = array_get_index($_POST[$prefix], 'case_sensitive', '0');

		return TRUE;

	}//end processCondition()


}//end class

?>