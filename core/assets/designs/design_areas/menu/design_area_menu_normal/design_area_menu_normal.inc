<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_areas/menu/design_area_menu_type/design_area_menu_type.inc';

/**
* Design_Area_Menu_Normal
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Design_Area_Menu_Normal extends Design_Area_Menu_Type
{
	/**
	* Outputs the way for this menu type to get their information and place it in the
	* $GLOBALS['MENU_DATA'] array
	*
	* @param object Design	$design the design for which we are currently printing
	*
	* @access public
	*/
	function printDataCollection(&$design)
	{
		$parent_menu = &$this->getParentMenu();

		// if we aren't a sub menu, or if our parent is only showing subs if we are in the lineage, use the asset from the lineage
		if (is_null($parent_menu) || $parent_menu->attr('show_subs') == 'on_current') {
			$majorids = 'Array((int) $PAGE_LINEAGE[$'.$this->attr('id_name').'_level][\'assetid\'])';
		} else {
			$majorids = '$'.$parent_menu->attr('id_name').'_assetids';
		}// end if

		echo '
		$'.$this->attr('id_name').'_assetids = menu_get_assets('.$majorids.');

		';

		$sub_menu = &$this->getSubMenu();
		if (!is_null($sub_menu)) {
			echo '
		if (!empty($'.$this->attr('id_name').'_assetids)) {
		';
			$sub_menu->printDataCollection($design);
			echo '
		}// end if $'.$this->attr('id_name').'_assetids not empty
		';
		}// end if

	}//end printDataCollection()

}//end class
?>
