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
* $Id: page_asset_listing_edit_fns.inc,v 1.15 2005/02/03 15:57:25 brobertson Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Page_Asset_Listing_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.15 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Asset_Listing_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Page_Asset_Listing_Edit_Fns()
	{
		$this->Page_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = false;
		$this->static_screens['details']['lock_type'] = 'content';

	}//end constructor


	/**
	* Paint links to the defaut formats
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDefaultFormatLinks(&$asset, &$o, $prefix)
	{
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header">Edit Format</td>
				<td class="sq-backend-table-header">Note</td>
			</tr>
			<?php
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'page_contents');
			if (!empty($link)) {
				$info = Array($link['minorid'] => 'contents');
				$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($info);
				?>
				<tr>
					<td class="sq-backend-table-cell">
						<a href="<?php echo $href[$link['minorid']]; ?>">Page Contents Bodycopy</a>
					</td>
					<td class="sq-backend-table-cell">
						This format defines the layout of the page. Ensure you use the %asset_listing% keyword to include the list of assets on the page.
					</td>
				</tr>
				<?php
			}
			
			$folder = &$asset->getFolder('type_formats');
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'default_format');
			if (!empty($link)) {
				$info = Array($link['minorid'] => 'contents');
				$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($info);
				?>
				<tr>
					<td class="sq-backend-table-cell">
						<a href="<?php echo $href[$link['minorid']]; ?>">Default Asset Format</a>
					</td>
					<td class="sq-backend-table-cell">
						This is the default format for all assets in the list if they do not have a customised format for their asset type or position.
					</td>
				</tr>
				<?php
			}
			?>
		</table>
		<?php

		return false;

	}//end paintDefaultFormatLinks()


	/**
	* Does nothing
	*
	* @return boolean
	* @access public
	*/
	function processDefaultFormatLinks()
	{
		return false;

	}//end processDefaultFormatLinks()


	/**
	* Paint the interface for editing custom type formats
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintTypeFormats(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('links');
		$types = $asset->attr('types');
		$formats = $asset->getFormats('type_formats');

		if (count($formats) > 1) {
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header">Asset Type</td>
					<td class="sq-backend-table-header">Edit Format</td>
					<?php
					if ($write_access) {
						?>
						<td class="sq-backend-table-header">Delete?</td>
						<?php
					}
					?>
				</tr>
				<?php
				foreach ($formats as $minorid => $value) {
					if ($value == 'default_format') continue;
	
					$info = Array($minorid => 'contents');
					$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($info)
					?>
					<tr>
						<td class="sq-backend-table-cell" style="width: 200px;">
							<?php
							echo ucwords(str_replace('_',' ', $value));
							?>
						</td>
						<td class="sq-backend-table-cell">
							<?php
							echo '<a href="'.$href[$minorid].'">'.ucwords(str_replace('_',' ', $value)).' Format</a>';
							?>
						</td>
						<?php
						if ($write_access) {
							?>
							<td class="sq-backend-table-cell" style="width: 100px;">
								<?php
								check_box($prefix.'_delete_type['.$value.']');
								?>
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
		} else {
			echo 'There are no custom type formats defined';
		}//end if cutomised formats
		
		if ($write_access) {
			$new_format_options = Array('' => '');
			foreach ($types as $type => $inherit) {
				if (in_array($type, $formats)) continue;
				$new_format_options[$type] = ucwords(str_replace('_',' ', $type));
			}
			if (count($new_format_options) > 1) {
				echo '<p><b>New Type Format: <b>';
				combo_box($prefix.'_new_type', $new_format_options, false, '');
				echo '</p>';
			}
		}
		return false;

	}//end paintTypeFormats()


	/**
	* Process the interface for editing custom type formats
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processTypeFormats(&$asset, &$o, $prefix)
	{
		$folder = &$asset->getFolder('type_formats');
		$types = $asset->attr('types');
		
		$deleted_formats = array_get_index($_POST, $prefix.'_delete_type', Array());
		$new_type = array_get_index($_POST, $prefix.'_new_type', '');
		if (isset($deleted_formats[$new_type])) {
			trigger_error('You cant delete a format and create it at the same time. The current type format for type'.$new_type.' has not been deleted.', E_USER_WARNING);
			unset($deleted_formats[$new_type]);
			$new_type = '';
		}

		foreach ($deleted_formats as $type => $on) {
			$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true, $type);
			if (empty($current_link)) {
				trigger_error('Failed deleting format for type '.$type.', couldnt get format bodycopy link', E_USER_WARNING);
			} else {
				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($current_link['linkid'])) {
					trigger_error('Failed deleting format for type '.$type, E_USER_WARNING);
				}
			}
		}

		// check that the new type is valid
		if ($new_type != '' && !isset($types[$new_type])) {
			trigger_error('Cannot customtise the format of type '.$new_type.', type is not valid', E_USER_WARNING);
			$new_type = '';
		}

		if ($new_type != '') {
			// check that this type isnt already customised
			$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true, $new_type);
			if (empty($current_link)) {
				// create a new format bodycopy for this type
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
	
				$copy_link = Array('asset' => &$folder, 'value' => $new_type, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
	
				$bodycopy = new Bodycopy();
				$bodycopy->setAttrValue('name', ucwords(str_replace('_',' ', $new_type)).' Format');
	
				if (!$bodycopy->create($copy_link)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			} else {
				trigger_error('Cannot customise type '.$new_type.', it is already customised', E_USER_WARNING);
			}
		}
		if (isset($asset->_tmp['formats']['type_formats'])) unset($asset->_tmp['formats']['type_formats']);
		return false;

	}//end processTypeFormats()


	/**
	* Paint the interface for editing custom position formats
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPositionFormats(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('links');
		$formats = $asset->getFormats('position_formats');

		if (!empty($formats)) {
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header">Position</td>
					<td class="sq-backend-table-header">Edit Format</td>
					<?php
					if ($write_access) {
						?><td class="sq-backend-table-header">Delete?</td><?php
					}
					?>
				</tr>
				<?php
				foreach ($formats as $minorid => $value) {
					$info = Array($minorid => 'contents');
					$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($info);
					$position = substr($value, 9);
					?>
					<tr>
						<td class="sq-backend-table-cell" style="width: 200px;">
							<?php
							echo $position;
							?>
						</td>
						<td class="sq-backend-table-cell">
							<?php
							echo '<a href="'.$href[$minorid].'">Position '.$position.' Format</a>';
							?>
						</td>
						<?php
						if ($write_access) {
							?>
							<td class="sq-backend-table-cell" style="width: 100px;">
								<?php
								check_box($prefix.'_delete_position['.$position.']');
								?>
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
		} else {
			echo 'There are no custom position formats defined';
		}
		
		if ($write_access) {
			echo '<p><b>New Position Format: </b>';
			text_box($prefix.'_new_position', '', 5);
			echo '</p>';
		}

		return $write_access;

	}//end paintPositionFormats()


	/**
	* Process the interface for editing custom position formats
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processPositionFormats(&$asset, &$o, $prefix)
	{
		$folder = &$asset->getFolder('position_formats');
		
		$deleted_formats = array_get_index($_POST, $prefix.'_delete_position', Array());
		$new_position = (int) array_get_index($_POST, $prefix.'_new_position', 0);
		if (isset($deleted_formats[$new_position])) {
			trigger_error('You cant delete a format and create it at the same time. The current list position format for position '.$new_position.' has not been deleted.', E_USER_WARNING);
			unset($deleted_formats[$new_position]);
			$new_position = 0;
		}

		foreach ($deleted_formats as $position => $on) {
			$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'position_'.$position);
			if (empty($current_link)) {
				trigger_error('Failed deleting format for position '.$position.', couldnt get format bodycopy link', E_USER_WARNING);
			} else {
				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($current_link['linkid'])) {
					trigger_error('Failed deleting format for position '.$position, E_USER_WARNING);
				}
			}
		}

		if ($new_position > 0) {
			// check that this position isnt already customised
			$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'position_'.$new_position);
			if (empty($current_link)) {

				// create a new format bodycopy for this list position
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
	
				$copy_link = Array('asset' => &$folder, 'value' => 'position_'.$new_position, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
	
				$bodycopy = new Bodycopy();
				$bodycopy->setAttrValue('name', 'Position '.$new_position.' Format');
	
				if (!$bodycopy->create($copy_link)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			} else {
				trigger_error('Cannot customise list position '.$new_position.', it is already customised', E_USER_WARNING);
			}
		}
		if (isset($asset->_tmp['formats']['position_formats'])) unset($asset->_tmp['formats']['position_formats']);
		return false;

	}//end processPositionFormats()


	/**
	* Paints the root node selection box
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintRootNode(&$asset, &$o, $prefix)
	{
		// get root links
		$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', false, 'major', 'root');
		$root_asset_ids = Array();
		foreach ($root_links as $root_link) {
			$root_asset_ids[$root_link['minorid']] = $root_link['minor_type_code'];
		}
		$root_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($root_asset_ids));
		
		$write_access = $asset->writeAccess('links');

		?>
		<table class="sq-backend-table">
			<?php
			$l = 1;
			foreach ($root_links as $root_link) {
				?>
				<tr>
					<td class="sq-backend-table-cell">
						<?php
						if ($write_access) {
							asset_finder($prefix.'_root_nodes['.$l.']', $root_link['minorid']);
						} else {
							echo $root_info[$root_link['minorid']]['name'].' [# '.$root_link['minorid'].']';
						}
						?>
					</td>
				</tr>
				<?php
				$l++;
			}//end foreach
		
			if ($write_access) {
				?>
				<tr>
					<td class="sq-backend-table-cell"><?php asset_finder($prefix.'_root_nodes['.$l.']', 0); ?></td>
				</tr>
				<?php
			}
			?>
		</table>
		<?php

		return $write_access;

	}//end paintRootNode()


	/**
	* Processes the value input from root node selection box
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processRootNode(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_root_nodes'])) return false;
		$new_root_nodes = $_POST[$prefix.'_root_nodes'];				
		foreach ($new_root_nodes as $new_root_node) {
			$new_root_node_ids[] = $new_root_node['assetid'];
		}
		
		// get existing root links
		$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', false, 'major', 'root');

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$ok = true;

		// lets delete the root nodes that have been removed
		foreach ($root_links as $root_link) {
			if (!in_array($root_link['minorid'], $new_root_node_ids)) {
				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($root_link['linkid'])) {
					$ok = false;
				}
			}
		}
		
		// now, create the new node links
		foreach ($new_root_nodes as $index => $node) {
			//first test to see if the node already exists -- if so, ignore it
			foreach($root_links as $root_link) {
				if ($root_link['minorid'] == $node['assetid']) continue 2;
			}
			// otherwise, create the appropriate notice link
			if ($node['assetid'] != 0) {
				$new_root = &$GLOBALS['SQ_SYSTEM']->am->getAsset($node['assetid']);
				if (!$asset->createLink($new_root, SQ_LINK_NOTICE, 'root')) {
					$ok = false;
					trigger_error('Failed adding new root node "'.$new_root->name.'" [#'.$new_root->id.']', E_USER_WARNING);
				}
			}
		}
		
		if ($ok) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

	}//end processRootNode()


	/**
	* Paint the current asset types that have been added
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCurrentAssetTypes(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('content');
		$folder = &$asset->getFolder('type_formats');

		$types = $asset->attr('types');
		if (empty($types)) {
			echo 'There are no types defined';
			return false;
		}

		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header">Asset Type</td>
				<td class="sq-backend-table-header">Inherit Types</td>
				<?php
				if ($write_access) {
					?>
					<td class="sq-backend-table-header">Remove?</td>
					<?php
				}
				?>
			</tr>
			<?php
			foreach ($types as $type => $inherit) {
				?>
				<tr>
					<td class="sq-backend-table-cell">
						<?php
						echo ucwords(str_replace('_',' ', $type));
						?>
					</td>
					<td class="sq-backend-table-cell">
						<?php
						if ($write_access) {
							check_box($prefix.'_inherit_types[]', $type, $inherit);
						} else {
							echo ($inherit) ? 'Yes' : 'No';
						}
						?>
					</td>
					<?php
					if ($write_access) {
						?>
						<td class="sq-backend-table-cell">
							<?php
							check_box($prefix.'_remove[]', $type, false);
							?>
						</td>
						<?php
					}
					?>
				</tr>
				<?php
			}//end foreach $types
			?>
		</table>
		<?php

	}//end paintCurrentAssetTypes()


	/**
	* Process the current asset types that have been added
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processCurrentAssetTypes(&$asset, &$o, $prefix)
	{
		$types = $asset->attr('types');
		$folder = &$asset->getFolder('type_formats');
		$formats = $asset->getFormats('type_formats');

		// we are looking to see if any types have been removed
		$removed = array_get_index($_POST, $prefix.'_remove', Array());
		foreach ($removed as $type) {
			if (isset($types[$type])) {
				unset($types[$type]);

				if (in_array($type, $formats)) {
					// we also want to send the bodycopy to the trash
					$link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true, $type);
					if (empty($link)) {
						trigger_error('Format for type '.$type.' not found', E_USER_WARNING);
					}
					// back up in your trash with the resurection
					$folder->deleteLink($link['linkid']);
				}
			}
		}


		if (!isset($_POST[$prefix.'_inherit_types'])) $_POST[$prefix.'_inherit_types'] = Array();
		for (reset($types); null !== ($k = key($types)); next($types)) {
			$types[$k] = (in_array($k, $_POST[$prefix.'_inherit_types'])) ? true : false;
		}
		$asset->setAttrValue('types', $types);

		return true;

	}//end processCurrentAssetTypes()


	/**
	* Paint section to add an asset type
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddAssetType(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('content')) return false;

		$types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();
		$types = array_diff($types, $asset->attr('types'));
		asort($types);

		$uc_types = Array('' => '');
		foreach ($types as $type) $uc_types[$type] = ucwords(str_replace('_',' ', $type));
		combo_box($prefix.'_new_type', $uc_types, false, '');

	}//end paintAddAssetType()
	
	
	/**
	* Paint the interface for editing custom asset positions
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAssetPositions(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$asset_positions = $asset->attr('asset_positions');

		if (!empty($asset_positions)) {
			ksort($asset_positions, SORT_NUMERIC);
			if (!$write_access) {
				$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_values($asset_positions));
			}
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header">Position</td>
					<td class="sq-backend-table-header">Asset</td>
					<?php
					if ($write_access) {
						?><td class="sq-backend-table-header">Delete?</td><?php
					}
					?>
				</tr>
				<?php
				foreach ($asset_positions as $position => $assetid) {
					?>
					<tr>
						<td class="sq-backend-table-cell"><?php echo $position; ?></td>
						<td class="sq-backend-table-cell">
							<?php
							if ($write_access) {
								asset_finder($prefix.'_asset_position['.$position.']', $assetid, Array('asset' => 'D'));
							} else {
								$info = array_get_index($asset_info, $assetid, Array());
								if (empty($info)) {
									echo 'Unknown asset';
								} else {
									echo $info['short_name'].' (#'.$assetid.')';
								}
							}
							?>
						</td>
						<?php
						if ($write_access) {
							?>
							<td class="sq-backend-table-cell">
								<?php
								check_box($prefix.'_delete_asset_position['.$position.']');
								?>
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
		} else {
			echo 'There are no asset positions defined. All assets will be listed using the sorting settings.';
		}
		
		if ($write_access) {
			echo '<p><b>New Asset Position: </b>';
			text_box($prefix.'_new_asset_position_pos', '', 5);
			echo ' ';
			asset_finder($prefix.'_new_asset_position_asset', '', Array('asset' => 'D'));
			echo '</p>';
		}

		return $write_access;

	}//end paintPositionFormats()


	/**
	* Process the interface for editing custom asset positions
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAssetPositions(&$asset, &$o, $prefix)
	{
		$asset_positions = Array();

		$new_position_pos = (int) array_get_index($_POST, $prefix.'_new_asset_position_pos', 0);
		if ($new_position_pos > 0) {
			$new_position_data = array_get_index($_POST, $prefix.'_new_asset_position_asset', Array());
			if ($new_position_assetid = $new_position_data['assetid']) {
				$asset_positions[$new_position_pos] = $new_position_assetid;
			}
		}
		
		$current_asset_positions = array_get_index($_POST, $prefix.'_asset_position', Array());
		foreach ($current_asset_positions as $position => $data) {
			if ($data['assetid']) {
				$asset_positions[$position] = $data['assetid'];
			}
		}
		
		$delete_asset_position = array_get_index($_POST, $prefix.'_delete_asset_position', Array());
		foreach ($delete_asset_position as $position => $on) {
			unset($asset_positions[$position]);
		}

		return $asset->setAttrValue('asset_positions', $asset_positions);

	}//end processAssetPositions()


	/**
	* Process the section to add a new format for an asset
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddAssetType(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_new_type'])) {
			$type = trim($_POST[$prefix.'_new_type']);
			if (empty($type)) return false;
			$types = $asset->attr('types');
			if (!isset($types[$type])) {
				$types[$type] = true;
				$asset->setAttrValue('types', $types);
				return true;
			}
		}
		return false;

	}//end processAddAssetType()


	/**
	* Paints the interface for setting the number of columns to display per row
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintColumnSettings(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$current = $asset->attr('column_layout');
		
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header">Row</td>
				<td class="sq-backend-table-header">Column(s)</td>
				<?php
				if ($write_access) {
					?>
					<td class="sq-backend-table-header">Delete ?</td>
					<?php
				}
				?>
			</tr>
			<?php
			if (count($current) > 1) {
				foreach ($current as $row => $columns) {
					if ($row == 0) continue;
					?>
					<tr>
						<td class="sq-backend-table-cell"><?php echo $row; ?></td>
						<td class="sq-backend-table-cell">
							<?php
							if ($write_access) {
								text_box($prefix.'_columns['.$row.']', $columns, 5);
							} else {
								echo $columns;
							}
							?>
						</td>
						<?php
						if ($write_access) {
							?>
							<td class="sq-backend-table-cell">
								<?php check_box($prefix.'_delete_row['.$row.']'); ?>
							</td>
							<?php
						}
					?>
					</tr>
					<?php
				}//end foreach
			}//end if

			?>
			<tr>
				<td class="sq-backend-table-cell">All remaining</td>
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) {
						text_box($prefix.'_columns[0]', $current['0'], 5);
					} else {
						echo $current['0'];
					}
					?>
				</td>
				<?php
				if ($write_access) {
					?><td class="sq-backend-table-cell">&nbsp;</td><?php
				}
				?>
			</tr>
		</table>
		<?php

		if ($write_access) {
			$o->closeField();
			$o->openField('Customise Row');
				echo 'row ';
				text_box($prefix.'_new_row', '', 5);
				echo ' columns ';
				text_box($prefix.'_new_column', '', 5);
			// field closed by asset_edit_interface
		}

		return $write_access;

	}//end paintColumnSettings()


	/**
	* Processes the interface for setting the number of columns to display per row
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processColumnSettings(&$asset, &$o, $prefix)
	{
		// change existing column settings
		$column_layout = array_get_index($_POST, $prefix.'_columns', Array());
		$deleted_rows  = array_get_index($_POST, $prefix.'_delete_row', Array());
		foreach ($deleted_rows as $row => $on) {
			if (isset($column_layout[$row])) unset($column_layout[$row]);
		}
		
		// add a new column setting
		$new_row = (int)array_get_index($_POST, $prefix.'_new_row', 0);
		$new_column = (int)array_get_index($_POST, $prefix.'_new_column', 0);
		if ($new_row > 0) {
			if ($new_column > 0) {
				if (!isset($column_layout[$new_row])) {
					$column_layout[$new_row] = $new_column;
				} else {
					trigger_error('Failed adding customisation for row '.$new_row.', row has already been customised', E_USER_WARNING);
				}
			} else {
				trigger_error('Failed adding customisation for row '.$new_row.', number of columns must be greater than zero', E_USER_WARNING);
			}
		}
		
		return $asset->setAttrValue('column_layout', $column_layout);

	}//end processColumnSettings()


	/**
	* Paints the interface for previewing the column layout
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintColumnPreview(&$asset, &$o, $prefix)
	{
		$column_layout = $asset->attr('column_layout');
		$num_per_page  = $asset->attr('num_per_page');

		$num_rows = max(array_keys($column_layout)) + 1;
		$num_columns = max(array_values($column_layout));
		
		?>
		<table class="sq-backend-table">
			<?php
			$cell_num = 1;
			for ($i = 1; $i <= $num_rows; $i++) {
				echo '<tr>';
				$cols_in_row = array_get_index($column_layout, $i, $column_layout['0']);
				$each_colspan = (int)($num_columns / $cols_in_row);
				$first_colspan = $each_colspan + $num_columns - ($cols_in_row * $each_colspan);

				for ($c = 1; $c <= $cols_in_row; $c++) {
					$colspan = ($c == 1) ? $first_colspan : $each_colspan;
					$class = 'sq-backend-table-cell';
					if ($num_per_page > 0 && $cell_num > $num_per_page) $class .= '-alt';
					?>
					<td class="<?php echo $class; ?>" style="<?php echo ($c < $cols_in_row) ? ' border-right: 1px solid #CCCCCC;' : ''; ?>"<?php echo ($colspan > 1) ? ' colspan="'.$colspan.'"' : ''; ?>>
						<b><?php echo $cell_num; ?></b>
					</td>
					<?php
					$cell_num++;
				}
				echo '</tr>';
			}
			?>
			<tr>
				<td class="sq-backend-table-cell" style="text-align: center;" colspan="<?php echo $num_columns; ?>">
					All remaining rows will have <?php echo $column_layout[0]; ?> column(s)
					<?php
					if ($num_per_page > 0 && $cell_num > $num_per_page) {
						echo '<br /><b>Note: This asset has been configured to list '.$num_per_page.' assets per page, so the highlighted cells will not be displayed</b>';
					}
					?>
				</td>
			</tr>
		</table>
		<?php

	}//end paintColumnPreview()


	/**
	* Processes the interface for previewing the column layout
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processColumnPreview(&$asset, &$o, $prefix)
	{
		return false;

	}//end processColumnPreview()


}//end class

?>
