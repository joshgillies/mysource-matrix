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
* $Id: design_area_menu_type_edit_fns.inc,v 1.58 2012/08/30 01:09:06 ewang Exp $
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
* @version $Revision: 1.58 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Menu_Type_Edit_Fns extends Design_Area_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Loops through the contents array and creates the necessary objects
	*
	* @param Design_Area_Menu_Type	$asset		the asset to which we belong
	* @param array					&$contents	the contents array we generated with _parseString()
	*
	* @return boolean
	* @access private
	*/
	public function _processContents(Design_Area_Menu_Type $asset, Array &$contents)
	{
		$asset->setAttrValue('settings', Array());
		if (!$this->_setVariables($asset, $contents)) {
			return FALSE;
		}
		$settings = $asset->attr('settings');
		// Now just make sure that current, hierarchy and normal entries all exist for the settings
		for (reset($settings); NULL !== ($name = key($settings)); next($settings)) {
			if (!isset($settings[$name]['normal'])) {
				trigger_localised_error('CORE0172', E_USER_WARNING, $name, $asset->name);
				return FALSE;
			}

			if (!isset($settings[$name]['hierarchy'])) {
				$settings[$name]['hierarchy'] = (isset($settings[$name]['current'])) ? $settings[$name]['current'] : $settings[$name]['normal'];
			}

			if (!isset($settings[$name]['current'])) {
				$settings[$name]['current'] = (isset($settings[$name]['hierarchy'])) ? $settings[$name]['hierarchy'] : $settings[$name]['normal'];
			}

		}// end foreach

		if (!$asset->setAttrValue('settings', $settings)) {
			return FALSE;
		}

		$sub_menu_processed = FALSE;
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
							return FALSE;
						}//end if

						$design_area = 'design_area_'.strtolower($sub_element['attributes']['design_area']);

						// first let's make sure that we are actually going to create a menu section type
						$design_area_parent_types = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($design_area);
						if (!in_array('design_area_menu_type', $design_area_parent_types)) {
							trigger_localised_error('CORE0170', E_USER_WARNING, $asset->name);
							return FALSE;
						}

						if (!$this->_processSubMenu($asset, $sub_element['contents'], $design_area)) {
							return FALSE;
						}
						$sub_menu_processed = TRUE;

						break;

					}//end foreach
				break;

				case 'divider' :
					$divider_index = $index;
				break;

			}//end switch

		}//end foreach

		// OK, so we have a working sub menu, add it in
		if ($sub_menu_processed && $asset_index >= 0 && $sub_index >= 0) {
			// Sub menu contents used to be cleared here, which was causing probs with customisations.
			// This remains empty for the moment, but any alterations to sub-menu attributes should
			// be done here.
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
				return FALSE;
			}

			// Now what we do is morph it so that this element doesn't contain all the un-necessary contents stuff
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
				return FALSE;
			}

			// Now we want to remove the divider section
			array_splice($contents, $divider_index, 1);

		}// end if

		$GLOBALS['SQ_PROCESSED_DESIGN_AREAS'][$asset->attr('id_name')] = TRUE;

		$asset->setAttrValue('contents', $contents);
		$asset->saveAttributes();
		return TRUE;

	}//end _processContents()


	/**
	* Takes the contents for the sub menu element and attempts to process it,
	* is a separate fn to allow menu types that don't want to allow sub menu's to override
	*
	* @param Design_Area_Menu_Type	$asset			the asset to which we belong
	* @param array					&$contents		the sub menu's contents array that we generated with _parseString()
	* @param string					$design_area	the menu type design area that the sub menu is supposed to be
	*
	* @return boolean
	* @access private
	* @see _processContents()
	*/
	protected function _processSubMenu(Design_Area_Menu_Type $asset, Array &$contents, $design_area)
	{
		// let's see if we already have a sub menu
		$sub = $asset->getSubMenu();

		// nup ? create one now
		if (empty($sub) || is_null($sub)) {

			// get the parent
			$design_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_3, 'design', FALSE, 'minor', $asset->attr('id_name'));

			if (empty($design_links)) return FALSE;
			$designs = Array();
			foreach ($design_links as $design_link) {
				$design_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($design_link['majorid']);
				if (!is_null($design_asset)) {
					$designs[] = $design_asset;
				}
			}

			if (is_null($designs)) return FALSE;

			$GLOBALS['SQ_SYSTEM']->am->includeAsset($design_area);
			$sub = new $design_area();

			$sub->setAttrValue('id_name', $asset->attr('id_name').'__sub_menu');
			$sub->setAttrValue('contents', $contents);

			$linkids = Array();
			$created = FALSE;
			// create link to design and customisation.
			foreach ($designs as $design) {
				$sub_link = Array('asset' => &$design, 'link_type' => SQ_LINK_TYPE_3, 'value' => $asset->attr('id_name').'__sub_menu', 'is_dependant' => 1);
				// Create the asset and link it to one of the parents.
				if (!$created) {
					$linkid = $sub->create($sub_link);
					if (!$linkid) return FALSE;
					$created = TRUE;
				} else {
					// Create link only now
					$linkid = $GLOBALS['SQ_SYSTEM']->am->createAssetLink($design, $sub, SQ_LINK_TYPE_3, $asset->attr('id_name').'__sub_menu', NULL, 1);
					if (!$linkid) return FALSE;
				}

				$linkids[] = $linkid;
			}

			// Create SQ_LINK_NOTICE to its Upper level menu
			$createAssetLink = $GLOBALS['SQ_SYSTEM']->am->createAssetLink($asset, $sub, SQ_LINK_NOTICE, 'sub_menu');
			if (!$createAssetLink) return FALSE;

		}//end if

		// if the current sub is a different type of menu type then morph it to the correct one
		if ($design_area != $sub->type()) {

			// first up
			if (!$sub->morph('design_area_menu_type')) {
				return FALSE;
			}
			// then down
			if (!$sub->morph($design_area)) return FALSE;
			if (!$sub->setAttrValue('id_name', $asset->attr('id_name').'__sub_menu')) {
				return FALSE;
			}
		}

		$edit_fns = $sub->getEditFns();

		if (!$edit_fns->_processContents($sub, $contents)) {
			return FALSE;
		}

		// We have to set this attribute here between the _processContents function above to make sure this attribute
		// is not reverted to the default empty array.
		$unwanted_asset_types_parents = $asset->attr('unwanted_asset_types');
		if (!empty($unwanted_asset_types_parents)) {
			$sub->setAttrValue('unwanted_asset_types', $unwanted_asset_types_parents);
		}//end if

		if (!$sub->saveAttributes()) {
			return FALSE;
		}

		$asset->saveAttributes();

		$wanted_asset_types_parents = $asset->attr('wanted_asset_types');
		if (!empty($wanted_asset_types_parents)) {
			$sub->setAttrValue('wanted_asset_types', $wanted_asset_types_parents);
		}

		if (!$sub->saveAttributes()) {
			return FALSE;
		}

		$asset->saveAttributes();


		return TRUE;

	}//end _processSubMenu()


	/**
	* Prints the interface for setting the menu level
	*
	* @param Design_Area_Menu_Type	$asset	the owning asset
	* @param Backend_Outputter		$o		the backend outputter
	* @param string					$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintLevel(Design_Area_Menu_Type $asset, Backend_Outputter $o, $prefix)
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
			combo_box($prefix.'_special_level', $special_levels, FALSE, $asset->attr('level'));
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
	* @param Design_Area_Menu_Type	$asset	the owning asset
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processLevel(Design_Area_Menu_Type $asset, Backend_Outputter $o, $prefix)
	{
		$special_level = (int) (isset($_POST[$prefix.'_special_level'])) ? $_POST[$prefix.'_special_level'] : '';
		$custom_level  = (int) (isset($_POST[$prefix.'_level'])) ? $_POST[$prefix.'_level'] : '';
		$level = (!empty($custom_level)) ? $custom_level : $special_level;
		return $asset->setAttrValue('level', $level);

	}//end processLevel()


    /**
    * Paint the interface for choosing asset types to exclude from the lineage
    *
    * @param Design_Area_Asset_Lineage  $asset  the asset whose interface we are painting
    * @param Backend_Outputter          $o      the Backend Outputter object
    * @param string                     $prefix prefix for the form elements
    *
    * @return boolean
    * @access public
    */
    public function paintExcludedTypes(Design_Area_Menu_Type $asset, Backend_Outputter $o, $prefix)
    {
        $excluded_types = $asset->attr('unwanted_asset_types');
        if ($asset->writeAccess('attributes')) {
            asset_type_chooser($prefix.'_excluded_types', TRUE, $excluded_types);
        } else {
            if (empty($excluded_types)) {
                echo 'No types are excluded';
            } else {
                ?>
                <ul>
                <?php
                foreach ($GLOBALS['SQ_SYSTEM']->am->getTypeInfo($excluded_types, 'name') as $type_name) {
                    ?>
                    <li><?php echo $type_name; ?></li>
                    <?php
                }
                ?>
                </ul>
                <?php
            }
        }

    }//end paintExcludedTypes()


    /**
    * Paint the interface for choosing asset types to exclude from the lineage
    *
    * @param Design_Area_Asset_Lineage  $asset  the asset whose interface we are painting
    * @param Backend_Outputter          $o      the Backend Outputter object
    * @param string                     $prefix prefix for the form elements
    *
    * @return boolean
    * @access public
    */
    public function processExcludedTypes(Design_Area_Menu_Type $asset, Backend_Outputter $o, $prefix)
    {
        if ($asset->writeAccess('attributes')) {
            // bug #2712: asset chooser returns an array of empty strings
            // instead of an empty array
            $exclude_list = Array();
            foreach ($_POST[$prefix.'_excluded_types'] as $type) {
                if ($type != '') $exclude_list[] = $type;
            }
            return $asset->setAttrValue('unwanted_asset_types', $exclude_list);
        } else {
            return FALSE;
        }

    }//end processExcludedTypes()


    /**
    * Paint the interface for choosing asset types to include in the lineage
    *
    * @param Design_Area_Asset_Lineage  $asset  the asset whose interface we are painting
    * @param Backend_Outputter          $o      the Backend Outputter object
    * @param string                     $prefix prefix for the form elements
    *
    * @return boolean
    * @access public
    */
    public function paintIncludedTypes(Design_Area_Menu_Type $asset, Backend_Outputter $o, $prefix)
    {
        $included_types = $asset->attr('wanted_asset_types');
        if ($asset->writeAccess('attributes')) {
            asset_type_chooser($prefix.'_included_types', TRUE, $included_types);
        } else {
            if (empty($included_types)) {
                echo 'All types are included';
            } else {
                ?>
                <ul>
                <?php
                foreach ($GLOBALS['SQ_SYSTEM']->am->getTypeInfo($included_types, 'name') as $type_name) {
                    ?>
                    <li><?php echo $type_name; ?></li>
                    <?php
                }
                ?>
                </ul>
                <?php
            }
        }

    }//end paintIncludedTypes()


    /**
    * Paint the interface for choosing asset types to include in the lineage
    *
    * @param Design_Area_Asset_Lineage  $asset  the asset whose interface we are painting
    * @param Backend_Outputter          $o      the Backend Outputter object
    * @param string                     $prefix prefix for the form elements
    *
    * @return boolean
    * @access public
    */
    public function processIncludedTypes(Design_Area_Menu_Type $asset, Backend_Outputter $o, $prefix)
    {
        if ($asset->writeAccess('attributes')) {

            $include_list = Array();
            foreach ($_POST[$prefix.'_included_types'] as $type) {
                if ($type != '') $include_list[] = $type;
            }
            return $asset->setAttrValue('wanted_asset_types', $include_list);
        } else {
            return FALSE;
        }

    }//end processIncludedTypes()

}//end class
?>
