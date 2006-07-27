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
* $Id: design_area_menu_normal.inc,v 1.15.2.1 2006/07/27 05:34:14 tbarrett Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_areas/menu/design_area_menu_type/design_area_menu_type.inc';

/**
* Design_Area_Menu_Normal
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.15.2.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Menu_Normal extends Design_Area_Menu_Type
{


	/**
	* Outputs the way for this menu type to get their information and place it in the
	* $GLOBALS['MENU_DATA'] array
	*
	* @return void
	* @access public
	*/
	function printDataCollection()
	{
		$parent_menu =& $this->getParentMenu();

		// if we aren't a sub menu, or if our parent is only showing subs if we are in the lineage, use the asset from the lineage
		if (is_null($parent_menu) || $parent_menu->attr('show_subs') == 'on_current') {
			$majorids = 'Array((int) $ASSET_LINEAGE[$'.$this->attr('id_name').'_level][\'assetid\'])';
			echo '
			if (isset($ASSET_LINEAGE[$'.$this->attr('id_name').'_level])) {
				$'.$this->attr('id_name').'_assetids = menu_get_assets(\''.$this->attr('id_name').'\', Array((int) $ASSET_LINEAGE[$'.$this->attr('id_name').'_level][\'assetid\']));
			} else {
				$'.$this->attr('id_name').'_assetids = Array();
			}

			';
		} else {
			echo '
			$'.$this->attr('id_name').'_assetids = menu_get_assets(\''.$this->attr('id_name').'\', $'.$parent_menu->attr('id_name').'_assetids);

			';
		}// end if

		$sub_menu =& $this->getSubMenu();
		if (!is_null($sub_menu)) {
			echo '
		if (!empty($'.$this->attr('id_name').'_assetids)) {
		';
			$sub_menu->printDataCollection();
			echo '
		}// end if $'.$this->attr('id_name').'_assetids not empty
		';
		}// end if

	}//end printDataCollection()


}//end class
?>
