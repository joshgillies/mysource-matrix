<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Menu_Type_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
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
		if (!$this->_setVariables($asset, $contents)) return false;
		$settings = $asset->attr('settings');
		// Now just make sure that current, hierarchy and normal entries all exist for the settings
		for(reset($settings); NULL !== ($name = key($settings)); next($settings)) {
			if (!isset($settings[$name]['normal'])) {
				trigger_error('Setting "'.$name.'" has no corresponding "normal", in "'.$asset->name.'" design area', E_USER_WARNING);
				return false;
			}

			if (!isset($settings[$name]['hierarchy'])) {
				$settings[$name]['hierarchy'] = (isset($settings[$name]['current'])) ? $settings[$name]['current'] : $settings[$name]['normal'];
			}

			if (!isset($settings[$name]['current'])) {
				$settings[$name]['current'] = (isset($settings[$name]['hierarchy'])) ? $settings[$name]['hierarchy'] : $settings[$name]['normal'];
			}

		}// end foreach

		if (!$asset->setAttrValue('settings', $settings)) return false;

		$sub_menu_processed = false;
		$page_index		= -1;
		$divider_index	= -1;
		$sub_index		= -1;
		// see if our page element has any sub's in it
		foreach($contents as $index => $element) {
			if ($element['_type'] != 'TAG') continue;

			switch ($element['operation']) {
				case 'page' :
					$page_index = $index;

					// if there is nothing to do get out of here :)
					if ($element['self_terminating'] || empty($element['contents'])) break;

					foreach($element['contents'] as $sub_element_index => $sub_element) {
						if ($sub_element['_type'] != 'TAG' || $sub_element['operation'] != 'sub') continue;

						$sub_index = $sub_element_index;

						// right so we have a sub menu, let's check it out

						// if there is a name and it's not the the same type as the current design area
						if (empty($sub_element['attributes']['design_area'])) {
							trigger_error('All sub menu design areas need to have a "design_area"', E_USER_WARNING);
							return false;
						}//end if

						$design_area = 'design_area_'.strtolower($sub_element['attributes']['design_area']);

						// first let's make sure that we are actually going to create a menu section type
						$design_area_parent_types = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($design_area);
						if (!in_array('design_area_menu_type', $design_area_parent_types)) {
							trigger_error('Only Design Area Menu Types can be used in the sub section of the menu section "'.$asset->name.'"', E_USER_WARNING);
							return false;
						}

						if (!$this->_processSubMenu($asset, $sub_element['contents'], $design_area)) return false;
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
		if ($sub_menu_processed && $page_index >= 0 && $sub_index >= 0) {
			// Now what we do is morph it so that this element doesn't contain all the un-necessary contents stuff
			$contents[$page_index]['contents'][$sub_index] = Array(
														'_type' => 'TAG',
														'attributes'  => Array (),
														'operation' => 'sub',
														'self_terminating' => 1
													);
		// otherwise remove any reference to it
		} else {
			$link = $asset->getSubMenuLink();
			if (!empty($link)) {
				$asset->deleteLink($link['linkid']);
			}
		}// end if

		// OK, so we have some page contents, so extract it
		if ($page_index >= 0) {

			// if there is no contents, then see if there is a default
			if (empty($contents[$page_index]['contents'])) {
				$contents[$page_index]['contents'] = $this->_getDefaultContents($asset, 'page');
			}

			if (!$asset->setAttrValue('page_contents', $contents[$page_index]['contents'])) {
				return false;
			}

			// Now what we do is morph it so that this element doesn't contain all the un-necessary contents stuff
			$contents[$page_index] = Array(
											'_type' => 'TAG',
											'attributes'  => Array (),
											'operation' => 'page',
											'self_terminating' => 1
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


		return $asset->setAttrValue('contents', $contents);

	}// end _processContents()


	/**
	* Returns a default contents for the passed type (either 'page' or 'divider')
	* Used when there is no contents supplied in parse file
	*
	* @param object Asset	$asset		the asset to which we belong
	* @param string	$content_type	either 'page' or 'divider'
	*
	* @return Array()
	* @access private
	* @see _processContents();
	*/
	function _getDefaultContents(&$asset, $content_type)
	{
		$parent_types = $asset->getTypeAncestors(false);
		array_unshift($parent_types, $asset->type());
		for($i = 0, $total = count($parent_types); $i < $total; $i++) {

			$file = SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($parent_types[$i], 'dir').'/default_'.$content_type.'_contents.html';
			if (file_exists($file)) {
				require_once SQ_FUDGE_PATH.'/general/file_system.inc';
				$contents = $this->_parseString(file_to_string($file));
				return $contents;
			}//	end if

			// we don't need to go any higher than this class
			if ($parent_types[$i] == 'design_area_menu_type') break;

		}// end for

		return Array();

	}// end _getDefaultContents()


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
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($design_area);
			$sub = new $design_area();
			$sub_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_3, 'value' => 'sub_menu', 'dependant' => '1');
			$sub->setAttrValue('id_name', $asset->attr('id_name').'__sub_menu');
			if (!$sub->create($sub_link)) return false;
		}

		// if the current sub is a different type of menu type then morph it to the correct one
		if ($design_area != $sub->type()) {
			// first up
			if (!$sub->morph('design_area_menu_type')) return false;
			// then down
			if (!$sub->morph($design_area)) return false;
			if (!$sub->setAttrValue('id_name', $asset->attr('id_name').'__sub_menu')) return false;
		}

		// set this value here so that if this sub has a sub, it's value can get in it's contents
		$sub_level = $asset->attr('level') + 1;
		$sub->setAttrValue('level', $sub_level);

		$edit_fns = $sub->getEditFns();
		if (!$edit_fns->_processContents($sub, $contents)) return false;

		if ($sub->attr('level') != $sub_level) {
			trigger_error('The sub menu for "'.$asset->attr('id_name').'" has a level value '.$sub->attr('level').', which is different from what it should be ('.$sub_level.'). This could cause display issues with the menu. Are you specifically setting the level in the sub menu section ? ', E_USER_WARNING);
		}

		return true;

	}// end _processSubMenu()

	/**
	* Used by _outputContents() to deal with a print tag
	*
	* @param array			$element	the print element tag
	* @param object Asset	$asset		the asset to which we belong
	* @param object Design	$design		the design object that is painting us
	*
	* @return boolean
	* @access private
	* @see Design_Area_Edit_Fns::_outputContents()
	*/
	function _outputElement($element, &$asset, &$design)
	{
		switch($element['operation']) {
			case 'page' :
				echo '<', '?php

				$'.$asset->attr('id_name').'_lineage_assetid = (empty($PAGE_LINEAGE[$'.$asset->attr('id_name').'_level + 1])) ? 0 : $PAGE_LINEAGE[$'.$asset->attr('id_name').'_level + 1][\'assetid\'];
				$'.$asset->attr('id_name').'_lineage_setting = (count($PAGE_LINEAGE) - 1 == $'.$asset->attr('id_name').'_level + 1) ? \'current\' : \'hierarchy\';

				';
				$parent_menu = &$asset->getParentMenu();
				// if we aren't a sub menu
				if (is_null($parent_menu)) {
					$parent_assetid_name = '$PAGE_LINEAGE[$'.$asset->attr('id_name').'_level][\'assetid\']';

					echo '
				$'.$asset->attr('id_name').'_base_url = (empty($PAGE_LINEAGE[$'.$asset->attr('id_name').'_level])) ? \'\' : current_protocol().\'://\'.$PAGE_LINEAGE[$'.$asset->attr('id_name').'_level][\'url\'];
				$'.$asset->attr('id_name').'_urls     = $am->getAssetHref(array_keys($GLOBALS[\'MENU_DATA\'][$PAGE_LINEAGE[$'.$asset->attr('id_name').'_level][\'assetid\']]), $'.$asset->attr('id_name').'_base_url);
					';

				} else {
					$parent_assetid_name = '$'.$parent_menu->attr('id_name').'_row[\'assetid\']';

					echo '
				$'.$asset->attr('id_name').'_base_url = $'.$parent_menu->attr('id_name').'_urls[$'.$parent_menu->attr('id_name').'_row[\'assetid\']];
				$'.$asset->attr('id_name').'_urls     = $am->getAssetHref(array_keys($GLOBALS[\'MENU_DATA\'][$'.$parent_menu->attr('id_name').'_row[\'assetid\']]), $'.$asset->attr('id_name').'_base_url);
					';

				}// end if

				echo '

				foreach($GLOBALS[\'MENU_DATA\']['.$parent_assetid_name.'] as $'.$asset->attr('id_name').'_row) {
					$'.$asset->attr('id_name').'_current_setting = ($'.$asset->attr('id_name').'_lineage_assetid == $'.$asset->attr('id_name').'_row[\'assetid\']) ? $'.$asset->attr('id_name').'_lineage_setting : \'normal\';
				?', '>';
				if ($page_contents = $asset->attr('page_contents')) {
					if (!$this->_outputContents($page_contents, $asset, $design)) return false;
				}
				echo '<', '?php
				}// end foreach '.$asset->attr('id_name').' data
				?', '> ';

				return true;

			case 'sub' :

				$sub = &$asset->getSubMenu();
				if (is_null($sub)) return true;

				return $sub->printArea($design);

			default :
				return parent::_outputElement($element, $asset, $design);
		}// end switch

	}// end _outputElement()

}//end class

?>