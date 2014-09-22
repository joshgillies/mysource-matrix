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
* $Id: design_area_show_if_edit_fns.inc,v 1.16 2012/08/30 01:09:06 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Show_If_Edit_Fns
*
* Purpose
*
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.16 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Show_If_Edit_Fns extends Design_Area_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		$this->static_screens['details']['force_unlock'] = FALSE;
		parent::__construct();

	}//end constructor


	/**
	* Loops through the contents array and creates the necessary objects
	*
	* @param Design_Area_Show_If	$asset		the asset to which we belong
	* @param array					&$contents	the contents array we generated with _parseString()
	*
	* @return boolean
	* @access private
	*/
	public function _processContents(Design_Area_Show_If $asset, Array &$contents)
	{
		$then_element = FALSE;
		$else_element = FALSE;

		if (!$this->_setVariables($asset, $contents)) {
			return FALSE;
		}

		// Check if the provided condition exists on the system
		$condition = $asset->attr('condition');
		if (!$GLOBALS['SQ_SYSTEM']->am->installed('condition_'.$condition)) {
			trigger_localised_error('CORE0148', sprintf(translate('Condition %1$s not found for asset "%2$s" design area'), $condition, $asset->name), E_USER_WARNING)
			return FALSE;
		}

		// see if our asset element has any then/else elements in it
		foreach ($contents as $index => $element) {
			if ($element['_type'] != 'TAG') continue;

			switch ($element['operation']) {
				case 'then' :
					if (!$then_element) {
						if (!$asset->setAttrValue('then_contents', $element['contents'])) {
							return FALSE;
						}
					} else {
						trigger_localised_error('CORE0214', sprintf(translate('Cannot have more than one mysource_then section in "%s" design area'), $asset->name), E_USER_WARNING)
						return FALSE;
					}

					$then_element = TRUE;

					// Now we want to remove the Then section
					array_splice($contents, $index, 1);
				break;

				case 'else' :
					if (!$else_element) {
						if (!$asset->setAttrValue('else_contents', $element['contents'])) {
							return FALSE;
						}
					} else {
						trigger_localised_error('CORE0147', sprintf(translate('Cannot have more than one mysource_else section in "%s" design area'), $asset->name), E_USER_WARNING)
						return FALSE;
					}

					$else_element = TRUE;

					// Now we want to remove the Else section
					array_splice($contents, $index, 1);
				break;

			}//end switch

		}//end foreach

		if (!$then_element && !$else_element) {
			trigger_localised_error('CORE0169', sprintf(translate('Must have at least a mysource_then or a mysource_else section in "%s" design area'), $asset->name), E_USER_WARNING)
			return FALSE;
		}

		$GLOBALS['SQ_PROCESSED_DESIGN_AREAS'][$asset->attr('id_name')] = TRUE;
		return $asset->saveAttributes();

	}//end _processContents()


	/**
	* Paint the interface for choosing the condition type
	*
	* @param Design_Area_Show_If	&$asset	the asset whose interface we are painting
	* @param Backend_Outputter		&$o		the outputter class
	* @param string					$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintConditionType(Design_Area_Show_If $asset, Backend_Outputter $o, $prefix)
	{
		$names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('condition'), 'name');
		if ($asset->writeAccess('attributes')) {
			combo_box($prefix.'_condition_type', $names, FALSE, 'condition_'.$asset->attr('condition'));
			return TRUE;
		} else {
			echo array_get_index($names, 'condition_'.$asset->attr('condition'), $asset->attr('condition'));
			return FALSE;
		}

	}//end paintConditionType()


	/**
	* Process the interface for choosing the condition type
	*
	* @param Design_Area_Show_If	&$asset	the asset whose interface we are painting
	* @param Backend_Outputter		&$o		the outputter class
	* @param string					$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processConditionType(Design_Area_Show_If $asset, Backend_Outputter $o, $prefix)
	{
		if (isset($_POST[$prefix.'_condition_type']) && ($_POST[$prefix.'_condition_type'] != 'condition_'.$asset->attr('condition'))) {
			$this->_tmp['condition_type_changed'] = TRUE;
			return $asset->setAttrValue('condition', substr($_POST[$prefix.'_condition_type'], strlen('condition_')));
		}
		return FALSE;

	}//end processConditionType()


	/**
	* Paint the interface for setting details of the condition
	*
	* @param Design_Area_Show_If	&$asset	the asset whose interface we are painting
	* @param Backend_Outputter		&$o		the outputter class
	* @param string					$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintConditionDetails(Design_Area_Show_If $asset, Backend_Outputter $o, $prefix)
	{
		echo translate('condition_'.$asset->attr('condition').'_true');
		$edit_fns_classname = 'condition_'.$asset->attr('condition').'_edit_fns';
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('condition_'.$asset->attr('condition'), TRUE);
		call_user_func(Array($edit_fns_classname, 'paintEditInterface'), $asset->attr('condition_data'), $o, $prefix.'_condition', $asset->writeAccess('attributes'), Array());

	}//end paintConditionDetails()


	/**
	* Process the interface for setting details of the condition
	*
	* @param Design_Area_Show_If	&$asset	the asset whose interface we are painting
	* @param Backend_Outputter		&$o		the outputter class
	* @param string					$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processConditionDetails(Design_Area_Show_If $asset, Backend_Outputter $o, $prefix)
	{
		if (!array_get_index($this->_tmp, 'condition_type_changed')) {
			$edit_fns_classname = 'condition_'.$asset->attr('condition').'_edit_fns';
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('condition_'.$asset->attr('condition'), TRUE);
			$res = call_user_func(Array($edit_fns_classname, 'processEditInterface'), $o, $prefix.'_condition');
			return $asset->setAttrValue('condition_data', $res);
		}
		return FALSE;

	}//end processConditionDetails()


}//end class
?>