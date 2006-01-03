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
* $Id: design_area_menu_type_edit_fns.inc,v 1.47 2006/01/03 05:59:55 dmckee Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Menu_Type_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.47 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Menu_Type_Edit_Fns extends Design_Area_Edit_Fns
{


	/**
	* Loops through the contents array and creates the necessary objects
	*
	* @param object Asset	$asset		the asset to which we belong
	* @param array			$contents	the contents array we generated with _parseString()
	*
	* @return boolean
	* @access private
	*/
	function _processContents(&$asset, &$contents)
	{
		$asset->setAttrValue('settings', Array());
		if (!$this->_setVariables($asset, $contents)) {
			return false;
		}
		$settings = $asset->attr('settings');
		// Now just make sure that current, hierarchy and normal entries all exist for the settings
		for (reset($settings); NULL !== ($name = key($settings)); next($settings)) {
			if (!isset($settings[$name]['normal'])) {
				trigger_localised_error('CORE0172', E_USER_WARNING, $name, $asset->name);
				return false;
			}

			if (!isset($settings[$name]['hierarchy'])) {
				$settings[$name]['hierarchy'] = (isset($settings[$name]['current'])) ? $settings[$name]['current'] : $settings[$name]['normal'];
			}

			if (!isset($settings[$name]['current'])) {
				$settings[$name]['current'] = (isset($settings[$name]['hierarchy'])) ? $settings[$name]['hierarchy'] : $settings[$name]['normal'];
			}

		}// end foreach

		if (!$asset->setAttrValue('settings', $settings)) {
			return false;
		}

		$sub_menu_processed = false;
		$asset_index	= -1;
		$divider_index	= -1;
		$sub_index		= -1;
		// see if our asset element has any sub's in it
		foreach ($contents as $index => $element) {
			if ($element['_type'] != 'TAG') continue;
			switch ($element['operation']) {
				case 'asset' :
					$asset_index = $index;

					// if there is nothing to do get out of here :)
					if ($element['self_terminating'] || empty($element['contents'])) {
						break;
					}

					foreach ($element['contents'] as $sub_element_index => $sub_element) {
						if ($sub_element['_type'] != 'TAG' || $sub_element['operation'] != 'sub') {
							continue;
						}

						$sub_index = $sub_element_index;

						// right so we have a sub menu, let's check it out

						// if there is a name and it's not the the same type as the current design area
						if (empty($sub_element['attributes']['design_area'])) {
							trigger_localised_error('CORE0144', E_USER_WARNING);
							return false;
						}//end if

						$design_area = 'design_area_'.strtolower($sub_element['attributes']['design_area']);

						// first let's make sure that we are actually going to create a menu section type
						$design_area_parent_types = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($design_area);
						if (!in_array('design_area_menu_type', $design_area_parent_types)) {
							trigger_localised_error('CORE0170', E_USER_WARNING, $asset->name);
							return false;
						}

						if (!$this->_processSubMenu($asset, $sub_element['contents'], $design_area)) {
							return false;
						}
						$sub_menu_processed = true;

						break;

					}// end foreach
				break;

				case 'divider' :
					$divider_index = $index;
				break;

			}// end switch

		}//end foreach

		// OK, so we have a working sub menu, add it in
		if ($sub_menu_processed && $asset_index >= 0 && $sub_index >= 0) {
			//Sub menu contents used to be cleared here, which was causing probs with customisations.
			//This remains empty for the moment, but any alterations to sub-menu attributes should
			//be done here.
		// otherwise remove any reference to it
		} else {
			$link = $asset->getSubMenuLink();
			if (!empty($link)) {
				$asset->deleteLink($link['linkid']);
			}
		}// end if

		// OK, so we have some asset contents, so extract it
		if ($asset_index >= 0) {

			// if there is no contents, then see if there is a default
			if (empty($contents[$asset_index]['contents'])) {
				$contents[$asset_index]['contents'] = $this->_getDefaultContents($asset, 'asset');
			}

			if (!$asset->setAttrValue('asset_contents', $contents[$asset_index]['contents'])) {
				return false;
			}

			//Now what we do is morph it so that this element doesn't contain all the un-necessary contents stuff
			$contents[$asset_index] = Array(
										'_type'				=> 'TAG',
										'attributes'		=> Array (),
										'operation'			=> 'asset',
										'self_terminating'	=> 1,
									  );

		}// end if

		// OK, so we have some divider contents, so extract it
		if ($divider_index >= 0) {

			// if there is no contents, then see if there is a default
			if (empty($contents[$divider_index]['contents'])) {
				$contents[$divider_index]['contents'] = $this->_getDefaultContents($asset, 'divider');
			}

			if (!$asset->setAttrValue('divider_contents', $contents[$divider_index]['contents'])) {
				return false;
			}

			// Now we want to remove the divider section
			array_splice($contents, $divider_index, 1);

		}// end if

		$GLOBALS['SQ_PROCESSED_DESIGN_AREAS'][$asset->attr('id_name')] = true;

		$asset->setAttrValue('contents', $contents);
		$asset->saveAttributes();
		return true;

	}//end _processContents()


	/**
	* Takes the contents for the sub menu element and attempts to process it,
	* is a separate fn to allow menu types that don't want to allow sub menu's to override
	*
	* @param object Asset	$asset			the asset to which we belong
	* @param array			$contents		the sub menu's contents array that we generated with _parseString()
	* @param string			$design_area	the menu type design area that the sub menu is supposed to be
	*
	* @return boolean
	* @access private
	* @see _processContents();
	*/
	function _processSubMenu(&$asset, &$contents, $design_area)
	{
		// let's see if we already have a sub menu
		$sub = &$asset->getSubMenu();
		// nup ? create one now
		if (is_null($sub)) {
			// get the parent
			$design_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_3, 'design', false, $asset->attr('id_name'), 'minor');
			if (empty($design_link)) return false;
			$design = &$GLOBALS['SQ_SYSTEM']->am->getAsset($design_link['majorid']);
			if (is_null($design)) return false;

			$GLOBALS['SQ_SYSTEM']->am->includeAsset($design_area);
			$sub = new $design_area();
			$sub_link = Array('asset' => &$design, 'link_type' => SQ_LINK_TYPE_3, 'value' => $asset->attr('id_name').'__sub_menu', 'is_dependant' => 1);
			$sub->setAttrValue('id_name', $asset->attr('id_name').'__sub_menu');
			$sub->setAttrValue('contents', $contents);
			if (!$sub->create($sub_link)) return false;

			if (!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($asset, $sub, SQ_LINK_NOTICE, 'sub_menu')) {
				return false;
			}
		}

		// if the current sub is a different type of menu type then morph it to the correct one
		if ($design_area != $sub->type()) {
			// first up
			if (!$sub->morph('design_area_menu_type')) {
				return false;
			}
			// then down
			if (!$sub->morph($design_area)) return false;
			if (!$sub->setAttrValue('id_name', $asset->attr('id_name').'__sub_menu')) {
				return false;
			}
		}
		$edit_fns = $sub->getEditFns();
		if (!$edit_fns->_processContents($sub, $contents) && $sub->saveAttributes()) {
			return false;
		}
		$asset->saveAttributes();
		return true;

	}//end _processSubMenu()




	/**
	* Prints the interface for setting the menu level
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintLevel(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$current_level = $asset->attr('level');
		$special_levels = Array(
							''		=> '-- '.strtolower(translate('custom')).' --',
							'-2'	=> translate('children'),
							'-1'	=> translate('current'),
							'0'		=> translate('top'),
							'1'		=> translate('sub'),
						  );

		if ($write_access) {
			combo_box($prefix.'_special_level', $special_levels, false, $asset->attr('level'));
			echo ' '.strtolower(translate('or')).' ';
			text_box($prefix.'_level', (!isset($special_levels[$current_level])) ? $current_level : '');
		} else {
			if (isset($special_levels[$current_level])) {
				echo $special_levels[$current_level];
			} else {
				echo $current_level;
			}
		}

		return $write_access;

	}//end paintLevel()


	/**
	* Processes the interface for setting the menu level
	*
	* @param object Asset				&$asset	the owning asset
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processLevel(&$asset, &$o, $prefix)
	{
		$special_level = (int) (isset($_POST[$prefix.'_special_level'])) ? $_POST[$prefix.'_special_level'] : '';
		$custom_level  = (int) (isset($_POST[$prefix.'_level'])) ? $_POST[$prefix.'_level'] : '';
		$level = (!empty($custom_level)) ? $custom_level : $special_level;
		return $asset->setAttrValue('level', $level);

	}//end processLevel()


}//end class
?>