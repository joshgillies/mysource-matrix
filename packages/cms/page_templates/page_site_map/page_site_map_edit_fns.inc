<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: page_site_map_edit_fns.inc,v 1.18.2.1 2006/02/16 22:22:38 sdanis Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Page_Site_Map_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.18.2.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Site_Map_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Page_Site_Map_Edit_Fns()
	{
		$this->Page_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = FALSE;
		$this->static_screens['details']['lock_type'] = 'content';

	}//end constructor


	/**
	* Paints the root node selection box
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintRootNode(&$asset, &$o, $prefix)
	{
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', FALSE, 'root');
		$write_access = $asset->writeAccess('links');

		if (!$write_access) {
			$root_asset = NULL;
			if (!empty($root_link)) {
				echo get_asset_tag_line($root_link['minorid']);
			} else {
				echo translate('cms_no_root_node');
			}
		} else {
			asset_finder($prefix.'_rootid', (!empty($root_link)) ? $root_link['minorid'] : '');
		}

		return $write_access;

	}//end paintRootNode()


	/**
	* Processes the value input from root node selection box
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processRootNode(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_rootid']['assetid'])) {
			return FALSE;
		}
		$new_root  = $_POST[$prefix.'_rootid']['assetid'];

		// getting existing root link
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', FALSE, 'root');

		// dont process anything if the root node has not changed
		if (!empty($root_link) && $root_link['minorid'] == $new_root) {
			return FALSE;
		}

		$ok = TRUE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// deleting old root link if any found...
		if (!empty($root_link) && !$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($root_link['linkid'])) {
			$ok = FALSE;
		} else if ($new_root) {
			$root =& $GLOBALS['SQ_SYSTEM']->am->getAsset($new_root);
			if (!is_null($root)) {
				if (!$asset->createLink($root, SQ_LINK_NOTICE, 'root')) {
					$ok = FALSE;
					trigger_localised_error('CMS0016', E_USER_WARNING, $root->name, $root->id);
				}
			}
		}

		if ($ok) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $ok;

	}//end processRootNode()


	/**
	* paints the exclude list and the asset finder for adding to the exclude list
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintExcludeList(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$curr_list = $asset->attr('exclude_list');

		if ($write_access) {
			multiple_asset_finder($prefix.'_exclude_list', $curr_list, Array('asset' => 'D'));
		} else {
			if (empty($curr_list)) {
				echo translate('cms_no_assets_excluded');
			} else {
				?><ul style="margin:1px 15px;"><?php
				foreach ($curr_list as $assetid) {
					echo '<li>'.get_asset_tag_line($assetid).'</li>';
				}
				?></ul><?php
			}
		}

		return $write_access;

	}//end paintExcludeList()


	/**
	* processes the exclude list and the asset finder for adding to the exclude list
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processExcludeList(&$asset, &$o, $prefix)
	{
		$exclude_post = array_get_index($_POST, $prefix.'_exclude_list', Array());
		$exclude_list = Array();

		foreach ($exclude_post as $exclude_item) {
			if (!empty($exclude_item['assetid'])) {
				$exclude_list[$exclude_item['assetid']] = $exclude_item['assetid'];
			}
		}

		return $asset->setAttrValue('exclude_list', $exclude_list);

	}//end processExcludeList()


	/**
	* paints the include type list and the asset type finder for adding to the include list
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintTypeIncludeList(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$curr_list = $asset->attr('include_types');

		if ($write_access) {
			asset_type_chooser($prefix.'_include_types', TRUE, $curr_list, NULL, TRUE, TRUE);
		} else {
			if (empty($curr_list)) {
				echo translate('cms_page_site_map_all_asset_types_included');
			} else {
				?><ul style="margin:1px 15px;"><?php

				$type_inherit = Array();
				if (isset($curr_list['inherit'])) {
					$type_inherit = $curr_list['inherit'];
				}

				foreach ($curr_list['type_code'] as $type_code) {
					if (!empty($type_code)) {
						$inherit = FALSE;
						if (!empty($type_inherit)) {
							$inherit = array_shift($type_inherit);
						}

						$type_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'name');
						echo '<li>';
						get_asset_type_icon($type_code);
						echo ' '.$type_name;

						if ($inherit) {
							echo ' ('.translate('cms_page_site_map_inherits_child_types').')';
						}

						echo '</li>';
					}
				}
				?></ul><?php
			}
		}//end else

		return $write_access;

	}//end paintTypeIncludeList()


	/**
	* processes the type include list
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processTypeIncludeList(&$asset, &$o, $prefix)
	{
		$exclude_types = Array();
		if (isset($_POST[$prefix.'_include_types']) && !empty($_POST[$prefix.'_include_types']['type_code'][0])) {
			$exclude_types = $_POST[$prefix.'_include_types'];
		}

		return $asset->setAttrValue('include_types', $exclude_types);

	}//end processTypeIncludeList()


}//end class

?>