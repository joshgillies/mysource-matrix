<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: design_area_show_if_edit_fns.inc,v 1.13 2006/12/05 22:42:14 emcdonald Exp $
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
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Show_If_Edit_Fns extends Design_Area_Edit_Fns
{


	/**
	* Constructor
	*
	* @return void
	* @access public
	*/
	function Design_Area_Show_If_Edit_Fns()
	{
		$this->static_screens['details']['force_unlock'] = FALSE;
		return $this->Design_Area_Edit_Fns();

	}//end constructor


	/**
	* Loops through the contents array and creates the necessary objects
	*
	* @param object	&$asset		the asset to which we belong
	* @param array	&$contents	the contents array we generated with _parseString()
	*
	* @return boolean
	* @access private
	*/
	function _processContents(&$asset, &$contents)
	{
		$then_element = FALSE;
		$else_element = FALSE;

		if (!$this->_setVariables($asset, $contents)) {
			return FALSE;
		}

		// Check if the provided condition exists on the system
		$condition = $asset->attr('condition');
		if (!$GLOBALS['SQ_SYSTEM']->am->installed('condition_'.$condition)) {
			trigger_localised_error('CORE0148', E_USER_WARNING, $condition, $asset->name);
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
						trigger_localised_error('CORE0214', E_USER_WARNING, $asset->name);
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
						trigger_localised_error('CORE0147', E_USER_WARNING, $asset->name);
						return FALSE;
					}

					$else_element = TRUE;

					// Now we want to remove the Else section
					array_splice($contents, $index, 1);
				break;

			}//end switch

		}//end foreach

		if (!$then_element && !$else_element) {
			trigger_localised_error('CORE0169', E_USER_WARNING, $asset->name);
			return FALSE;
		}

		$GLOBALS['SQ_PROCESSED_DESIGN_AREAS'][$asset->attr('id_name')] = TRUE;
		return $asset->saveAttributes();

	}//end _processContents()


	/**
	* Paint the interface for choosing the condition type
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintConditionType(&$asset, &$o, $prefix)
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
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processConditionType(&$asset, &$o, $prefix)
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
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintConditionDetails(&$asset, &$o, $prefix)
	{
		echo translate('condition_'.$asset->attr('condition').'_true');
		$edit_fns_classname = 'condition_'.$asset->attr('condition').'_edit_fns';
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('condition_'.$asset->attr('condition'), TRUE);
		call_user_func(Array($edit_fns_classname, 'paintEditInterface'), $asset->attr('condition_data'), $o, $prefix.'_condition', $asset->writeAccess('attributes'));

	}//end paintConditionDetails()


	/**
	* Process the interface for setting details of the condition
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processConditionDetails(&$asset, &$o, $prefix)
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