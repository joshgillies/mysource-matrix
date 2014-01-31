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
* $Id: cache_manager_edit_fns.inc,v 1.25 2012/11/19 04:57:33 akarelia Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_INCLUDE_PATH.'/asset_edit_interface.inc';

/**
* Cache_Manager_Edit_Fns
*
* Edit functions for the Cache manager.
*
* @see Asset_Edit_Fns
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.25 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackag __core__
*/
class Cache_Manager_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::Asset_Edit_Fns
	*/
	function __construct()
	{
		parent::__construct();

		unset($this->static_screens['settings']);
		unset($this->static_screens['preview']);
		unset($this->static_screens['lookupValues']);
		unset($this->static_screens['web_paths']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['workflow']);
		unset($this->static_screens['tagging']);
		unset($this->static_screens['dependants']);
		unset($this->static_screens['linking']);
		unset($this->static_screens['roles']);
		unset($this->static_screens['permissions']);

	}//end constructor


//--        SPECIFIC CACHE EXPIRY        --//


	/**
	* Paints the interface for selecting an asset type to customise
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintChooseTypeCode(&$asset, &$o, $prefix)
	{
		$type_codes = $asset->attr('type_codes');
		$asset_types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();

		foreach ($asset_types as $type_code) {
			$info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
			$options[$type_code] = $info['name'];
		}
		asort($options);

		// we dont use the combo_box function here as we are going
		// to colourise the options that have been customised

		if (isset($_REQUEST[$prefix.'_select_type'])) {
			$selected = $_REQUEST[$prefix.'_select_type'];
		} else {
			$selected = (isset($_REQUEST[$prefix.'_type'])) ? $_REQUEST[$prefix.'_type'] : '';
		}

		?>
		<select name="<?php echo $prefix; ?>_select_type" onchange="this.form.submit();">
			<option value="">-- <?php echo translate('select_asset_type'); ?> --</option>
			<?php
			foreach ($options as $type_code => $name) {
				$style = (isset($type_codes[$type_code])) ? ' style="background-color: #ddd";' : '';
				?>
				<option<?php echo $style; ?> value="<?php echo $type_code; ?>"<?php echo ($selected == $type_code) ? ' selected' : ''; ?>>
					<?php echo $name; ?>
				</option>
				<?php
			}
		?></select><?php
		$o->addFormActionGetVar($prefix.'_type', $selected, TRUE);
		submit_button($prefix.'_go', translate('go'));
		$o->note(translate('type_code_precedence_over_root_node').' '.translate('type_code_current_customised_highlighted'));

		return FALSE;

	}//end paintChooseTypeCode()


	/**
	* Processes the interface for selecting an asset type to customise
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processChooseTypeCode(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processChooseTypeCode()


	/**
	* Paints the interface for customising an individual asset type's cache options
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintCustomiseTypeCodeCaching(&$asset, &$o, $prefix)
	{

		$write_access = $asset->writeAccess('attributes');

		if (isset($_REQUEST[$prefix.'_select_type'])) {
			$selected = $_REQUEST[$prefix.'_select_type'];
		} else {
			$selected = (isset($_REQUEST[$prefix.'_type'])) ? $_REQUEST[$prefix.'_type'] : '';
		}

		if (!empty($selected)) {

			$this->_paintSpecificCacheManagement($asset, $o, $prefix, 'type_codes', $selected, FALSE);

		} else {
			if ($write_access) {
				echo '<em>'.translate('customise_asset_selection_below').'</em>';
			} else {
				echo '<em>'.translate('view_asset_selection_below').'</em>';
			}
			return FALSE;

		}//end if !empty()

		return $write_access;

	}//end paintCustomiseTypeCodeCaching()


	/**
	* Paints the cache management section for the provided type code or root node
	*
	* @param object		&$asset			the asset of the layout manager
	* @param object		&$o				the backend outputter
	* @param string		$prefix			a unique prefix for variables
	* @param string		$attr_name		the attribute name where current values are stored
	* @param string		$selected		the value which needs to be configured
	* @param boolean	$check_exists	check whether the selected value exists in the current value list before painting
	*
	* @return void
	* @access private
	*/
	function _paintSpecificCacheManagement(&$asset, &$o, $prefix, $attr_name, $selected, $check_exists=FALSE)
	{
		$root_nodes = $asset->attr('root_nodes');
		$wa = $asset->writeAccess('attributes');
		$current_values = $asset->attr($attr_name);
		// check if the value to be configured actually exists if specified
		if ($check_exists) {
			$paint = in_array($selected, array_keys($current_values)) ? TRUE : FALSE;
		} else {
			$paint = TRUE;
		}
		if ($paint) {
			// field opened by asset_edit_interface
			if (isset($_REQUEST[$prefix.'_customise_root_node']) && in_array($_REQUEST[$prefix.'_customise_root_node'], array_keys($root_nodes))) {
				if ($wa) {
					echo translate('currently_customising', get_asset_tag_line($_REQUEST[$prefix.'_customise_root_node']));
				} else {
					echo translate('currently_viewing', get_asset_tag_line($_REQUEST[$prefix.'_customise_root_node']));
				}
			} else {
				unset($_REQUEST[$prefix.'_customise_root_node']);
			}

			$o->closeField();



			if (isset($_REQUEST[$prefix.'_select_type']) && !empty($_REQUEST[$prefix.'_select_type'])) {
				$o->openField(translate('asset_field_type_code'));
					if ($wa) {
						echo translate('currently_customising_type_code');
					} else {
						echo translate('currently_viewing_type_code');
					}
					echo '<strong>';
					echo get_asset_type_icon($_REQUEST['cache_manager_20_select_type']);
					echo $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($_REQUEST['cache_manager_20_select_type'], 'name');
					echo '<Ystrong>';
				$o->closeField();
			}

			$o->openField(translate('cache_manager_cache_status'));
			$using_default = !isset($current_values[$selected]['enabled']);
			if ($using_default) {
				echo translate('using_default_value_of',(($asset->attr('enabled')) ? translate('on') : translate('off'))).'<br/>';
			} else {
				$caching = ($current_values[$selected]['enabled']) ? 1 : 0;
				if ($wa) {
					combo_box($prefix.'_enabled', Array('1' => translate('on'), '0' => translate('off')), FALSE, $caching);
					echo ' ';
				} else {
					echo ($current_values[$selected]['enabled']) ? translate('on') : translate('off');
				}
			}
			if ($wa) {
				echo '<span class="sq-inline-fields-wrapper baseline">';
				check_box($prefix.'_default_enabled', '1', $using_default);
				label(translate('use_default'), $prefix.'_default_enabled');
				echo '</span>';
			}
			$o->closeField();

			$o->openField(translate('default_expiry'), '', translate('valid_cache_length'));
				$using_default = !isset($current_values[$selected]['expiry']);
				if ($using_default) {
					echo translate('using_default_value_of', $asset->attr('expiry').' seconds').'<br/>';
				} else {
					if ($wa) {
						text_box($prefix.'_expiry', $current_values[$selected]['expiry'], 5);
						echo ' ';
					} else {
						echo $current_values[$selected]['expiry'];
					}
				}
				if ($wa) {
					echo '<span class="sq-inline-fields-wrapper baseline">';
					check_box($prefix.'_default_expiry', '1', $using_default);
					label(translate('use_default'), $prefix.'_default_expiry');
					echo '</span>';
				}
			$o->closeField();

			// Accelerator cache expiry field
			$o->openField(translate('cache_manager_accelerator_cache_expiry'), '', translate('cache_manager_accelerator_valid_cache_length'));
				$using_default_accelerator = !isset($current_values[$selected]['accelerator_cache_expiry']);
				if ($using_default_accelerator) {
					$accelerator_cache_expiry = $asset->attr('accelerator_cache_expiry');
					if (empty($browser_cache_expiry)) {
						echo translate('cache_manager_using_blank_default_value_equal_to', 'the system Default Expiry', $asset->attr('expiry').' seconds').'<br>';
					} else {
						echo translate('using_default_value_of', $accelerator_cache_expiry.' seconds').'<br/>';
					}
				} else {
					if ($wa) {
						text_box($prefix.'_accelerator_cache_expiry', $current_values[$selected]['accelerator_cache_expiry'], 5);
						echo ' ';
					} else {
						echo $current_values[$selected]['accelerator_cache_expiry'];
					}
				}

				if ($wa) {
					echo '<span class="sq-inline-fields-wrapper baseline">';
					check_box($prefix.'_default_accelerator_cache_expiry', '1', $using_default_accelerator);
					label(translate('use_default'), $prefix.'_default_accelerator_cache_expiry');
					echo '</span>';
				}
			$o->closeField();

			$o->openField(translate('cache_manager_browser_cache_expiry'), '', translate('cache_manager_browser_valid_cache_length'));
				$using_default = !isset($current_values[$selected]['browser_cache_expiry']);
				if ($using_default) {
					$browser_cache_expiry = $asset->attr('browser_cache_expiry');
					if (empty($browser_cache_expiry)) {
						echo translate('cache_manager_using_blank_default_value_equal_to', 'the system Default Expiry', $asset->attr('expiry').' seconds').'<br>';
					} else {
						echo translate('using_default_value_of', $browser_cache_expiry.' seconds').'<br/>';
					}
				} else {
					if ($wa) {
						text_box($prefix.'_browser_cache_expiry', $current_values[$selected]['browser_cache_expiry'], 5);
						echo ' ';
					} else {
						echo $current_values[$selected]['browser_cache_expiry'];
					}
				}

				if ($wa) {
					echo '<span class="sq-inline-fields-wrapper baseline">';
					check_box($prefix.'_default_browser_cache_expiry', '1', $using_default);
					label(translate('use_default'), $prefix.'_default_browser_cache_expiry');
					echo '</span>';
				}
			$o->closeField();

			$o->openField(translate('http_header_cacheable_field'));
				$using_default = !isset($current_values[$selected]['send_cacheable_header']);
				if ($using_default) {
					echo translate('using_default_value_of', SQ_CONF_SEND_CACHEABLE_HEADER ? translate('yes') : translate('no')).'<br/>';
				} else {
					if ($wa) {
						combo_box($prefix.'_send_cacheable_header', Array('1' => translate('on'), '0' => translate('off')), FALSE, $current_values[$selected]['send_cacheable_header']);
						echo ' ';
					} else {
						if ($current_values[$selected]['send_cacheable_header'] == '0' ){
							echo translate('off');
						} else {
							echo translate('on');
						}
					}
				}
				if ($wa) {
					echo '<span class="sq-inline-fields-wrapper baseline">';
					check_box($prefix.'_default_send_cacheable_header', '1', $using_default);
					label(translate('use_default'), $prefix.'_default_send_cacheable_header');
					echo '</span>';
				}

				// field closed by asset_edit_interface
		}

	}//end _paintSpecificCacheManagement();


	/**
	* Processes the interface for customising specific cache management options
	*
	* @param object		&$asset					the asset of the layout manager
	* @param object		&$o						the backend outputter
	* @param string		$prefix					a unique prefix for variables
	* @param string		$attr_name				the attribute name where current values are stored
	* @param string		$value_get_var			the value which needs to be configured
	* @param boolean	$indicate_customised	whether to indicate in the saved attribute if a value is customised
	*
	* @return boolean
	* @access private
	*/
	function _processSpecificCacheManagement(&$asset, &$o, $prefix, $attr_name, $value_get_var, $indicate_customised=FALSE)
	{
		$current_values = $asset->attr($attr_name);

		if ($asset->writeAccess('attributes') && !isset($_POST[$value_get_var]) && isset($_GET[$value_get_var])) {
			$type = $_REQUEST[$value_get_var];
			$defaults = Array('enabled', 'expiry', 'browser_cache_expiry', 'accelerator_cache_expiry', 'send_cacheable_header');
			$save = FALSE;

			foreach ($defaults as $default) {
				$using_default = FALSE;
				if (!isset($_POST[$prefix.'_default_'.$default])) {
					if (!isset($current_values[$type][$default])) {
						$save = TRUE;
						if($default === 'send_cacheable_header') {
							$current_values[$type][$default] = SQ_CONF_SEND_CACHEABLE_HEADER;
						}
						else {
							$current_values[$type][$default] = $asset->attr($default);
						}
					}
				} else {
					$save = TRUE;
					$using_default = TRUE;
					if (isset($current_values[$type][$default])) {
						unset($current_values[$type][$default]);
					}
				}
				if (isset($_POST[$prefix.'_'.$default]) && !$using_default) {
					if (!isset($current_values[$type][$default]) || $current_values[$type][$default] != $_POST[$prefix.'_'.$default]) {
						$current_values[$type][$default] = $_POST[$prefix.'_'.$default];
						$save = TRUE;
					}
				}
			}
			if (isset($current_values[$type]) && empty($current_values[$type])) {
				unset($current_values[$type]);
			}

			if ($indicate_customised) {
				// indicate that this value is customised
				if (count((array_diff($defaults, array_keys($current_values[$type])))) < count($defaults)) {
					$current_values[$type]['customised'] = 1;
				} else {
					$current_values[$type]['customised'] = 0;
				}
			}

			return ($save) ? $asset->setAttrValue($attr_name, $current_values) : FALSE;
		}
		return FALSE;

	}//end _processSpecificCacheManagement()


	/**
	* Processes the interface for customising an individual asset type's cache options
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processCustomiseTypeCodeCaching(&$asset, &$o, $prefix)
	{
		return $this->_processSpecificCacheManagement($asset, $o, $prefix, 'type_codes', $prefix.'_type', FALSE);

	}//end processCustomiseTypeCodeCaching()


	/**
	* Paints the interface for selecting a root node to customise
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintChooseRootNode(&$asset, &$o, $prefix)
	{
		$root_nodes = $asset->attr('root_nodes');
		$wa = $asset->writeAccess('attributes');
		if ($wa && empty($root_nodes)) {
			$root_nodes = Array(0);
		}

		$o->openField('','wide_col');

			$key = 0;
			?>
			<table class="sq-backend-table" id="select-root-node-table">
				<tbody>
					<tr>
						<th>Root node</th>
						<th title="<?php echo translate('include_all_assets'); ?>?"><?php echo translate('cascade'); ?>?</th>
						<th><?php echo translate('customise'); ?></th>
					</tr>
					<?php
					foreach ($root_nodes as $assetid => $data) {
					?>
						<tr>
							<td>
								<?php
									if ($wa) {
										// holds the previous value
										hidden_field($prefix.'_root_node_previous_values['.$key.']', $assetid);
										asset_finder($prefix.'_root_node_X_'.$key.'_X_', $assetid);
									} else {
										echo get_asset_tag_line($assetid);
									}
								?>
							</td>
							<td>
								<?php
									if ($wa) {
										$options = Array(
														0	=> translate('no'),
														1	=> translate('yes'),
												   );
										$combo_name = $prefix.'_all_underneath_X_'.$key.'_X_';
										combo_box($combo_name, $options, FALSE, $data['all_underneath']);
									} else {
										if ($data['all_underneath']) {
											echo '<img src="'.sq_web_path('lib').'/web/images/tick.gif" /> ';
											echo '<strong>'.translate('yes').'</strong> ('.translate('assets_underneath_root_node_affected').')';
										} else {
											echo '<img src="'.sq_web_path('lib').'/web/images/cross.gif" /> ';
											echo '<strong>'.translate('no').'</strong> ('.translate('assets_underneath_root_node_not_affected').')';
										}
									}
								?>
							</td>
							<td>
								<?php
									if (isset($data['customised'])) {
										$customised = $data['customised'];
									} else {
										$customised = FALSE;
									}
									if ($wa && !empty($assetid)) {
										$radio_name = $prefix.'_customise_root_node';
										$radio_id = $prefix.'_'.$assetid.'_customise_root_node';
										//this.form.submit();
										check_box($radio_name, $assetid, FALSE, '', 'id="'.$radio_id.'" onchange="customiseCheck(this, \'select-root-node-table\', false); return false;"');
										if ($customised) {
											label(translate('edit_customisations'), $radio_id);
										} else {
											label(translate('customise'), $radio_id);
										}
									} else {
										if ($customised) {
											$radio_name = $prefix.'_customise_root_node';
											$radio_id = $prefix.'_'.$assetid.'_customise_root_node';
											check_box($radio_name, $assetid, FALSE, '', 'id="'.$radio_id.'"');
											label(translate('view_customisations'), $radio_id);
										} else {
											if (!empty($assetid)) {
												echo translate('not_yet_customised');
											}
										}
									}
								?>
							</td>
						</tr>
						<?php
						if ($wa && $key+1 === count($root_nodes)) {
						?>
							<tr>
								<td colspan="3">
									<?php
										$more_button_name = $prefix.'_root_node_more_button_X_'.$key.'_X_';
										normal_button($more_button_name, translate('add_root_node'), 'var table = prependLastRow(this); resetLastRowFields(table, false)');
									?>
								</td>
							</tr>
						<?php
						}
						$key++;
						//if ($key !== count($root_nodes)) echo '<br />';
					}//end foreach
					?>
				</tbody>
			</table>

				<script type="text/javascript">

					function customiseCheck(theCheckbox, targetID, checktoggle)
					{
						if (theCheckbox.checked) {
						  	var checkboxes = new Array(); 
						  	checkboxes = document.getElementById(targetID).getElementsByTagName('input');
						  	for (var i=0; i<checkboxes.length; i++) {
						    	if (checkboxes[i].type == 'checkbox') {
						      		checkboxes[i].checked = checktoggle;
						    	}
						  	}
							theCheckbox.checked = true;
						}
						return false;
					}

					function prependLastRow(moreButton)
					{
						var lastRow = moreButton.parentNode;
						while (lastRow.tagName != 'TR') {
							lastRow = lastRow.parentNode;
						}

						// to be used when adding the button back
						var lastSibling = lastRow.previousSibling;
						var lastRowButtonCell = moreButton.parentNode;

						// look for sq-backend-data to prepend this row
						var validLocation = moreButton.parentNode;
						while (validLocation.tagName != 'TBODY') {
							validLocation = validLocation.parentNode;
						}

						var clone = lastRow.previousSibling.previousSibling.cloneNode(true);

						// inserting clone after original
						validLocation.insertBefore(clone, lastRow.previousSibling);

						// append the 'More' button to the cloned row
						//lastRowButtonCell.appendChild(moreButton);

						// remove the 'More' button from the original after cloning
						//moreButton.parentNode.removeChild(moreButton);

						// remove the customise button elements from the clone
						var cloneCustomiseCell = clone.cells[2];
						removeElementChildNodes(cloneCustomiseCell);

						// add text in place of the customise button elements in the clone
						var cloneCustomiseText = document.createElement('em');
						cloneCustomiseCell.appendChild(cloneCustomiseText);

						// use innerHTML for IE6
						cloneCustomiseText.innerHTML = js_translate('customise_after_commit');

						// remove the asset finder from the clone
						var cloneAssetFinderCell = clone.cells[0];
						removeElementChildNodes(cloneAssetFinderCell);

						// traverse DOM and increment each field name
						incrementTableFieldNames(clone);

						// add an asset finder for the cloned row
						cloneInnerHTML(lastRow.previousSibling.previousSibling.previousSibling.cells[0], cloneAssetFinderCell);

						return clone;
					}

					// adds a new asset finder as a child of the specified element
					function cloneInnerHTML(cellToCopy, cellToPopulate)
					{
						var htmlToClone = cellToCopy.innerHTML;
						// discover what the new key should be
						var currentKey = htmlToClone.match(/_X_(\d+)_X_/);
						var newKey = parseInt(currentKey[1]) + 1;
						cellToPopulate.innerHTML = htmlToClone.replace(/_X_(\d+)_X_/g, '_X_' + newKey + '_X_');
					}

					// removes all childNodes from the specified element
					function removeElementChildNodes(element)
					{
						if (element.hasChildNodes()) {
							while (element.childNodes.length > 0 ) {
								element.removeChild(element.firstChild);
							}
						}
					}

					function incrementTableFieldNames(element)
					{
						var i=0;
						var elementChildren = element.childNodes[i];
						var tagName = element.tagName;

						if ((tagName == 'INPUT') || (tagName == 'SELECT')) {
							// increment field name
							// have to use the _X_ delimiters because asset_finder cleans form names
							var currentKey = element.name.match(/_X_(\d+)_X_/);
							var newKey = parseInt(currentKey[1]) + 1;
							element.name = element.name.replace(/_X_(\d+)_X_/, '_X_' + newKey + '_X_');
							element.id = element.id.replace(/_X_(\d+)_X_/, '_X_' + newKey + '_X_');
						}

						while (elementChildren) {
							incrementTableFieldNames(elementChildren);
							i++;
							elementChildren = element.childNodes[i];
						}
					}

					function resetLastRowFields(element, findTable)
					{
						// clear all elements inside the table
						if (findTable) {
							var element = element.parentNode;
							while (element.tagName != 'ROW') {
								element = element.parentNode;
							}
						}

						var i=0;
						var elementChildren = element.childNodes[i];
						var tagName = element.tagName;

						if (tagName == 'INPUT') {
							// don't clear buttons
							if (element.type != 'button') {
								element.value = '';
							}
							return;
						} else if (tagName == 'SELECT') {
							element.selectedIndex = 0;
							return;
						}

						while (elementChildren) {
							resetLastRowFields(elementChildren, false);
							i++;
							elementChildren = element.childNodes[i];
						}
					}
				</script>
			<?php

		$o->closeField();

		return TRUE;

	}//end paintChooseRootNode()


	/**
	* Processes the interface for selecting a root node to customise
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processChooseRootNode(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			$root_node_key = $prefix.'_root_node';
			$submitted_root_nodes = Array();
			$deleteme = Array();
			foreach ($_POST as $key => $value) {
				// look for form names which have _X_ delimiters
				if (strpos($key, '_X_')) {
					// retrieve the current index value
					if (preg_match("/^$root_node_key".'_X_(\d+)_X_/', $key, $matches)) {
						$current_row = $matches[1];
						if (isset($_POST[$prefix.'_root_node_previous_values'][$current_row])) {
							$previous_value = $_POST[$prefix.'_root_node_previous_values'][$current_row];
						} else {
							// populate the previous value with the actual value when it is retrieved
							// this is because when new root nodes are created, they do not have a previous value
							$previous_value = '';
						}
					}

					// retrieve the actual values submitted
					if (preg_match("/^$root_node_key/", $key)) {
						$root_node = htmlspecialchars($_POST[$key]['assetid']);
						// if no root node was submitted, skip it and mark for deletion
						if (!$root_node) {
							// record the previous assetid (i.e. the one to delete)
							$deleteme[] = $previous_value;
							continue;
						}

						if (empty($previous_value)) {
							$submitted_root_nodes[$root_node]['previous'] = $root_node;
						} else {
							$submitted_root_nodes[$root_node]['previous'] = $previous_value;
						}
					} else if (isset($submitted_root_nodes[$root_node]) && strpos($key, 'all_underneath')) {
						$all_underneath = $_POST[$key];
						$submitted_root_nodes[$root_node]['all_underneath'] = $all_underneath;
					}
				}
			}

			// compare submitted values with stored ones and update/delete accordingly
			$current_root_nodes = $asset->attr('root_nodes');
			if (!empty($submitted_root_nodes)) {
				foreach ($submitted_root_nodes as $assetid => $data) {
					if (isset($current_root_nodes[$assetid])) {
						// updating this existing node's options
						foreach ($current_root_nodes[$assetid] as $key => $value) {
							if (isset($data[$key])) {
								$current_root_nodes[$assetid][$key] = $data[$key];
							} else {
								// submitted data does not have this key so use the existing value
								$current_root_nodes[$assetid][$key] = $value;
							}
						}
					} else if ($assetid !== $data['previous'] && isset($current_root_nodes[$data['previous']])) {
						// updating this existing's nodes assetid
						$old_values = $current_root_nodes[$data['previous']];
						$current_root_nodes[$assetid] = $old_values;
						unset($current_root_nodes[$data['previous']]);
					} else if (!empty($submitted_root_nodes[$assetid])) {
						// this value does not exist so create it
						$current_root_nodes[$assetid] = $data;
						// do not show root node cache management section
						unset($_REQUEST[$prefix.'_customise_root_node']);
					}
				}

				// delete entries marked for deletion
				if (!empty($deleteme)) {
					foreach($deleteme as $row => $assetid) {
						unset($current_root_nodes[$assetid]);
					}
				}

				// do not need 'previous' anymore
				if (isset($current_root_nodes[$assetid]['previous'])) {
					unset($current_root_nodes[$assetid]['previous']);
				}
			} else {
				// delete them all
				$current_root_nodes = Array();
			}
			$asset->setAttrValue('root_nodes', $current_root_nodes);
		}
		return $wa;

	}//end processChooseRootNode()


	/**
	* Paints the interface for customising an individual root node's cache options
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintCustomiseRootNodeCaching(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');

		// get the assetid to customise
		if (isset($_REQUEST[$prefix.'_customise_root_node'])) {
			$this->_paintSpecificCacheManagement($asset, $o, $prefix, 'root_nodes', $_REQUEST[$prefix.'_customise_root_node'], TRUE);
		} else {
			if ($wa) {
				echo '<em>'.translate('customise_root_node_selection_below').'</em>';
			} else {
				echo '<em>'.translate('view_root_node_selection_below').'</em>';
			}
		}

		return $wa;

	}//end paintCustomiseRootNodeCaching()


	/**
	* Processes the interface for customising an individual root node's cache options
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processCustomiseRootNodeCaching(&$asset, &$o, $prefix)
	{
		if (isset($_REQUEST[$prefix.'_customise_root_node'])) {
			$o->addFormActionGetVar($prefix.'_customise_root_node', $_REQUEST[$prefix.'_customise_root_node'], TRUE);

			// first check if the value still exists
			if (in_array($_REQUEST[$prefix.'_customise_root_node'], array_keys($asset->attr('root_nodes')))) {
				return $this->_processSpecificCacheManagement($asset, $o, $prefix, 'root_nodes', $prefix.'_customise_root_node', TRUE);
			} else {
				return FALSE;
			}
		}

	}//end processCustomiseRootNodeCaching()


//--        CLEAR CACHE        --//


	/**
	* Paints the interface for clearing the cache files
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintClearCache(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			echo translate('cannot_clear_cache');
			return FALSE;
		}

		if (!$asset->canClearByAssetid()) {
			echo translate('cannot_clear_cache_by_assetid');
			return FALSE;
		}

		multiple_asset_finder($prefix.'_clear_cache', Array());

		$o->openField(translate('level'));
			$options = Array(
						'single'		=> translate('this_asset'),
						'dependants'	=> translate('this_asset_and_dependants'),
						'children'		=> translate('this_asset_and_children'),
					   );
			combo_box($prefix.'_level', $options, '', FALSE);
		$o->closeField();

		$o->openField(translate('asset_types'));
			$type_options = Array();
			$asset_types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();
			foreach ($asset_types as $type_code) {
				$info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
				if ($info['allowed_access'] != 'system' && $info['instantiable']) {
					$type_options[$type_code] = $info['name'];
				}
			}

			asort($type_options);
			$type_options = array_reverse($type_options, TRUE);
			$type_options[''] = '-- '.translate('all_asset_types').' --';
			$type_options = array_reverse($type_options, TRUE);
			combo_box($prefix.'_type_codes', $type_options, TRUE, '', 8);
		$o->closeField();

		return TRUE;

	}//end paintClearCache()


	/**
	* Processes the interface for clearing the cache files, and starts the clear cache hipo
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processClearCache(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_clear_cache']) && $_POST[$prefix.'_delete_all'] != 'yes') {
			$level = (isset($_POST[$prefix.'_level'])) ? $_POST[$prefix.'_level'] : 'single';

			if (isset($_POST[$prefix.'_type_codes']) && !empty($_POST[$prefix.'_type_codes'][0])) {
				$type_codes = $_POST[$prefix.'_type_codes'];
			} else {
				$type_codes = Array();
			}

			$assets_info	= array_get_index($_POST, $prefix.'_clear_cache', Array());
			$assetids		= Array();
			foreach ($assets_info as $asset_info) {
				if (!empty($asset_info['assetid'])) {
					$assetids[] = $asset_info['assetid'];
				}//end if
			}//end foreach

			$delete_all = $_POST[$prefix.'_delete_all'];

			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array(
						'assetid'		=> $assetids,
						'level'			=> $level,
						'type_codes'	=> $type_codes,
						'delete_all'	=> $delete_all,
					);
			$hh->queueHipo('hipo_job_clear_cache', $vars);

		}

		return TRUE;

	}//end processClearCache()


	/**
	* Paints the interface for clearing the cache files
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintSystemWideClearCache(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			echo translate('cannot_clear_cache');
			return FALSE;
		}

		$o->openField(translate('delete_all_cache'));
			$delete_all_ops = Array(
								'yes'	=> translate('yes'),
								'no'	=> translate('no'),
							  );
			combo_box($prefix.'_delete_all', $delete_all_ops, FALSE, 'no', 0, "onChange=\"Javascript: if (this.value == 'yes') { if(!confirm(js_translate('confirm_purge_cache'))) { this.value = 'no'; } } \"");
			$o->note(translate('trash_cache_warning'));
		$o->closeField();

		return TRUE;

	}//end paintSystemWideClearCache()


	/**
	* Processes the interface for clearing the cache files, and starts the clear cache hipo
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processSystemWideClearCache(&$asset, &$o, $prefix)
	{
		if ($_POST[$prefix.'_delete_all'] == 'yes') {
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array(
						'delete_all'	=> 'yes',
					);
			$hh->queueHipo('hipo_job_clear_cache', $vars);
		}

		return TRUE;

	}//end processSystemWideClearCache()


//--        REFRESH CACHE        --//


	/**
	* Paints the interface for refreshing asset cache
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintAddJob(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$caching_on = $asset->attr('enabled');
		if ($wa) {
			if ($this->cronConfigured()) {
				$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');

				// Include Refresh Cache Cron Job
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_refresh_cache');

				// Caching is off. Show the warning only
				if (!$caching_on) {
					?>
					<span class="sq-backend-warning"><b>NOTE:</b> The caching is turned off at the moment. Although you can add a job here, it will not have any effect</span><br /><br />
					<?php
				}

				require_once SQ_FUDGE_PATH.'/general/datetime.inc';
				$cron_job = new Cron_Job_Refresh_Cache();
				$edit_fns = $cron_job->getEditFns();
				$edit_fns->paintInLineBackend($cron_job, $o, $prefix.'_new_rc');
				$o->note(translate('cron_manager_next_run', $cron_mgr->readableRefreshTime(), readable_datetime($cron_mgr->timeOfNextRun())));

			} else {
				// Cron Manager is not configured yet.
				echo translate('cron_manager_not_configured_refresh');
			}
		}

		return TRUE;

	}//end paintAddJob()


	/**
	* Processes the interface for refreshing asset cache
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processAddJob(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if ($wa && $this->cronConfigured()) {
			$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');

			// Include Refresh Cache Cron Job
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_refresh_cache');

			if ($asset->writeAccess('')) {
				$rc = new Cron_Job_Refresh_Cache();
				$edit_fns = $rc->getEditFns();
				if ($edit_fns->processInLineBackend($rc, $o, $prefix.'_new_rc')) {
					$rc->saveAttributes();
				} else {
					return FALSE;
				}

				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				if ($cron_mgr->addJob($rc, $GLOBALS['SQ_SYSTEM']->user)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				} else {
					trigger_localised_error('CRON0054', E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return FALSE;
				}
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			}

		} else {
			// Cron Manager not configured
			return FALSE;
		}

		return TRUE;

	}//end processAddJob()


	/**
	* Paints the currently scheduled jobs
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintCurrentJobs(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_refresh_cache');
		$rc_jobs = $cron_mgr->getJobs('cron_job_refresh_cache');

		if (empty($rc_jobs)) {
			echo translate('cron_manager_refresh_no_job');
		} else {
?>
			<table class="sq-backend-table">
				<tr>
					<th class="sq-backend-table-header">When</th>
					<th class="sq-backend-table-header">Status</th>
					<th class="sq-backend-table-header">Run As</th>
					<th class="sq-backend-table-header">Delete</th>
				</tr>
				<?php
				foreach ($rc_jobs as $rc_job) {
					?>
					<tr>
						<td class="sq-backend-table-cell">
						<?php
							echo get_asset_tag_line($rc_job->attr('asset'));
							echo '<br /><b>Scheduled At:&nbsp;</b>';
							echo $rc_job->readableWhen();
						?>
						</td>
						<td class="sq-backend-table-cell">
						<?php
							if ($rc_job->attr('long') && $rc_job->attr('running')) {
								echo translate('cron_manager_refresh_cache_running');
							} else {
								echo translate('cron_manager_refresh_cache_not_running');
							}
						?>
						</td>
						<td class="sq-backend-table-cell">
						<?php
							$user = $rc_job->getRunningUser();
							echo get_asset_tag_line($user->id);
							unset($user);
						?>
						</td>
						<td class="sq-backend-table-cell">
						<?php
							if ($wa) {
								check_box($prefix.'_remove_job_'.$rc_job->id);
							}
						?>
						</td>
					</tr>
					<?php
				}//end foreach
				?>
			</table>
			<?php
		}//end else
		return TRUE;

	}//end paintCurrentJobs()


	/**
	* Processes the currently scheduled jobs
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processCurrentJobs(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if ($wa && $this->cronConfigured()) {
			$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');

			// Include Refresh Cache Cron Job
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_refresh_cache');

			// Any job delete?
			$jobs = $cron_mgr->getJobs('cron_job_refresh_cache');
			for ($i = 0; $i < count($jobs); $i++) {
				if (!empty($_POST[$prefix.'_remove_job_'.($jobs[$i]->id)])) {

					if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($cron_mgr->id, 'links')) {
						if (!$cron_mgr->removeJob($jobs[$i])) {
							trigger_localised_error('CRON0055', E_USER_WARNING);
						}
						$GLOBALS['SQ_SYSTEM']->am->releaseLock($cron_mgr->id, 'links');
					}
				}
			}//end for delete job
		}

		return TRUE;

	}//end processCurrentJobs()


	/**
	* Returns TRUE if the cron is configured in the system
	*
	* @return boolean
	* @access public
	*/
	function cronConfigured()
	{
		$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		$cron_epoc = (int)$cron_mgr->attr('epoch');
		$cron_refresh = (int)$cron_mgr->attr('refresh_time');

		return ($cron_epoc > 0) && ($cron_refresh > 0);

	}//end cronConfigured()


