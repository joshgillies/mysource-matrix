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
* $Id: design_area_access_history_edit_fns.inc,v 1.2 2005/05/18 05:18:05 arailean Exp $
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
* @version $Revision: 1.2 $
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
	* Returns an array of vars that are setable, used to create the parseString
	*
	* @param object Asset	$asset	the asset whose interface we are painting
	*
	* @return Array
	* @access public
	* @see Design_Area_Edit_Fns::createParseString()
	*/
	function getSetableVars(&$asset)
	{
		$vars = parent::getSetableVars($asset);
		unset($vars['asset_types']);
		unset($vars['asset_count']);
		unset($vars['asset_contents']);
		return $vars;

	}//end getSetableVars()


	/**
	* Loops through the contents array and creates the necessary objects
	*
	* @param object Asset	$asset		the asset to which we belong
	* @param array			$contents	the contents array we generated with _parseString()
	*
	* @return boolean
	* @access private
	*/
	function _processContents(&$asset, &$contents)
	{
		if (!$this->_setVariables($asset, $contents)) {
			return false;
		}

		// see if our page element has any sub's in it
		foreach ($contents as $index => $element) {
			if ($element['_type'] != 'TAG') continue;

			if ('asset' == $element['operation']) {
				if (empty($element['contents'])) {
					$element['contents'] = $this->_getDefaultContents($asset, 'asset');
				}

				if (!$asset->setAttrValue('asset_contents', $element['contents'])) {
					return false;
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

		$GLOBALS['SQ_PROCESSED_DESIGN_AREAS'][$asset->attr('id_name')] = true;
		return $asset->setAttrValue('contents', $contents);

	}//end _processContents()


	/**
	* Used by _outputContents() to deal with a print tag
	*
	* @param array			$element	the print element tag
	* @param object Asset	$asset		the asset to which we belong
	* @param object Design	$design		the design object that is painting us
	*
	* @return boolean
	* @access private
	* @see Design_Area_Edit_Fns::_outputContents()
	*/
	function _outputElement($element, &$asset, &$design)
	{
		$area_id = $asset->attr('id_name');

		switch ($element['operation']) {
			case 'asset' :

				echo '
					<?php
						foreach($history as $position => $asset_info) {
					?>
				';

				if ($asset_contents = $asset->attr('asset_contents')) {
					if (!$this->_outputContents($asset_contents, $asset, $design)) {
						return false;
					}
				}

				echo '
					<?
						}//end foreach
					?>
				';

				return true;

			default :
				return parent::_outputElement($element, $asset, $design);
		}//end switch

	}//end _outputElement()


	/**
	* Output the PHP/HTML code that will hopefully form part of the creating the design file
	*
	* @param Array			$contents	the parse contents to use in the output
	* @param object Asset	$asset		the asset to which we belong
	* @param object Design	$design		the design object that is painting us
	*
	* @return boolean
	* @access private
	*/
	function _createDesignFileOutput($contents, &$asset, &$design)
	{
		$area_id = $asset->attr('id_name');
		$asset_types = $asset->attr('asset_types');
		$max_count = $asset->attr('asset_count');
		$time_format = $asset->attr('time_format');

		// if the design area was not properly set-up, we don't want to output anything
		if (empty($asset_types) || !($max_count > 0)) {
			return true;
		}

		echo '
			<?php
				$allowed_asset_types = '.var_export($asset_types,true).';
				$max_asset_count = '.$max_count.';
				//// Access History Design Area

				// initialise the history data
				if (!isset($_SESSION["design_areas"]["'.$area_id.'"])) {
					$_SESSION["design_areas"]["'.$area_id.'"] = Array();
				}
				$saved_data = $_SESSION["design_areas"]["'.$area_id.'"];
				$history = array_get_index($saved_data, "history", Array());
				$history_hash = array_get_index($saved_data, "history_hash", Array());

				// check if this asset type is being tracked
				$require_query_vars = array_get_index($allowed_asset_types, $ASSET->type());

				if (!is_null($require_query_vars)) {
					// check how existence of query vars influences history recording
					$url = null;
					$query_var = array_get_index($_SERVER, "QUERY_STRING");
					if (!empty($query_var)) {
						$query = true;
						$query_var = "?".$query_var;
					} else {
						$query = false;
					}
					$current_url = current_url();

					if ((($require_query_vars == '.SQ_DESIGN_AREA_HISTORY_STATUS_PRESENT.') && $query) || ($require_query_vars == '.SQ_DESIGN_AREA_HISTORY_STATUS_IGNORE.')) {
						$url = $current_url.$query_var;
					} else if (($require_query_vars == '.SQ_DESIGN_AREA_HISTORY_STATUS_ABSENT.') && !$query) {
						$url = $current_url;
					}

					// now update history if necessary
					if (!is_null($url)){
						// if this url already exists in history, remove it
						$history_index = array_get_index($history_hash, $url);

						if (!is_null($history_index)) {
							unset($history[$history_index]);
						}

						if (count($history) > $max_asset_count) {
							array_pop($history);
						}

						// create a new history entry
						$new_entry["assetid"] 			= $ASSET->id;
						$new_entry["asset_link"] 		= $url;
						$new_entry["asset_name"] 		= $ASSET->name;
						$new_entry["asset_short_name"]	= $ASSET->short_name;
						$new_entry["access_time"]		= date("'.$time_format.'");

						array_unshift($history, $new_entry);

						// update history hash
						foreach($history as $index => $contents){
							$new_history_hash[$contents["asset_link"]] = $index;
						}

						$_SESSION["design_areas"]["'.$area_id.'"]["history"] = $history;
						$_SESSION["design_areas"]["'.$area_id.'"]["history_hash"] = $new_history_hash;

						// now that the session is saved, we want to remove the first item because
						// we do not want to list the current page
						array_shift($history);
					}
				}

				$history = array_slice($history, 0, $max_asset_count);
				$history_count = count($history);
			?>
		';

		return parent::_createDesignFileOutput($contents, $asset, $design);

	}//end _createDesignFileOutput()


	/**
	* Paint the interface for choosing which asset types to list
	*
	* @param object Design_Area_Access_History	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter			&$o		the outputter class
	* @param string								$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAssetTypes(&$asset, &$o, $prefix)
	{

		$write_access = $asset->writeAccess('attributes');
		$asset_types = $asset->attr('asset_types');
		$action_column = $write_access && count($asset_types);

		$types = $GLOBALS['SQ_SYSTEM']->am->getAssetTypes(true);

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
		$even = false;
		foreach ($asset_types as $type => $query_req) {
			$type_prefix = $prefix.'[types]['.$type.']';

			$row_class = $even? 'class="alt"':'';
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
			echo combo_box($prefix.'[new][type_code]', $print_types, false, '');
			echo '</td>';

			// request types for new asset type
			$new_radio_prefix = $prefix.'[new][require_query]';
			$selected = true;
			foreach ($this->_request_types as $req_type => $name) {
				echo '<td>';
				radio_button($new_radio_prefix, $req_type, $selected);
				echo '</td>';
				$selected = false;
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
	* @param object Design_Area_Access_History	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter			&$o		the outputter class
	* @param string								$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAssetTypes(&$asset, &$o, $prefix)
	{
		$request_info = array_get_index($_REQUEST, $prefix, Array());

		if (empty($request_info)) return true;

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

		return true;

	}//end processAssetTypes()


}//end class
?>
