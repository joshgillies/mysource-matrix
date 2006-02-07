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
* $Id: design_area_access_history_edit_fns.inc,v 1.7 2006/02/07 01:06:57 dmckee Exp $
*
*/

define('SQ_DESIGN_AREA_HISTORY_STATUS_IGNORE',	'0');
define('SQ_DESIGN_AREA_HISTORY_STATUS_PRESENT',	'1');
define('SQ_DESIGN_AREA_HISTORY_STATUS_ABSENT',	'2');

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Access_History_Edit_Fns
*
* Purpose
*	Print the last N assets viewed
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Access_History_Edit_Fns extends Design_Area_Edit_Fns
{

	/**
	* @var Array	$_request_types	Types and names of request variable requirements
	*/
	var $_request_types = Array(
							SQ_DESIGN_AREA_HISTORY_STATUS_IGNORE	=> 'Ignore',
							SQ_DESIGN_AREA_HISTORY_STATUS_PRESENT	=> 'Must exist',
							SQ_DESIGN_AREA_HISTORY_STATUS_ABSENT	=> 'Must NOT exist',
						  );


	/**
	* Loops through the contents array and creates the necessary objects
	*
	* @param object	&$asset		the asset to which we belong
	* @param array	&$contents	the contents array we generated with _parseString()
	*
	* @return boolean
	* @access private
	*/
	function _processContents(&$asset, &$contents)
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
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAssetTypes(&$asset, &$o, $prefix)
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
		$req_type_count = count($this->_request_types);
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
		foreach ($this->_request_types as $name) {
			echo '<th>'.$name.'</th>';
		}

		echo '</tr>';

		// now print the types that were specified previously
		$even = FALSE;
		foreach ($asset_types as $type => $query_req) {
			$type_prefix = $prefix.'[types]['.$type.']';

			$row_class = $even ? 'class="alt"' : '';
			$even = !$even;

			echo '<tr '.$row_class.'><td>'.$present_types[$type].'</td>';
			foreach ($this->_request_types as $req_type => $req_name) {
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
			foreach ($this->_request_types as $req_type => $name) {
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
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAssetTypes(&$asset, &$o, $prefix)
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
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintExcludeAssets(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			multiple_asset_finder($prefix.'exclude_assets', $asset->attr('exclude_assets'), Array());
			return TRUE;
		} else {
			$asset_ids = $asset->attr('exclude_assets');
			$asset_names = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($asset_ids, Array(), TRUE, 'name');
			foreach ($asset_names as $id => $name) {
				echo $name.' (id #'.$id.') <br />';
			}
			return FALSE;
		}

	}//end paintExcludeAssets()


	/**
	* Process the Exclude Assets.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processExcludeAssets(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'exclude_assets'])) {
			$asset_ids = Array();
			foreach ( $_POST[$prefix.'exclude_assets']	as $asset_info ) {
				$asset_ids[] = $asset_info['assetid'];
			}
			return ($asset->setAttrValue('exclude_assets',$asset_ids));
		} else {
			return FALSE;
		}

	}//end processExcludeAssets()


}//end class
?>