//--        DETAILS        --//


	/**
	* Paint the interface for choosing cachable root URLs
	*
	* @param object	&$asset	the asset we are working on
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintCachableRootURLs(&$asset, &$o, $prefix)
	{
		$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
		$non_cachable_urls = $asset->attr('non_cachable_urls');
		$wa = $asset->writeAccess('attributes');
		?>
		<table class="sq-backend-table" style="width: auto">
			<thead>
				<tr>
					<th><?php echo translate('url'); ?></th>
					<th><?php echo translate('use_cache'); ?></th>
				</tr>
			</thead>
			<tbody>
			<?php
			foreach ($root_urls as $i => $url) {
				?>
				<tr>
					<td><?php echo $url; ?></td>
					<td>
						<?php
						if ($wa) {
							check_box($prefix.'_url_'.$i, 1, !in_array($url, $non_cachable_urls));
						} else {
							$filename = in_array($url, $non_cachable_urls) ? 'cross' : 'tick';
							echo '<img src="'.sq_web_path('lib').'/web/images/'.$filename.'.gif" />';
						}
						?>
					</td>
				</tr>
				<?php
			}
			?>
			</tbody>
		</table>
		<?php

	}//end paintCachableRootURLs()


	/**
	* Process the interface for choosing cachable root URLs
	*
	* @param object	&$asset	the asset we are working on
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processCachableRootURLs(&$asset, &$o, $prefix)
	{
		$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
		$cachable = Array();
		foreach ($root_urls as $i => $url) {
			if (!empty($_POST[$prefix.'_url_'.$i])) {
				$cachable[] = $url;
			}
		}
		// Note that we save the *non-cachable* root URLs, so that any new URLs are
		// cachable by default.
		$non_cachable = array_diff($root_urls, $cachable);
		return $asset->setAttrValue('non_cachable_urls', $non_cachable);

	}//end processCachableRootURLs()


	/**
	* Paint the interface for choosing the cache storage type
	*
	* @param object	&$asset	the asset we are working on
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintCacheStorageType(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$current_type = $asset->attr('cache_storage_type');

		$storage_types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('cache_storage');
		$names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($storage_types, 'name');

		// Eliminate those cache storage types that are not available
		// based on the current settings
		foreach ($storage_types as $storage_type) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($storage_type);
			if (eval('return '.$storage_type.'::isAvailable();') === FALSE) {
				unset($names[$storage_type]);
			}
		}

		// If the current type has been eliminated, revert to the default
		if (isset($names['cache_storage_'.$current_type]) === FALSE) {
			$current_type = 'default';
		}

		if ($wa) {
			combo_box($prefix.'_cache_storage_type', $names, FALSE, 'cache_storage_'.$current_type);
		} else {
			echo $names['cache_storage_'.$current_type];
		}

	}//end paintCacheStorageType()


	/**
	* Process the interface for choosing the cache storage type
	*
	* @param object	&$asset	the asset we are working on
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processCacheStorageType(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes') === FALSE) return FALSE;

		$type = $_POST[$prefix.'_cache_storage_type'];
		$type = substr($type, strlen('cache_storage_'));
		return $asset->setAttrValue('cache_storage_type', $type);

	}//end processCacheStorageType()


//--        ROOT URL SPECEFIC        --//


	/**
	* Paint the interface for choosing root URLs that would send
	* cacheable headers regardless of other Matrix cache settings
	*
	* @param object	&$asset	the asset we are working on
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintCacheableHeadersRootURLs(&$asset, &$o, $prefix)
	{
		$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
		$cacheable_header_urls = $asset->attr('non_cacheable_header_urls');
		$cacheable_user_header_urls = $asset->attr('user_cacheable_header_urls');
		$wa = $asset->writeAccess('attributes');
		?>
		<table class="sq-backend-table" style="width: auto">
			<thead>
				<tr>
					<th><?php echo translate('url'); ?></th>
					<th><?php echo "Send Headers"; ?></th>
					<th><?php echo "Public User Only"; ?></th>
				</tr>
			</thead>
			<tbody>
			<?php
			foreach ($root_urls as $i => $url) {
				?>
				<tr>
					<td><?php echo $url; ?></td>
					<td>
						<?php
						if ($wa) {
							check_box($prefix.'_url_'.$i, 1, !in_array($url, $cacheable_header_urls));
						} else {
							$filename = in_array($url, $cacheable_header_urls) ? 'cross' : 'tick';
							echo '<img src="'.sq_web_path('lib').'/web/images/'.$filename.'.gif" />';
						}
						?>
					</td>
					<td>
						<?php
						if ($wa) {
							check_box($prefix.'_user_url_'.$i, 1, !in_array($url, $cacheable_user_header_urls));
						} else {
							$filename = in_array($url, $cacheable_user_header_urls) ? 'cross' : 'tick';
							echo '<img src="'.sq_web_path('lib').'/web/images/'.$filename.'.gif" />';
						}
						?>
					</td>
				</tr>
				<?php
			}
			?>
			</tbody>
		</table>
		<?php

		return $wa;

	}//end processCacheableHeadersRootURLs()


	/**
	* Process the interface for root URLs to send cacheable URLS
	* regardless of other Matrix cahce settings
	*
	* @param object	&$asset	the asset we are working on
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processCacheableHeadersRootURLs(&$asset, &$o, $prefix)
	{
		$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
		$cachable = Array();
		foreach ($root_urls as $i => $url) {
			if (!empty($_POST[$prefix.'_url_'.$i])) {
				$cachable[] = $url;
			}
		}
		// NOTE: Though it might look little confusing, we save the root URLs that shouldn't send cacheable headers,
		// so that any new URLs send cacheable headers by default.
		$non_cachable = array_diff($root_urls, $cachable);

		$user_cachable = Array();
		foreach ($root_urls as $i => $url) {
			if (isset($_POST[$prefix.'_user_url_'.$i]) && !empty($_POST[$prefix.'_user_url_'.$i])) {
				$user_cachable[] = $url;
			}
		}
		// save urls that we will send headers for ALL users
		$user_cacheable = array_diff($root_urls, $user_cachable);
		$asset->setAttrValue('user_cacheable_header_urls', $user_cacheable);

		return $asset->setAttrValue('non_cacheable_header_urls', $non_cachable);

	}//end processCacheableHeadersRootURLs()


	/**
	* Displays the current "Send Cacheable Header" settings
	*
	* @param object	&$asset	the asset we are working on
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintCacheableHeadersSettings(&$asset, &$o, $prefix)
	{
        $o->openField(translate('http_header_cacheable_field'));
            echo (SQ_CONF_SEND_CACHEABLE_HEADER) ? translate('yes') : translate('no');
        $o->note(translate('cache_manager_send_cacheable_headers_settings_node'));
        $o->closeField();
	}


}//end class

?>
