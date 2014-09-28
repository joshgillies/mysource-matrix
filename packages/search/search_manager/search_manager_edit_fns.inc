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
* $Id: search_manager_edit_fns.inc,v 1.30 2012/08/30 00:59:15 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Search_Manager_Edit_Fns
*
* Edit functions for the search manager.
*
* @see Asset_Edit_Fns
*
* @author Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.30 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Manager_Edit_Fns extends Asset_Edit_Fns
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
		unset($this->static_screens['metadata']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['workflow']);
		unset($this->static_screens['tagging']);
		unset($this->static_screens['dependants']);
		unset($this->static_screens['linking']);
		unset($this->static_screens['roles']);
		unset($this->static_screens['permissions']);
		unset($this->static_screens['layouts']);

	}//end constructor


	/**
	* Paints the interface for customising the assets to be indexed
	*
	* @param object	&$asset	the search manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintAssetWeights(&$asset, &$o, $prefix)
	{
		$customised = array_keys($asset->getAssetWeightings());
		return $this->_paintWeightInterface($asset, $o, $prefix, 'asset', $customised);

	}//end paintAssetWeights()


	/**
	* Processes the interface for customising the assets to be indexed
	*
	* @param object	&$asset	the search manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processAssetWeights(&$asset, &$o, $prefix)
	{
		return $this->_processWeights($asset, $o, $prefix, 'asset');

	}//end processAssetWeights()


	/**
	* Paints the interface for customising the asset trees to be indexed
	*
	* @param object	&$asset	the search manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintAssetTreeWeights(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$assetid = $this->_getCurrentCustomisation($prefix, 'asset_tree');

		$global_customised = $asset->getAssetTreeWeightings();
		// remove the root folder from the weightings as these are the global weightings
		$root_folder_assetid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('root_folder');
		if (isset($global_customised[$root_folder_assetid])) {
			unset($global_customised[$root_folder_assetid]);
		}

		$this->_paintCurrentCustomisations($asset, $o, $prefix, array_keys($global_customised), 'asset');
		$this->_paintWeightingSourceInterface($asset, $o, $prefix, 'asset');

		if (!empty($assetid)) {
			return $this->_paintTreeWeightInterface($asset, $o, $prefix, $assetid);
		}

		return $write_access;

	}//end paintAssetTreeWeights()


	/**
	* Processes the interface for customising the asset trees to be indexed
	*
	* @param object	&$asset	the search manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processAssetTreeWeights(&$asset, &$o, $prefix)
	{
		if ($this->_usingGoButton($prefix)) return FALSE;
		return $this->_processWeights($asset, $o, $prefix, 'asset_tree');

	}//end processAssetTreeWeights()


	/**
	* Paints the interface for customising the assets to be indexed
	*
	* @param object	&$asset	the search manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintMetadataWeights(&$asset, &$o, $prefix)
	{
		$weights = $asset->getMetadataWeightings();
		return $this->_paintMetadataWeightInterface($asset, $o, $prefix, $weights);

	}//end paintMetadataWeights()


	/**
	* Processes the interface for customising the assets to be indexed
	*
	* @param object	&$asset	the search manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processMetadataWeights(&$asset, &$o, $prefix)
	{
		$weights = $asset->getMetadataWeightings();
		$this->_processMetadataWeights($prefix, $weights);
		$asset->saveMetadataWeightings($weights);

		return FALSE;

	}//end processMetadataWeights()


	/**
	* Paints the interface for customising the global weightings
	*
	* @param object	&$asset	the search manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintGlobalWeights(&$asset, &$o, $prefix)
	{
		// for global weightings we store them as tree weightings under the root folder assetid
		$root_folder_assetid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('root_folder');
		$global_customised = $asset->getAssetTreeWeightings($root_folder_assetid);
		$customised = (isset($global_customised['type_weights'])) ? array_keys($global_customised['type_weights']) : Array();
		$this->_paintCurrentCustomisations($asset, $o, $prefix, $customised, 'asset_type');
		return $this->_paintTreeWeightInterface($asset, $o, $prefix, $root_folder_assetid, FALSE);

	}//end paintGlobalWeights()


	/**
	* Processes the interface for customising the global weightings
	*
	* @param object	&$asset	the search manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processGlobalWeights(&$asset, &$o, $prefix)
	{
		if ($this->_usingGoButton($prefix)) return FALSE;
		return $this->_processWeights($asset, $o, $prefix, 'asset_tree');

	}//end processGlobalWeights()


	/**
	* Paints the interface for customising tree weightings for the specified asset
	*
	* @param object		&$asset				the search manager asset
	* @param object		&$o					the backend outputter
	* @param string		$prefix				the prefix for fields
	* @param string		$assetid			the assetid of the tree root
	* @param boolean	$paint_customised	Paint customsised
	*
	* @return boolean
	* @access private
	*/
	function _paintTreeWeightInterface(&$asset, &$o, $prefix, $assetid, $paint_customised=TRUE)
	{
		$write_access = $asset->writeAccess('attributes');
		$tree_type = array_get_index($_POST, $prefix.'_selected_tree_type', '');

		if ($this->_customisationsRemoved($prefix, 'asset_tree')) {
			return $write_access;
		}
		$o->addHiddenField($prefix.'_current_assetid', $assetid);
		// check to see if this tree has been customised before - if so
		// we can bring up the details for this tree
		$weights   = $asset->getAssetTreeWeightings($assetid);
		$tree_type = array_get_index($_POST, $prefix.'_selected_tree_type', '');

		// section opened by xml
			$o->openField(translate('Customise').'?');

				$types = Array(
							'asset_type'	=> translate('Asset Types'),

							'Metadata'		=> translate('Metadata'),

						 );
				combo_box($prefix.'_selected_tree_type', $types, FALSE, $tree_type);
				echo '&nbsp;';
				submit_button($prefix.'_selected_tree_type_go', translate('Go'), 'main_form.submit()');

			$o->closeField();

		// check to see if they have chosen to customise a different tree asset than the current
		// one, if so we want to clear the current interface from here down
		$key = array_get_index($_POST, $prefix.'_current_assetid', '');
		if (!empty($key) && $this->_getCurrentCustomisation($prefix, 'asset') != $key) {
			return $write_access;
		}

		if (!empty($tree_type)) {
			if ($tree_type == 'asset_type') {
				$type_code = $this->_getCurrentCustomisation($prefix, 'asset_type');
				if ($paint_customised) {
					$customised = isset($weights['type_weights']) ? array_keys($weights['type_weights']) : Array();
				} else {
					$customised = Array();
				}
				$weights = isset($weights['type_weights'][$type_code]) ? $weights['type_weights'][$type_code] : Array();
				return $this->_paintWeightInterface($asset, $o, $prefix, $tree_type, $customised, $weights);
			} else {
				$metadata_weights = array_get_index($weights, 'metadata_weights', Array());
				return $this->_paintMetadataWeightInterface($asset, $o, $prefix, $metadata_weights);
			}
		}
		return $write_access;

	}//end _paintTreeWeightInterface()


	/**
	* Prints the interface used to specify the weights for asset components
	*
	* @param object	&$asset		the search_manager asset
	* @param object	&$o			the backend_outputer
	* @param string	$prefix		the prefix for fields
	* @param array	$weights	the asset weight values
	*
	* @return boolean
	* @access private
	*/
	function _paintAssetWeightInterface(&$asset, &$o, $prefix, $weights)
	{
		$write_access = $asset->writeAccess('attributes');
		$o->openSection(translate('Asset Field Weightings'));

			$o->openField('');
				?>
				<table class="sq-backend-table" style="width:50%">
					<tr>
						<td width="250" class="sq-backend-table-header"><?php echo translate('Weighting Field'); ?></td>

						<td class="sq-backend-table-header"><?php echo translate('Weighting'); ?></td>

						<td class="sq-backend-table-header"><?php echo translate('Indexed'); ?>?</td>

					</tr>
					<?php

					$components = $asset->getIndexableAssetComponents();

					foreach ($components as $comp => $name) {
						$weight  = (isset($weights[$comp]['weight'])) ? $weights[$comp]['weight'] : 1;
						$indexed = (isset($weights[$comp]['indexed'])) ? $weights[$comp]['indexed'] : 1;
						?>
						<tr>
							<td class="sq-backend-table-cell">
								<?php echo $name; ?>
							</td>
							<td class="sq-backend-table-cell">
								<?php
									if ($write_access) {
										text_box($prefix.'_asset_weights['.$comp.'][weight]', $weight, 5);
									} else {
										echo $weight;
									}
								?>
							</td>
							<td class="sq-backend-table-cell">
								<?php
									if ($write_access) {
										check_box($prefix.'_asset_weights['.$comp.'][indexed]', 1, $indexed);
									} else {
										echo ($indexed) ? translate('Yes') : translate('No');


									}
								?>
							</td>
						</tr>
						<?php
					}
					?>
				</table>
				<?php
			$o->closeField();
		$o->closeSection();

		return $write_access;

	}//end _paintAssetWeightInterface()


	/**
	* Prints the interface used to specify the weights for attributes
	*
	* @param object	&$asset		the search_manager asset
	* @param object	&$o			the backend_outputer
	* @param string	$prefix		the prefix for fields
	* @param array	$weights	the attribute weight values
	* @param string	$type_code	the type code that the attributes belong to
	*
	* @return boolean
	* @access private
	*/
	function _paintAttributeWeightInterface(&$asset, &$o, $prefix, $weights, $type_code)
	{
		$write_access = $asset->writeAccess('attributes');
		$attrs        = $asset->getIndexableAttributes($type_code);

		if (!empty($attrs)) {
			$o->openSection(translate('Asset Attribute Weightings'));

				$o->openField('');
					?>
						<table class="sq-backend-table" style="width:50%">
							<tr>
								<td width="250" class="sq-backend-table-header"><?php echo translate('Weighting Attribute'); ?></td>

								<td class="sq-backend-table-header"><?php echo translate('Weighting'); ?></td>

								<td class="sq-backend-table-header"><?php echo translate('Indexed'); ?>?</td>

							</tr>
							<?php

							foreach ($attrs as $name => $info) {

								// don't accept weightings for names and shortnames as we have these
								// as asset components
								if ($name == 'name' || $name == 'short_name') {
									continue;
								}

								$type    = $info['type'];
								$weight  = (isset($weights[$name]['weight'])) ? $weights[$name]['weight'] : 1;
								$indexed = (isset($weights[$name]['indexed'])) ? $weights[$name]['indexed'] : 1;
								?>
								<tr>
									<td class="sq-backend-table-cell">
										<?php echo ucwords(str_replace('_', ' ', $name)); ?>
									</td>
									<td class="sq-backend-table-cell">
										<?php
											if ($write_access) {
												text_box($prefix.'_attr_weights['.$name.'][weight]', $weight, 5);
											} else {
												echo $weight;
											}
										?>
									</td>
									<td class="sq-backend-table-cell">
										<?php
											if ($write_access) {
												check_box($prefix.'_attr_weights['.$name.'][indexed]', 1, $indexed);
											} else {
												echo ($indexed) ? translate('Yes') : translate('No');


											}
										?>
									</td>
								</tr>
								<?php
							}//end foreach attr
							?>
						</table>
					<?php
				$o->closeField();
			$o->closeSection();
		}//end if !empty(attrs)

		return $write_access;

	}//end _paintAttributeWeightInterface()


	/**
	* Prints the interface used to customise metadata weightings
	*
	* @param object	&$asset		the search_manager asset
	* @param object	&$o			the backend_outputer
	* @param string	$prefix		the prefix for fields
	* @param array	$weights	the attribute weight values
	*
	* @return boolean
	* @access private
	*/
	function _paintMetadataWeightInterface(&$asset, &$o, $prefix, $weights)
	{
		$write_access = $asset->writeAccess('attributes');
		$valid_types = Array(
						'metadata_schema'	=> 'I',
						'metadata_section'	=> 'I',
						'metadata_field'	=> 'D',
					   );
		if ($write_access) {
			$o->openSection(translate('Metadata Weightings'));

				$o->openField('');
					?>
					<table class="sq-backend-table" style="width:60%">
						<tr>
							<td class="sq-backend-table-header"><?php echo translate('Metadata Asset'); ?></td>

							<td class="sq-backend-table-cell">
								<?php asset_finder($prefix.'_metadata_asset', 0, $valid_types); ?>
							</td>
						</tr>
						<tr>
							<td class="sq-backend-table-header"><?php echo translate('Weighting'); ?></td>

							<td class="sq-backend-table-cell">
								<?php text_box($prefix.'_metadata_weight', 1, 5); ?>
							</td>
						</tr>
					</table>
					<?php
				$o->closeField();
			$o->closeSection();
		}

		$o->openSection(translate('Metadata Asset Weightings'));

			$o->openField('');
				if (!empty($weights)) {
					?>
						<table class="sq-backend-table" style="width:60%">
							<tr>
								<td class="sq-backend-table-header"><?php echo translate('Assetid'); ?></td>

								<td class="sq-backend-table-header"><?php echo translate('Metadata Asset Name'); ?></td>

								<td class="sq-backend-table-header"><?php echo translate('Asset Type'); ?></td>

								<td class="sq-backend-table-header"><?php echo translate('Weighting'); ?></td>

								<td class="sq-backend-table-header"><?php echo translate('Indexed'); ?>?</td>

								<?php
									if ($write_access) {
										echo '<td class="sq-backend-table-header">'.translate('Remove').'?</td>';

									}
								?>
							</tr>
							<?php

							foreach ($weights as $assetid => $info) {

								$type_name  = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($info['type_code'], 'name');
								$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid), Array(), TRUE, 'name');
								$indexed    = (isset($weights[$assetid]['indexed'])) ? $weights[$assetid]['indexed'] : 1;
								$asset_name = !empty($asset_info[$assetid]) ? $asset_info[$assetid] : "<span style='color:red'>(".translate('Asset not found').")</span>";

								?>
								<tr>
									<td class="sq-backend-table-cell">
										<?php echo $assetid; ?>
									</td>
									<td class="sq-backend-table-cell">
										<?php echo $asset_name; ?>
									</td>
									<td class="sq-backend-table-cell" border="0">
										<?php echo $type_name; ?>
									</td>
									<td class="sq-backend-table-cell">
										<?php
											if ($write_access) {
												text_box($prefix.'_metadata_assets['.$assetid.'][weight]', $info['weight'], 5);
											} else {
												echo $info['weight'];
											}
										?>
									</td>
									<td class="sq-backend-table-cell">
										<?php
											if ($write_access) {
												echo check_box($prefix.'_metadata_assets['.$assetid.'][indexed]', 1, $indexed);
											} else {
												echo ($indexed) ? translate('Yes') : translate('No');


											}
										?>
									</td>
										<?php
											if ($write_access) {
												echo '<td class="sq-backend-table-cell">';
												echo check_box($prefix.'_metadata_assets['.$assetid.'][remove]', 1, FALSE);
												echo '</td>';
											}
										?>
								</tr>
								<?php
							}//end foreach weights
							?>
						</table>
					<?php
				} else {
					echo translate('Currently no metadata assets customised');

				}
			$o->closeField();
		$o->closeSection();

		return $write_access;

	}//end _paintMetadataWeightInterface()


	/**
	* Paints the interface for choosing an asset type.
	*
	* Only non-system assets and instantiable assets and their ancestors are printed
	*
	* @param string	$field_name			the name to use for the combo box
	* @param string	$selected_type_code	the type_code that should be currently selected
	*
	* @return void
	* @access private
	*/
	function _paintAssetTypeChooser($field_name, $selected_type_code)
	{
		$asset_types = $GLOBALS['SQ_SYSTEM']->am->getAssetTypes(TRUE);

		$all_types = Array();
		foreach ($asset_types as $type_code => $info) {
			$all_types[$type_code] = $info['name'];
			$type_anc = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type_code);

			foreach ($type_anc as $anc_type_code) {
				if ($anc_type_code == 'asset') continue;
				// save on db calls if we have this ancestor already
				if (!in_array($anc_type_code, array_keys($all_types))) {
					$all_types[$anc_type_code] = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($anc_type_code, 'name');
				}
			}
		}
		$all_types = array_unique($all_types);
		arsort($all_types);
		$all_types['asset'] = '-- '.translate('All Asset Types').' --';

		$all_types[''] = '';
		$all_types = array_reverse($all_types);

		combo_box($field_name, $all_types, FALSE, $selected_type_code);
		echo '&nbsp;';
		submit_button($field_name.'_go', translate('Go'), 'main_form.submit()');


	}//end _paintAssetTypeChooser()


	/**
	* Paints the interface for choosing a source to customise
	*
	* @param object	&$asset			the search_manager asset
	* @param object	&$o				the backend_outputer
	* @param string	$prefix			the prefix for fields
	* @param string	$screen			the screen being processed
	* @param string	$field_suffix	the suffix to append to the field name
	*
	* @return void
	* @access private
	*/
	function _paintWeightingSourceInterface(&$asset, &$o, $prefix, $screen, $field_suffix='')
	{
		$write_access = $asset->writeAccess('attributes');
		if ($screen == 'asset_type') {
			$o->openField(translate('Customise Asset Types'));

				$current_customisation = $this->_getCurrentCustomisation($prefix, 'asset_type');
				if ($write_access) {
					$this->_paintAssetTypeChooser($prefix.'_selected_type_code'.$field_suffix, $current_customisation);
				} else {
					if (!empty($current_customisation)) {
						if ($current_customisation == 'asset') {
							echo translate('All Asset Types');

						} else {
							echo $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($current_customisation, 'name');
						}
					}
				}
			$o->closeField();
		} else if ($screen == 'asset') {
			if ($write_access) {
				$o->openField(translate('Customise Asset'));

					$current_customisation = $this->_getCurrentCustomisation($prefix, 'asset');
					asset_finder($prefix.'_selected_assetid'.$field_suffix, $current_customisation, Array());
				$o->closeField();
			}
		}

	}//end _paintWeightingSourceInterface()


	/**
	* Paints an interface with the current cusomisations for the specified weights and screen
	*
	* @param object	&$asset		the search_manager asset
	* @param object	&$o			the backend_outputer
	* @param string	$prefix		the prefix for fields
	* @param array	$weights	the weights
	* @param string	$screen		the current screen
	*
	* @return void
	* @access private
	*/
	function _paintCurrentCustomisations(&$asset, &$o, $prefix, $weights, $screen)
	{
		$post_field  = $this->_getPostField($screen);
		$current_key = $this->_getCurrentCustomisation($prefix, $screen);

		if (empty($weights)) return;
		$write_access = $asset->writeAccess('attributes');
		$o->openField('');
			$o->openSection('Current customisations');
				$o->openField('');
					?>
						<table class="sq-backend-table" style="width:60%">
							<tr>
								<?php
								// only print this header if we are printing non asset_type screens
								if ($screen != 'asset_type') {
									?><td class="sq-backend-table-header"><?php echo translate('Assetid'); ?></td><?php

								}
								if ($screen === 'asset') {
								    $type_col_name = translate('Asset Name');
								} else {
								    $type_col_name = translate('Asset Type');
								}
								?>
								<td class="sq-backend-table-header"><?php echo $type_col_name; ?></td>
								<?php
									if ($write_access) {
										echo '<td class="sq-backend-table-header">'.translate('Remove').'?</td>';

									}
								?>
							</tr>
							<?php
								foreach ($weights as $key) {
									// highlight the current table row begin customised
									$css_class = 'sq-backend-table-cell';
									if ($key == $current_key) $css_class .= '-alt';

									// when we click on a customisation, set a hidden field to the
									// key that we are customising and submit the screen to save any options
									// that have been changed
									$field = $prefix.'_customise_'.$post_field;
									$o->addHiddenField($field, '');
									$src  = 'Javascript: document.getElementById(\''.$field.'\').value = \''.$key.'\';';
									$src .= 'main_form.submit()';
									?>
									<tr>
										<?php
											if ($screen != 'asset_type') $key_name = $key; // assetid
											$type_name = $this->_getDisplayName($key, $screen);

											// we only paint the section for the assetid if we are
											// printing non-asset_type screens
											if ($screen != 'asset_type') {
												?>
												<td class="<?php echo $css_class; ?>">
													<?php echo $key_name; ?>
												</td>
												<?php
											}
											?>
											<td class="<?php echo $css_class; ?>">
												<a href="<?php echo $src?>"><?php echo $type_name; ?></a>
											</td>
											<?php
												if ($write_access) {
													?>
														<td class="<?php echo $css_class; ?>">
															<?php echo check_box($field.'_remove['.$key.']', 1, FALSE); ?>
														</td>
													<?php
												}
											?>
									</tr>
									<?php
								}//end foreach
							?>
						</table>
					<?php
				$o->closeField();
			$o->closeSection();
		$o->closeField();

	}//end _paintCurrentCustomisations()


	/**
	* Returns the displayable name for the specified key and screen
	*
	* @param string	$key	the key - which is either an assetid or type code
	* @param string	$screen	the current screen being painted
	*
	* @return string
	* @access private
	*/
	function _getDisplayName($key, $screen)
	{
		if (empty($key)) return '';

		if ($screen != 'asset_type') {
			$info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($key), Array(), TRUE);
			$type_name = $info[$key]['name'];
		} else {
			// if the key is 'asset' then we are processing all asset types
			if ($key != 'asset') {
				$type_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($key, 'name');
			} else {
				$type_name = translate('All Asset Types');

			}
		}
		return $type_name;

	}//end _getDisplayName()


	/**
	* Returns the key of the current source being customised
	*
	* @param string	$prefix	the field prefix
	* @param string	$screen	the screen currently being processed
	*
	* @return string
	* @access private
	*/
	function _getCurrentCustomisation($prefix, $screen)
	{
		$post_field = $this->_getPostField($screen);
		$key = array_get_index($_POST, $prefix.'_selected_'.$post_field, '');

		// if the screen requires an asset finder, extract the
		// assetid into $key from the info posted from the asset finder
		if ($post_field == 'assetid') {
			$key = isset($key['assetid']) ? $key['assetid'] : '';
		}
		// if there was nothing selected to customise, check for a current customisation in process
		if (empty($key)) {
			$key = array_get_index($_POST, $prefix.'_current_'.$post_field, '');
		}

		// check to see if a current customisation was selected
		if (isset($_POST[$prefix.'_customise_'.$post_field]) && !empty($_POST[$prefix.'_customise_'.$post_field])) {
			$key = $_POST[$prefix.'_customise_'.$post_field];
		}
		return $key;

	}//end _getCurrentCustomisation()


	/**
	* Returns the field identifier for the specified screen
	*
	* @param string	$screen	the screen of the wanted identifier
	*
	* @return string
	* @access private
	*/
	function _getPostField($screen)
	{
		$post_fields = Array(
						'asset_tree'	=> 'assetid',
						'asset'			=> 'assetid',
						'asset_type'	=> 'type_code',
						'metadata'		=> 'assetid',
					   );
		return $post_fields[$screen];

	}//end _getPostField()


	/**
	* Paints the weighting interface for all screens
	*
	* @param object	&$asset		the search_manager asset
	* @param object	&$o			the backend_outputer
	* @param string	$prefix		the prefix for fields
	* @param string	$screen		the screen that we are painting
	* @param array	$customised	Customisation
	* @param array	$weights	the weighting information to display, if null
	*							the weights will be obtained based on the screen type
	*
	* @return boolean
	* @access private
	*/
	function _paintWeightInterface(&$asset, &$o, $prefix, $screen, $customised=Array(), $weights=NULL)
	{
		$write_access = $asset->writeAccess('attributes');
		$post_field   = $this->_getPostField($screen);
		$key          = $this->_getCurrentCustomisation($prefix, $screen);

		// section opened by xml
			$this->_paintCurrentCustomisations($asset, $o, $prefix, $customised, $screen);
			$this->_paintWeightingSourceInterface($asset, $o, $prefix, $screen);
		$o->closeSection();

		// if any customisations have been removed, dont show the weighting interface
		// as the previous interface shown may have been for a removed customisation
		if ($this->_customisationsRemoved($prefix, $screen)) {
			return $write_access;
		}

		$o->openSection(translate('sch_'.$screen.'_management'));
			$o->openField('');
				if (!empty($key)) {
					// obtain the current weightings for this screen
					$fn = 'get'.str_replace('_', '', $screen).'weightings';
					if (is_null($weights)) $weights = $asset->$fn($key);

					if ($post_field == 'type_code') {
						$type_code = $key;
					} else {
						$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($key), Array(), TRUE);
						$type_code  = $asset_info[$key]['type_code'];
					}

					$o->openField(translate('Enable Indexing ?'));


						if (isset($weights['indexed'])) {
							$indexed = $weights['indexed'];
						} else {
							$indexed = 1;
						}
						$options = Array(
									1	=> translate('Yes'),

									0	=> translate('No'),

								   );
						if ($write_access) {
							combo_box($prefix.'_indexed', $options, FALSE, $indexed);
						} else {
							echo $options[$indexed];
						}
					$o->closeField();

					$asset_weights    = array_get_index($weights, 'asset_weights', Array());
					$attr_weights     = array_get_index($weights, 'attr_weights', Array());
					$metadata_weights = array_get_index($weights, 'metadata_weights', Array());

					$this->_paintAssetWeightInterface($asset, $o, $prefix, $asset_weights);
					$this->_paintAttributeWeightInterface($asset, $o, $prefix, $attr_weights, $type_code);
					$this->_paintMetadataWeightInterface($asset, $o, $prefix, $metadata_weights);

					if (!$this->_customisationsRemoved($prefix, $screen)) {
						$o->addHiddenField($prefix.'_current_'.$post_field, $key);
					}
				}//end if !empty(assetid)
			$o->closeField();
		// section closed by xml

		return $write_access;

	}//end _paintWeightInterface()


	/**
	* Processes the weightings for all weightings screen
	*
	* @param object	&$asset	the search_manager asset
	* @param object	&$o		the backend_outputer
	* @param string	$prefix	the prefix for fields
	* @param string	$screen	the screen to process
	*
	* @return boolean
	* @access private
	*/
	function _processWeights(&$asset, &$o, $prefix, $screen)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		$post_field = $this->_getPostField($screen);

		// if they have clicked on a current customisation, dont process
		if (!empty($_POST[$prefix.'_customise_type_code']) || !empty($_POST[$prefix.'_customise_assetid'])) {
			return FALSE;
		}

		$function_suffix = str_replace('_', '', $screen).'Weightings';
		$get_function    = 'get'.$function_suffix;
		$save_function   = 'save'.$function_suffix;

		$key     = array_get_index($_POST, $prefix.'_current_'.$post_field, '');
		$weights = $asset->$get_function();

		if (!empty($key)) {
			// the asset tree screen is a bit special - we need to dig down the
			// array depending on what we are customising
			if ($screen == 'asset_tree') {

				if (!isset($weights[$key])) $weights[$key] = Array();
				$tree_assetid = $key;
				$tree_weights = $weights;
				$weights      = $weights[$key];

				$tree_type = array_get_index($_POST, $prefix.'_selected_tree_type', '');

				if (empty($tree_type)) return;

				if ($tree_type == 'asset_type') {
					$type_code = array_get_index($_POST, $prefix.'_current_type_code', '');
					if (!empty($type_code)) {
						// the key will be the type code within the asset tree array
						// that we are currently customising
						$key = $type_code;
						if (!isset($weights['type_weights'])) {
							$weights['type_weights'] = Array();
						}
						$weights = $weights['type_weights'];
					} else {
						// remove any asset type customisatons or tree customisations that have
						// be flagged for removal
						$this->_removeUnwantedCustomisations($prefix, 'asset_type', $weights['type_weights']);
						$tree_weights[$key] = $weights;
						$this->_removeUnwantedCustomisations($prefix, $screen, $tree_weights);
						$asset->$save_function($tree_weights);
						return FALSE;
					}
				} else {
					// we are setting metadata weights for assets underneath 'key', which
					// is stored in the global section for the asset tree that we are customising
					if (!isset($weights['metadata_weights'])) {
						$weights['metadata_weights'] = Array();
					}
					$this->_processMetadataWeights($prefix, $weights['metadata_weights']);
					$tree_weights[$tree_assetid] = $weights;
					$asset->$save_function($tree_weights);

					// no more work to do so just return
					return FALSE;
				}
			}//end if asset_tree

			if (!isset($weights[$key])) $weights[$key] = Array();

			foreach (Array('asset_weights', 'attr_weights', 'metadata_weights') as $index) {
				if (!isset($weights[$key][$index])) {
					$weights[$key][$index] = Array();
				}
			}

			// set the 1 if there is no 'indexed' value set
			$indexed = array_get_index($_POST, $prefix.'_indexed', 1);
			$weights[$key]['indexed'] = $indexed;

			$this->_processAssetWeights($prefix, $weights[$key]['asset_weights']);
			$this->_processAttributeWeights($prefix, $weights[$key]['attr_weights']);
			$this->_processMetadataWeights($prefix, $weights[$key]['metadata_weights']);

			if ($screen == 'asset_tree') {
				// remove any asset type customisations that have been flagged for removal
				$this->_removeUnwantedCustomisations($prefix, 'asset_type', $weights);
				// we only get into here if tree_type == asset_type
				$tree_weights[$tree_assetid]['type_weights'] = $weights;
				$weights = $tree_weights;
			}
		}//end if

		if (!empty($weights)) {
			// remove any unwanted customisations
			$this->_removeUnwantedCustomisations($prefix, $screen, $weights);
			$asset->$save_function($weights);
		}

		// we never save attributes
		return FALSE;

	}//end _processWeights()


	/**
	* Processes the weightings for asset component weights
	*
	* @param string	$prefix		the prefix for fields
	* @param array	&$weights	the weights to process
	*
	* @return void
	* @access private
	*/
	function _processAssetWeights($prefix, &$weights)
	{
		$asset_weights = array_get_index($_POST, $prefix.'_asset_weights', Array());

		foreach ($asset_weights as $comp => $info) {
			// verify that all weights are positive
			if ($info['weight'] <= 0) {
				if (!isset($weights[$comp]['weight'])) {
					$asset_weights[$comp]['weight'] = 1;
				} else {
					$asset_weights[$comp]['weight'] = (int) $weights[$comp]['weight'];
				}
			}
			if (!isset($info['indexed'])) {
				$asset_weights[$comp]['indexed'] = 0;
			}
		}
		$weights = $asset_weights;

	}//end _processAssetWeights()


	/**
	* Processes the weightings for attribute weights
	*
	* @param string	$prefix		the prefix for fields
	* @param array	&$weights	the weights to process
	*
	* @return void
	* @access private
	*/
	function _processAttributeWeights($prefix, &$weights)
	{
		$attr_weights = array_get_index($_POST, $prefix.'_attr_weights', Array());
		foreach ($attr_weights as $name => $info) {
			// verify that all weights are positive
			if ($info['weight'] <= 0) {
				if (!isset($weights[$name]['weight'])) {
					$attr_weights[$name]['weight'] = 1;
				} else {
					$attr_weights[$name]['weight'] = (int) $weights[$name]['weight'];
				}
			}
			if (!isset($info['indexed'])) {
				$attr_weights[$name]['indexed'] = 0;
			}
		}
		$weights = $attr_weights;

	}//end _processAttributeWeights()


	/**
	* Processes the weightings for metadata weights
	*
	* @param string	$prefix		the prefix for fields
	* @param array	&$weights	the weights to process
	*
	* @return void
	* @access private
	*/
	function _processMetadataWeights($prefix, &$weights)
	{
		// information about already added metadata assets
		$metadata_assets = array_get_index($_POST, $prefix.'_metadata_assets', Array());

		// process any assets that have been previously added
		foreach ($metadata_assets as $assetid => $info) {
			if (isset($info['remove']) && $info['remove']) {
				unset($weights[$assetid]);
			} else {
				$weight = ((int) $info['weight'] > 0) ? $info['weight'] : NULL;
				if (!is_null($weight)) {
					$weights[$assetid]['weight'] = (int) $weight;
				}
				$weights[$assetid]['indexed'] = array_get_index($info, 'indexed', FALSE);
			}
		}

		// the information posted by the asset finder
		$metadata_asset = array_get_index($_POST, $prefix.'_metadata_asset', NULL);

		if (!is_null($metadata_asset) && $metadata_asset['assetid'] != 0) {
			$weight = array_get_index($_POST, $prefix.'_metadata_weight', 1);
			if ($weight < 0) $weight = 1;

			$assetid   = $metadata_asset['assetid'];
			$type_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid), Array(), TRUE, 'type_code');
			$type_code = $type_info[$assetid];

			// save any values from a metadata asset that was selected using the asset finder
			$weights[$assetid] = Array(
									'weight'	=> (int) $weight,
									'type_code'	=> $type_code,
									'indexed'	=> 1,
								 );
		}

	}//end _processMetadataWeights()


	/**
	* Removes any customisations that have been flagged for removal
	*
	* @param string	$prefix		the field prefix
	* @param string	$screen		the screen being processed
	* @param array	&$weights	the weights being processed
	*
	* @return void
	* @access private
	*/
	function _removeUnwantedCustomisations($prefix, $screen, &$weights)
	{
		$post_field = $this->_getPostField($screen);
		// remove any customisations that have been checked for removal
		$remove_array = array_get_index($_POST, $prefix.'_customise_'.$post_field.'_remove', Array());
		foreach ($remove_array as $key => $info) {
			unset($weights[$key]);
		}

	}//end _removeUnwantedCustomisations()


	/**
	* Returns true if customisations were removed in the last save action
	*
	* @param string	$prefix	the field prefix
	* @param string	$screen	the screen being processed
	*
	* @return boolean
	* @access private
	*/
	function _customisationsRemoved($prefix, $screen)
	{
		$post_field = $this->_getPostField($screen);
		$removed = array_get_index($_POST, $prefix.'_customise_'.$post_field.'_remove', Array());

		return (!empty($removed));

	}//end _customisationsRemoved()


	/**
	* Paint the interface for queuing a HIPO to reindex the system
	*
	* @param object	&$asset	the search manager asset
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	the prefix for field
	*
	* @return boolean
	* @access public
	*/
	function paintReindexing(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			asset_finder($prefix.'_reindexing_root_assetid', '');
			echo '&nbsp;&nbsp;&nbsp;<b>'.strtoupper(translate('Or')).'</b>';

			$o->closeField();
			$o->openField(translate('System Wide'));

			check_box($prefix.'_reindexing_root_folder');
			echo translate('Reindex all assets in the system');

			return TRUE;
		}
		return FALSE;

	}//end paintReindexing()


	/**
	* Process the interface for queuing a HIPO to reindex the system
	*
	* @param object	&$asset	the search manager asset
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	the prefix for field
	*
	* @return boolean
	* @access public
	*/
	function processReindexing(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_reindexing_root_assetid']['assetid'])) {
			$root_assetid = $_POST[$prefix.'_reindexing_root_assetid']['assetid'];
			if ($root_assetid > 0) {
				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$running_vars = Array(
									'root_assetid'	=> $root_assetid,
									'contextid'		=> $GLOBALS['SQ_SYSTEM']->getContextId(),
								);
				$hh->queueHipo('hipo_job_reindex', $running_vars, '', SQ_PACKAGES_PATH.'/search/hipo_jobs');
				return TRUE;
			}
		}

		if (isset($_POST[$prefix.'_reindexing_root_folder'])) {
			// System-wide reindex.
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$running_vars = Array(
					'root_assetid'	=> $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('root_folder'),
					'contextid'		=> $GLOBALS['SQ_SYSTEM']->getContextId(),
				);
			$hh->queueHipo('hipo_job_reindex', $running_vars, '', SQ_PACKAGES_PATH.'/search/hipo_jobs');
			return TRUE;
		}
		return FALSE;

	}//end processReindexing()


	/**
	* Paint the interface for Noise Words
	*
	* @param object	&$asset	the search manager asset
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	the prefix for field
	*
	* @return boolean
	* @access public
	*/
	function paintNoiseWords(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$noise_words = $asset->attr('noise_word_list');
		$noise_words = (is_array($noise_words)) ? implode(' ', $noise_words) : '';

		if ($write_access) {
			text_area($prefix.'_noise_words', $noise_words, 60, 5);
		} else {
			echo $noise_words;
		}
		return $write_access;

	}//end paintNoiseWords()


	/**
	* Process the interface for adding noise words
	*
	* @param object	&$asset	the search manager asset
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	the prefix for field
	*
	* @return boolean
	* @access public
	*/
	function processNoiseWords(&$asset, &$o, $prefix)
	{

		$words = array_get_index($_POST, $prefix.'_noise_words', FALSE);

		$matches	= Array();
		$new_words	= Array();

		if (preg_match_all('/\w+/i', $words, $matches, PREG_SET_ORDER)) {
			foreach ($matches as $match) {
				$word = array_pop($match);
				$new_words[$word] = $word;
			}
		}

		$asset->setAttrValue('noise_word_list', $new_words);

		return TRUE;

	}//end processNoiseWords()


	/**
	* Paint the interface for White Words
	*
	* @param object	&$asset	the search manager asset
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	the prefix for field
	*
	* @return boolean
	* @access public
	*/
	function paintWhiteWords(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$words = $asset->attr('white_word_list');
		$words = (is_array($words)) ? implode(' ', $words) : '';

		if ($write_access) {
			text_area($prefix.'_white_words', $words, 60, 5);
		} else {
			echo $words;
		}
		return $write_access;

	}//end paintWhiteWords()


	/**
	* Process the interface for adding white words
	*
	* @param object	&$asset	the search manager asset
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	the prefix for field
	*
	* @return boolean
	* @access public
	*/
	function processWhiteWords(&$asset, &$o, $prefix)
	{
		$words = array_get_index($_POST, $prefix.'_white_words', FALSE);

		$matches	= Array();
		$new_words	= Array();

		if (preg_match_all('/\w+/i', $words, $matches, PREG_SET_ORDER)) {
			foreach ($matches as $match) {
				$word = array_pop($match);
				$new_words[$word] = $word;
			}
		}

		$asset->setAttrValue('white_word_list', $new_words);

		return TRUE;

	}//end processWhiteWords()


	/**
	* Returns true if search indexing is on.
	*
	* @param object	&$owner	the search manager
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access public
	*/
	function indexingOn(&$owner, $prefix)
	{
		return $owner->attr('indexing');

	}//end indexingOn()


	/**
	 * Determine whether we are using the interface "Go" buttons.
	 *
	 * @param string $prefix
	 */
	private function _usingGoButton($prefix)
	{
		$tree_type_go = array_get_index($_POST, $prefix.'_selected_tree_type_go', FALSE);
		$type_code_go = array_get_index($_POST, $prefix.'_selected_type_code_go', FALSE);

		return ($tree_type_go !== FALSE || $type_code_go !== FALSE);

	}// end _usingGoButton()


}//end class
?>
