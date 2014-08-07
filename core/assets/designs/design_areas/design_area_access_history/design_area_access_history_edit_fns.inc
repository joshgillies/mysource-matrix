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
* $Id: design_area_access_history_edit_fns.inc,v 1.12 2012/08/30 01:09:05 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Access_History_Edit_Fns
*
* Purpose
*	Print the last N assets viewed
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Access_History_Edit_Fns extends Design_Area_Edit_Fns
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
	* @param Design_Area_Access_History	$asset		the asset to which we belong
	* @param array						&$contents	the contents array we generated with _parseString()
	*
	* @return boolean
	* @access private
	*/
	public function _processContents(Design_Area_Access_History $asset, Array &$contents)
	{
		if (!$this->_setVariables($asset, $contents)) {
			return FALSE;
		}

		// see if our page element has any sub's in it
		foreach ($contents as $index => $element) {
			if ($element['_type'] != 'TAG') continue;

			if ('asset' == $element['operation']) {
				if (empty($element['contents'])) {
					$element['contents'] = $this->_getDefaultContents($asset, 'asset');
				}

				if (!$asset->setAttrValue('asset_contents', $element['contents'])) {
					return FALSE;
				}
				// Now what we do is morph it so that this element doesn't contain all the un-necessary contents stuff
				$contents[$index] = Array(
										'_type'				=> 'TAG',
										'attributes'		=> Array (),
										'operation'			=> 'asset',
										'self_terminating'	=> 1,
									);
				break;
			}
		}//end foreach

		$GLOBALS['SQ_PROCESSED_DESIGN_AREAS'][$asset->attr('id_name')] = TRUE;


		return $asset->setAttrValue('contents', $contents);

	}//end _processContents()


	/**
	* Paint the interface for choosing which asset types to list
	*
	* @param Design_Area_Access_History	$asset	the asset to which we belong
	* @param Backend_Outputter			$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAssetTypes(Design_Area_Access_History $asset, Backend_Outputter $o, $prefix)
	{

		$write_access = $asset->writeAccess('attributes');
		$asset_types = $asset->attr('asset_types');
		$action_column = $write_access && count($asset_types);

		$types = $GLOBALS['SQ_SYSTEM']->am->getAssetTypes(TRUE);

		$print_types[''] = '-- Add New Type --';

		foreach ($types as $code => $details) {
			$type_name = $details['name'];

			if (isset($asset_types[$code])) {
				$present_types[$code] = $type_name;
			} else {
				$print_types[$code] = $type_name;
			}
		}

		asort($print_types);
		$req_type_count = count($asset->request_types);
		echo '
			<table class="sq-backend-table">
				<tr>
					<th rowspan="2" style="vertical-align:bottom">Asset Type:</th>
					<th colspan="'.$req_type_count.'">What to do with request parameters?</th>';

		if ($action_column) {
			echo '<th rowspan="2" style="vertical-align:bottom; background-color: #ccc;">Delete?</th>';
		}

		echo'</tr><tr>';

		// request type headers
		foreach ($asset->request_types as $name) {
			echo '<th>'.$name.'</th>';
		}

		echo '</tr>';

		// now print the types that were specified previously
		$even = FALSE;
		foreach ($asset_types as $type => $query_req) {
			$type_prefix = $prefix.'[types]['.$type.']';

			$row_class = $even ? 'class="alt"' : '';
			$even = !$even;

			echo '<tr '.$row_class.'><td>';
			echo get_asset_type_icon($type).$present_types[$type].'</td>';
			foreach ($asset->request_types as $req_type => $req_name) {
				echo '<td>';
				if ($write_access) {
					radio_button($type_prefix.'[req_type]', $req_type, $req_type==$query_req);
				} else {
					if ($req_type==$query_req) {
						echo '<img src="'.sq_web_path('lib').'/web/images/tick.gif" alt="'.$req_name.'" />';
					} else {
						echo '&ndash;';
					}
				}
				echo '</td>';
			}

			if ($action_column) {
				echo '<td>';
				check_box($type_prefix.'[delete]');
				echo '</td>';
			}

			echo '</tr>';
		}

		if ($write_access) {
			// new type selector
			echo '<tr class="alt"><td>';
			echo combo_box($prefix.'[new][type_code]', $print_types, FALSE, '');
			echo '</td>';

			// request types for new asset type
			$new_radio_prefix = $prefix.'[new][require_query]';
			$selected = TRUE;
			foreach ($asset->request_types as $req_type => $name) {
				echo '<td>';
				radio_button($new_radio_prefix, $req_type, $selected);
				echo '</td>';
				$selected = FALSE;
			}
			if ($action_column) echo '<td>&nbsp;</td>';
			echo '</tr>';
		}

		if (empty($asset_types) && !$write_access) {
			echo '<tr><td colspan="'.($req_type_count+1).'" style="color: red">No asset types have been chosen</td></tr>';
		}

		echo '</table>';

		return $write_access;

	}//end paintAssetTypes()


	/**
	* Process the asset types to list
	*
	* @param Design_Area_Access_History	$asset	the asset to which we belong
	* @param Backend_Outputter			$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAssetTypes(Design_Area_Access_History $asset, Backend_Outputter $o, $prefix)
	{
		$request_info = array_get_index($_REQUEST, $prefix, Array());

		if (empty($request_info)) return TRUE;

		$asset_types = $asset->attr('asset_types');

		// add a new asset type
		$new_type = array_get_index($request_info, 'new');
		if (!empty($new_type) && !empty($new_type['type_code'])) {
			$updated_types[$new_type['type_code']] = $new_type['require_query'];
		}

		// update the existing types
		$existing_types = array_get_index($request_info, 'types');
		if (!empty($existing_types) && is_array($existing_types)) {
			foreach ($existing_types as $type => $params) {
				if (!isset($params['delete']) && isset($asset_types[$type])) {
					$updated_types[$type] = $params['req_type'];
				}
			}
		}

		$asset->setAttrValue('asset_types', $updated_types);

		return TRUE;

	}//end processAssetTypes()


	/**
	* Paints the Exclued Assets. These assets are not included in the Asset History.
	*
	* @param Design_Area_Access_History	$asset	the asset to which we belong
	* @param Backend_Outputter			$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintExcludeAssets(Design_Area_Access_History $asset, Backend_Outputter $o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			multiple_asset_finder($prefix.'exclude_assets', $asset->attr('exclude_assets'), Array());
			return TRUE;
		} else {
			$asset_ids = $asset->attr('exclude_assets');
			$asset_names = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($asset_ids, Array(), TRUE, 'name');
			foreach ($asset_names as $id => $name) {
				echo get_asset_tag_line($id);
			}
			return FALSE;
		}

	}//end paintExcludeAssets()


	/**
	* Process the Exclude Assets.
	*
	* @param Design_Area_Access_History	$asset	the asset to which we belong
	* @param Backend_Outputter			$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processExcludeAssets(Design_Area_Access_History $asset, Backend_Outputter $o, $prefix)
	{
		if (isset($_POST[$prefix.'exclude_assets'])) {
			$asset_ids = Array();
			foreach ( $_POST[$prefix.'exclude_assets']	as $asset_info ) {
				if (!empty($asset_info['assetid'])) {
					$asset_ids[] = $asset_info['assetid'];
				}
			}
			return ($asset->setAttrValue('exclude_assets',$asset_ids));
		} else {
			return FALSE;
		}

	}//end processExcludeAssets()


	/**
	* Paint the interface for choosing which asset types to list
	*
	* @param Design_Area_Access_History	$asset	the asset to which we belong
	* @param Backend_Outputter			$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddHistoryAssetTypes(Design_Area_Access_History $asset, Backend_Outputter $o, $prefix)
	{
		$types = $asset->attr('add_history_asset_types');
		if ($asset->writeAccess('attributes')) {
			// print the form fields
			$display_values = Array('type_code' => Array(), 'inherit' => Array());
			foreach ($types as $type => $inherit) {
				$display_values['type_code'][] = $type;
				$display_values['inherit'][] = $inherit;
			}
			asset_type_chooser($prefix.'_button_types', TRUE, $display_values, NULL, TRUE, TRUE);
			return TRUE;
		} else {
			// print the read-only version
			if (empty($types)) {
				echo '<p class="sq-backend-warning">'.translate('no_asset_types_selected').'</p>';
			} else {
				$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(array_keys($types), 'name');
				?>
					<table class="sq-backend-table">
						<tr>
							<th><?php echo translate('type') ?></th>
							<th><?php echo translate('inherit_question') ?></th>
						</tr>
				<?php
				$inherit_image_path = sq_web_path('lib').'/web/images/';
				foreach ($types as $type => $inherit) {
					$inherit_image = $inherit_image_path.($inherit ? 'tick' : 'cross').'.gif';
					$inherit_alt = $inherit ? translate('yes') : translate('no');
					?>
						<tr>
							<td><?php
								echo get_asset_type_icon($type);
								echo $type_names[$type];
							?></td>
							<td><img src="<?php echo $inherit_image; ?>" alt="<?php echo $inherit_alt; ?>" /></td>
						</tr>
					<?php
				}
				?>
					</table>
				<?php
			}
			return FALSE;
		}//end else - if write access to content

	}//end paintAddHistoryAssetTypes()


	/**
	* Process the asset types to list
	*
	* @param Design_Area_Access_History	$asset	the asset to which we belong
	* @param Backend_Outputter			$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddHistoryAssetTypes(Design_Area_Access_History $asset, Backend_Outputter $o, $prefix)
	{
		if (isset($_POST[$prefix.'_button_types'])) {
			$existing_types = $asset->attr('add_history_asset_types');
			$submitted_types = Array();
			foreach ($_POST[$prefix.'_button_types']['type_code'] as $i => $type_code) {
				if (!empty($type_code)) {
					$submitted_types[$type_code] = $_POST[$prefix.'_button_types']['inherit'][$i];
				}
			}

			$asset->setAttrValue('add_history_asset_types', $submitted_types);
			return TRUE;
		} else {
			return FALSE;
		}

	}//end processAddHistoryAssetTypes()


	/**
	* Show/Hide text box
	*
	* @param Design_Area_Access_History	$asset	the asset to which we belong
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function useAddButton(Design_Area_Access_History $asset, $prefix)
	{
		return !$asset->attr('add_history_button_image');

	}//end useAddButton()


	/**
	* Show/Hide text box
	*
	* @param Design_Area_Access_History	$asset	the asset to which we belong
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function useRemoveButton(Design_Area_Access_History $asset, $prefix)
	{
		return !$asset->attr('remove_history_button_image');

	}//end useRemoveButton()


	/**
	* Show/Hide asset picker
	*
	* @param Design_Area_Access_History	$asset	the asset to which we belong
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function showRemoveImageSelection(Design_Area_Access_History $asset, $prefix)
	{
		return $asset->attr('remove_history_button_image');

	}//end showRemoveImageSelection()


	/**
	* Show/Hide asset picker
	*
	* @param Design_Area_Access_History	$asset	the asset to which we belong
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function showAddImageSelection(Design_Area_Access_History $asset, $prefix)
	{
		return $asset->attr('add_history_button_image');

	}//end showAddImageSelection()


}//end class
?>
