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
	* $'.$this->attr('id_name').'_data array
	*
	* @param object Design	$design the design for which we are currently printing
	*
	* @access public
	*/
	function printDataCollection(&$design)
	{
		$parent_menu = &$this->getParentMenu();


		echo '


		$sql = \'SELECT a.assetid, l.majorid, a.type_code, a.name, a.short_name, p.path
				FROM sq_asset a
					INNER JOIN sq_asset_path p on a.assetid = p.assetid
					INNER JOIN sq_asset_link l on a.assetid = l.minorid
				WHERE p.sort_order = 0
				  AND l.majorid ';

		// if we aren't a sub menu, or if our parent is only showing subs if we are in the lineage, use the asset from the lineage
		if (is_null($parent_menu) || $parent_menu->attr('show_subs') == 'on_current') {
			echo '= \'.((int) $PAGE_LINEAGE[$'.$this->attr('id_name').'_level][\'assetid\']).\'';
		} else {
			echo 'IN (\'.implode(\',\', $'.$this->attr('id_name').'_assetids).\')';
		}// end if

		echo '
				  AND (l.link_type & \'.SQ_LINK_FRONTEND_NAV.\') > 0
				ORDER BY l.majorid, l.sort_order\';
#pre_echo($sql);
		$'.$this->attr('id_name').'_result = $db->query($sql);
		if (DB::isError($'.$this->attr('id_name').'_result)) {
			trigger_error($'.$this->attr('id_name').'_result->getMessage().\'<br/>\'.$'.$this->attr('id_name').'_result->getUserInfo(), E_USER_ERROR);
		}
		$'.$this->attr('id_name').'_data = Array();
		';

		if ($this->attr('show_subs') == 'always') {
			echo '
		$'.$this->attr('id_name').'_assetids = Array();
		';
		}// end if

		echo '
		while (DB_OK === $'.$this->attr('id_name').'_result->fetchInto($'.$this->attr('id_name').'_row)) {
		';
		if ($this->attr('show_subs') == 'always') {
			echo '
			$'.$this->attr('id_name').'_assetids[] = (int) $'.$this->attr('id_name').'_row[\'assetid\'];
		';
		}// end if
		echo '
			if (!isset($'.$this->attr('id_name').'_data[$'.$this->attr('id_name').'_row[\'majorid\']])) $'.$this->attr('id_name').'_data[$'.$this->attr('id_name').'_row[\'majorid\']] = Array();
			$'.$this->attr('id_name').'_data[$'.$this->attr('id_name').'_row[\'majorid\']][] = $'.$this->attr('id_name').'_row;
		}
		';

		$sub_menu = &$this->getSubMenu();
		if (!is_null($sub_menu)) {
			echo '
		if (!empty($'.$this->attr('id_name').'_data)) {
		';
			$sub_menu->printDataCollection($design);
			echo '
		}// end if $'.$this->attr('id_name').'_data not empty
		';
		}// end if

	}//end printDataCollection()

}//end class
?>
