<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_areas/menu/design_area_menu_type/design_area_menu_type_edit_fns.inc';

/**
* Design_Area_Menu_Recursive_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Menu_Recursive_Edit_Fns extends Design_Area_Menu_Type_Edit_Fns
{

	/**
	* The width of the arrow image
	* @var int
	*/
	var $arrow_width  = 4;

	/**
	* The height of the arrow image
	* @var int
	*/
	var $arrow_height = 6;


	/**
	* Takes the contents for the sub menu element and attempts to process it,
	* is a separate fn to allow menu types that don't want to allow sub menu's to override
	*
	* @param object Asset	$asset			the asset to which we belong
	* @param string			$design_area	the menu type design area that the sub menu is supposed to be
	* @param array			$contents		the sub menu's contents array that we generated with _parseString()
	*
	* @return boolean
	* @access private
	* @see Design_Area_Menu_Type_Edit_Fns::_processContents();
	*/
	function _processSubMenu(&$asset, &$contents)
	{
		// we just want to ignore any stupid sub menu's in our page contents
		trigger_error('You are unable to set sub menu\'s on the recursive menu type', E_USER_WARNING);
		return false;
	}// end _processSubMenu()


	/**
	* Used by _outputContents() to deal with a print tag
	*
	* @param array			$element	the print element tag
	* @param object Asset	$asset		the asset to which we belong
	* @param object Design	$design		the design object that is painting us
	*
	* @access private
	* @see Design_Area_Edit_Fns::_outputContents()
	*/
	function _outputElement($element, &$asset, &$design)
	{
		switch($element['operation']) {
			case 'page' :

				echo '<', '?php
				';

				$parent_menu = &$asset->getParentMenu();
				// if we aren't a sub menu
				if (is_null($parent_menu)) {
					$parent_assetid_name = '$PAGE_LINEAGE[$'.$asset->attr('id_name').'_level][\'assetid\']';

					echo '
				$'.$asset->attr('id_name').'_base_url = (empty($PAGE_LINEAGE[$'.$asset->attr('id_name').'_level])) ? \'\' : current_protocol().\'://\'.$PAGE_LINEAGE[$'.$asset->attr('id_name').'_level][\'url\'];
					';

				} else {
					$parent_assetid_name = '$'.$parent_menu->attr('id_name').'_row[\'assetid\']';

					echo '
				$'.$asset->attr('id_name').'_base_url = $'.$parent_menu->attr('id_name').'_base_url.\'/\'.$'.$parent_menu->attr('id_name').'_row[\'path\'];
					';

				}// end if

				echo '
				'.$asset->attr('id_name').'_recursive_fn($'.$asset->attr('id_name').'_level, '.$parent_assetid_name.', $GLOBALS[\'MENU_DATA\'], $'.$asset->attr('id_name').'_base_url);
				?', '>';

				return true;


			case 'sub' :
				# there ain't going to be any subs :)
				return true;

			default :
				return parent::_outputElement($element, $asset, $design);
		}// end switch

	}// end _outputElement()

}//end class
?>
