<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: js_api_edit_fns.inc,v 1.8 2012/08/30 00:59:46 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Javascript API
*
* Purpose
*
*
* @author  Nic Hubbard <nic@zedsaid.com>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class JS_Api_Edit_Fns extends Page_Edit_Fns
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
	* Paint the interface for choosing which asset types to list
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAssetTypes(&$asset, &$o, $prefix)
	{
		$types = $asset->attr('types');

		if ($asset->writeAccess('attributes')) {

			asset_type_chooser($prefix.'_types', TRUE, array_keys($types));
			return TRUE;
		} else {
			// print the read-only version
			if (empty($types)) {
				echo '<p><em>'.translate('js_api_no_asset_types_selected').'.</em></p>';
			} else {
				$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(array_keys($types), 'name');
				?>
					<table class="sq-backend-table">
						<tr>
							<th><?php echo translate('type') ?></th>
						</tr>
				<?php
				foreach ($types as $type => $value) {
					?>
						<tr>
							<td><?php
								echo get_asset_type_icon($type);
								echo $type_names[$type];
							?></td>
						</tr>
					<?php
				}
				?>
					</table>
				<?php
			}
			return FALSE;
		}//end else - if write access to content

	}//end paintAssetTypes()


	/**
	* Process the asset types to list
	*
	* When a type is removed from the listing, the type format bodycopy is
	* removed. We need to be able to have type formats for the children of all
	* our proxy asset types.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAssetTypes(&$asset, &$o, $prefix)
	{

		if (isset($_POST[$prefix.'_types'])) {
			$types = Array();
			foreach ($_POST[$prefix.'_types'] as $type) {
				if (!empty($type)) $types[$type] = 1;
			}
			$asset->setAttrValue('types', $types);
			return TRUE;
		}
		return FALSE;

	}//end processAssetTypes()
	
	
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
		$write_access = $asset->writeAccess('links');
		$curr_list = $asset->getRootNodes();

		if ($write_access) {
			multiple_asset_finder($prefix.'_roots', $curr_list, Array('asset' => 'D'));
		} else {
			if (empty($curr_list)) {
				echo '<p class="sq-backend-warning"><em>'.translate('js_api_no_root_nodes_selected').'.</em></p>';
			} else {
				?><ul style="margin:1px 15px;"><?php
				foreach ($curr_list as $assetid) {
					echo '<li>'.get_asset_tag_line($assetid).'</li>';
				}
				?></ul><?php
			}
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
		if (isset($asset->_tmp['reverting_to_system_version']) && $asset->_tmp['reverting_to_system_version']) {
			return FALSE;
		}
		$roots = array_get_index($_POST, $prefix.'_roots', Array());
		$root_nodes   = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', TRUE, 'major', 'root');
		$curr_list    = Array();

		foreach ($root_nodes as $node) {
			$node_id = array_get_index($node, 'minorid', 0);
			//while we are looping through each root node, lets remove if we are clearing any of the root node
			$found = FALSE;
			foreach ($roots as $root) {
				if($node_id !== 0 && in_array($node_id, $root)) {
					$found = TRUE;
				}
			}

			if ($node_id !== 0 && !$found) {
				$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($node['linkid']);
			} else if ($node_id !== 0 && $found) {
				$curr_list[] = $node_id;
			}
		}//end foreach

		foreach ($roots as $root) {
			if (!empty($root['assetid']) && !in_array($root['assetid'], $curr_list)) {
				$node = $GLOBALS['SQ_SYSTEM']->am->getAsset($root['assetid'], '', TRUE);
				if (!is_null($node)) {
					$new_link = $GLOBALS['SQ_SYSTEM']->am->createAssetLink($asset, $node, SQ_LINK_NOTICE, 'root');
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($node, TRUE);
				}//end if
			}//end if

		}//end foreach

		return TRUE;

	}//end processRootNode()
	
	
	/**
	* Paints the api key field
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintApiKey(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		
		if ($write_access) {
			text_box($prefix.'_key', $asset->attr('api_key')); 
		} else {
			echo '<p><em>'.translate('js_api_api_key_lock').'.</em></p>';
		}//end else

		return $write_access;

	}//end paintApiKey()
	
	
	/**
	* Processes the value input from the api key field
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processApiKey(&$asset, &$o, $prefix)
	{

		return $asset->setAttrValue('api_key', $_POST[$prefix.'_key']);

	}//end processRootNode()
	
	
	/**
	* Paint the interface for choosing which asset types to list
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintParentTypes(&$asset, &$o, $prefix)
	{
		$types = $asset->attr('types_restriction');

		if ($asset->writeAccess('attributes')) {

			asset_type_chooser($prefix.'_types_restrict', TRUE, array_keys($types));
			return TRUE;
		} else {
			// print the read-only version
			if (empty($types)) {
				echo '<p><em>'.translate('js_api_no_asset_types_selected').'.</em></p>';
			} else {
				$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(array_keys($types), 'name');
				?>
					<table class="sq-backend-table">
						<tr>
							<th><?php echo translate('type') ?></th>
						</tr>
				<?php
				foreach ($types as $type => $value) {
					?>
						<tr>
							<td><?php
								echo get_asset_type_icon($type);
								echo $type_names[$type];
							?></td>
						</tr>
					<?php
				}
				?>
					</table>
				<?php
			}
			return FALSE;
		}//end else - if write access to content

	}//end paintParentTypes()
	
	
	/**
	* Process the asset types to list
	*
	* When a type is removed from the listing, the type format bodycopy is
	* removed. We need to be able to have type formats for the children of all
	* our proxy asset types.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processParentTypes(&$asset, &$o, $prefix)
	{

		if (isset($_POST[$prefix.'_types_restrict'])) {
			$types = Array();
			foreach ($_POST[$prefix.'_types_restrict'] as $type) {
				if (!empty($type)) $types[$type] = 1;
			}
			$asset->setAttrValue('types_restriction', $types);
			return TRUE;
		}
		return FALSE;

	}//end processAssetTypes()
	
	
	/**
	* Print set all buttons
	*
	* Print 2 input buttons. One for making all values Yes, and one for No
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSetAll(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			echo '<span class="sq-inline-fields-wrapper">';
			print_set_all('Yes','1');
			print_set_all('No','0');
			echo '</span>';
		} 
		
		return FALSE;

	}//end processAssetTypes()


	/**
	* Print the warning if Standard Mode is used
	*
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function isUsingStandard(&$asset)
	{
		return !$asset->attr('use_enhanced_js');

	}//end processAssetTypes()


	/**
	* Paint the interface for displaying Standard Mode warning
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintWarning(&$asset, &$o, $prefix)
	{
		?>
		<div class="sq-backend-warning-wrapper">
			<table class="sq-backend-table sq-backend-warning-table">
				<tr>
					<th class="sq-backend-warning-header">
						<?php echo translate('standard_mode_deperacted'); ?>
					</th>
				</tr>	
				<tr>
					<td colspan="2" ><?php echo translate('standard_mode_deperacted_note'); ?>
					</td>
				</tr>
			</table>
		</div>
		<?php

		return TRUE;
	
	}//end paintWarning()


	/**
	* Processes the interface for displaying Standard Mode warning
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processWarning(&$asset, &$o, $prefix)
	{
		return TRUE;

	}//end processWarning()


}//end class

?>
