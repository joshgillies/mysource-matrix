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
* $Id: design_area_menu_normal.inc,v 1.25 2013/10/01 07:52:49 cupreti Exp $
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
* @version $Revision: 1.25 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Menu_Normal extends Design_Area_Menu_Type
{


	/**
	* Constructor
	*
	* @param string	$assetid	The asset ID to load
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Outputs the way for this menu type to get their information and place it in the
	* $GLOBALS['MENU_DATA'] array
	*
	* @return void
	* @access public
	*/
	public function printDataCollection()
	{
		$parent_menu = $this->getParentMenu();
		$ignore_permissions = $this->attr('ignore_permissions') ? ' , TRUE' : ' , FALSE';
		
		// if we aren't a sub menu, or if our parent is only showing subs if we are in the lineage, use the asset from the lineage
		if (is_null($parent_menu) || $parent_menu->attr('show_subs') == 'on_current') {


			$majorids = 'Array((int) $ASSET_LINEAGE[$'.$this->attr('id_name').'_level][\'assetid\'])';
			if ($this->attr('root_node') == 0) {
				echo '
					if (isset($ASSET_LINEAGE[$'.$this->attr('id_name').'_level])) {
						$'.$this->attr('id_name').'_assetids = menu_get_assets(\''.$this->attr('id_name').'\', Array((int) $ASSET_LINEAGE[$'.$this->attr('id_name').'_level][\'assetid\'])'.$ignore_permissions.');
					} else {
						$'.$this->attr('id_name').'_assetids = Array();
					}
				';
			} else if ($this->attr('root_node') != 0 && $this->attr('level') == 0) {
				// generate the menu data based on the supplied root node if level is set to 'Top'
				echo '$'.$this->attr('id_name').'_assetids = menu_get_assets(\''.$this->attr('id_name').'\', Array((int) '.$this->attr('root_node').')'.$ignore_permissions.');';
			}
		} else {
			echo '
			$'.$this->attr('id_name').'_assetids = menu_get_assets(\''.$this->attr('id_name').'\', $'.$parent_menu->attr('id_name').'_assetids'.$ignore_permissions.');

			';
		}// end if

		$sub_menu = $this->getSubMenu();
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


	/**
	* Print code that initialises this design area and should always be printed in place of the AREA tag
	*
	* We modify the asset lineage if a root node is supplied so that we can
	* print the menu based on the root node instead of the default top level site asset
	*
	* @return void
	* @access protected
	*/
	protected function _printInitCode()
	{
		if ($this->attr('root_node') != 0 && $this->attr('level') == 0) {
			echo '
			<?php

				$within_site = FALSE;
				$menu_'.$this->attr('id_name').'_lineage = Array();

				// if the specified root node is within the current site
				foreach ($ASSET_LINEAGE as $i => $asset_lineage) {
					if ($asset_lineage[\'assetid\'] == '.$this->attr('root_node').') {
						$menu_'.$this->attr('id_name').'_lineage = array_slice($ASSET_LINEAGE, $i);
						$within_site = TRUE;
						break;
					}
				}

				// get the lineage from the site asset to the specified root node
				$root_node = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset('.$this->attr('root_node').');
				if (!is_null($root_node) && !$within_site) {
					$root_url = $root_node->getURL(current_url());
					$root_url = str_replace(Array(\'http://\', \'https://\'), \'\' , $root_url);
					$root_lineage = $GLOBALS[\'SQ_SYSTEM\']->am->getLineageFromURL(null, $root_url);

					// extract the relevant lineage, and overwrites it with the original
					if (!empty($root_lineage)) {
						foreach ($root_lineage as $i => $info) {
							if ($info[\'assetid\'] == '.$this->attr('root_node').') {
								$menu_'.$this->attr('id_name').'_lineage[0] = $root_lineage[$i];
								break;
							}
						}
					}
				}

				if (!empty($menu_'.$this->attr('id_name').'_lineage)) {
					$original_lineage = $ASSET_LINEAGE;
					$ASSET_LINEAGE = $menu_'.$this->attr('id_name').'_lineage;
				}
			?>';
		}//end if

		parent::_printInitCode();

		if ($this->attr('root_node') != 0 && $this->attr('level') == 0) {
			echo '
			<?php
				// restore the asset lineage so that other menus are not affected
				if (!empty($menu_'.$this->attr('id_name').'_lineage)) {
					$ASSET_LINEAGE = $original_lineage;
				}
			?>';
		}

	}//end _printInitCode()


	/**
	* Print code that actually results in HTML output to the browser for this design area
	* This function is called at the AREA tag if print != no, and at the point of any PRINT
	* tags that reference this area.
	*
	* If a root node is supplied, we again modify the asset lineage, replacing it with the
	* lineage generated in _printInitCode
	*
	* @return void
	* @access protected
	*/
	protected function _printPaintingCode()
	{
		if ($this->attr('root_node') != 0 && $this->attr('level') == 0) {
			echo '
			<?php
				if (!empty($menu_'.$this->attr('id_name').'_lineage)) {
					$original_lineage = $ASSET_LINEAGE;
					$ASSET_LINEAGE = $menu_'.$this->attr('id_name').'_lineage;
				}
			?>';
		}//end if

		parent::_printPaintingCode();

		if ($this->attr('root_node') != 0 && $this->attr('level') == 0) {
			echo '
			<?php
				// restore the asset lineage so that other menus are not affected
				if (!empty($menu_'.$this->attr('id_name').'_lineage)) {
					$ASSET_LINEAGE = $original_lineage;
				}
			?>';
		}

	}//end _printPaintingCode()


}//end class
?>
