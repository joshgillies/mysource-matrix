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
* $Id: search_folder_edit_fns.inc,v 1.11 2005/03/24 03:04:29 dheppell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Search_Folder_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Folder_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Search_Folder_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

		$this->static_screens = Array(
									  'details'     => Array(
															 'name'         => 'Details',
															 'force_unlock' => false,
															 'lock_type'    => 'attributes',
															 ),
									  'permissions' => Array(
															 'name'         => 'Permissions',
															 'force_unlock' => true,
															 'lock_type'    => 'permissions',
															 ),
									  'linking'     => Array(
															 'name'         => 'Linking',
															 'force_unlock' => true,
															 'lock_type'    => 'links',
															 ),
									  'history'     => Array(
															 'name'         => 'History',
															 'force_unlock' => true,
															 'lock_type'    => 'none',
															 ),
									  );

	}//end constructor


	/**
	* Paints the root node selection box
	*
	* @param object Search_Folder		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRoot(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$settings = $asset->attr('settings');
		$current = (isset($settings['root'])) ? $settings['root'] : '';

		if ($write_access) {
			asset_finder($prefix.'_rootid', $current);
		} else {
			$root_asset = null;
			if (!empty($current)) $root_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current);
			if (!is_null($root_asset)) echo $root_asset->name . ' (Id: #' . $root_asset->id . ')';
			else echo 'No search root has been selected. The whole system will be searched.';
		}

		return $write_access;

	}//end paintRoot()


	/**
	* Processes the value input from root node selection box
	*
	* @param object Search_Folder		&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	function processRoot(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return false;

		$settings = $asset->attr('settings');

		if (isset($_POST[$prefix.'_rootid']['assetid'])) {
			$settings['root'] = $_POST[$prefix.'_rootid']['assetid'];
		} else {
			$settings['root'] = '';
		}

		return $asset->setAttrValue('settings', $settings);

	}//end processRoot()


	/**
	* Paint the interface for entering a search phrase
	*
	* @param object Search_Folder		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPhrase(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$settings = $asset->attr('settings');
		$current = (isset($settings['phrase'])) ? $settings['phrase'] : '';

		if ($write_access) {
			text_box($prefix.'_phrase', $current, 50);
		} else {
			if (empty($current)) {
				echo 'No search terms have been entered. All assets will be searched.';
			} else {
				echo $current;
			}
		}

		return $write_access;

	}//end paintPhrase()


	/**
	* Processes the interface for entering a search phrase
	*
	* @param object Search_Folder		&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processPhrase(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return false;

		$settings = $asset->attr('settings');

		if (isset($_POST[$prefix.'_phrase'])) {
			$settings['phrase'] = trim($_POST[$prefix.'_phrase']);
		} else {
			$settings['phrase'] = '';
		}

		return $asset->setAttrValue('settings', $settings);

	}//end processPhrase()


	/**
	* Paint the interface for selecting search logic
	*
	* @param object Search_Folder		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintLogic(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$settings = $asset->attr('settings');
		$current = (isset($settings['logic'])) ? $settings['logic'] : 'all';

		if ($write_access) {
			combo_box($prefix.'_logic', Array('and' => 'Results include all terms', 'or' => 'Results include any terms'), false, $current);
		} else {
			if ($current == 'or') echo 'Results include any terms';
			else echo 'Results include all terms';
		}

		return $write_access;

	}//end paintLogic()


	/**
	* Processes the interface for selecting search logic
	*
	* @param object Search_Folder		&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processLogic(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return false;

		$settings = $asset->attr('settings');

		if (isset($_POST[$prefix.'_logic'])) {
			$settings['logic'] = trim($_POST[$prefix.'_logic']);
		} else {
			$settings['logic'] = '';
		}

		return $asset->setAttrValue('settings', $settings);

	}//end processLogic()


	/**
	* Paint the interface for selecting searched statuses
	*
	* @param object Search_Folder		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintStatus(&$asset, &$o, $prefix)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$write_access = $asset->writeAccess('attributes');

		$settings = $asset->attr('settings');
		$current = (isset($settings['statuses'])) ? $settings['statuses'] : Array();

		if ($write_access) {
			require_once SQ_LIB_PATH.'/html_form/html_form.inc';
			$statuses = get_constant_values('SQ_STATUS');

			$options = Array();
			foreach ($statuses as $constant => $value) $options[$value] = get_status_description($value);

			combo_box($prefix.'_statuses', $options, true, $current);
		} else {
			if (empty($current)) {
				echo 'No statuses are selected';
			} else {
				echo '<ul>';
				foreach ($current as $status) echo '<li>'.get_status_description($status).'</li>';
				echo '</ul>';
			}
		}

		return $write_access;

	}//end paintStatus()


	/**
	* Processes the interface for selecting searched statuses
	*
	* @param object Search_Folder		&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processStatus(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return false;

		$settings = $asset->attr('settings');

		if (isset($_POST[$prefix.'_statuses'])) {
			$settings['statuses'] = Array();
			foreach ($_POST[$prefix.'_statuses'] as $status) {
				$settings['statuses'][] = $status;
			}
			$settings['statuses'] = array_unique($settings['statuses']);
		} else {
			$settings['statuses'] = Array();
		}

		return $asset->setAttrValue('settings', $settings);

	}//end processStatus()


	/**
	* Paint the interface for selecting searched asset types
	*
	* @param object Search_Folder		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintTypes(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$settings = $asset->attr('settings');
		$current = (isset($settings['types'])) ? $settings['types'] : Array();

		if (!empty($current)) {

				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header">Searched Asset Types</td>
						<?php
						if ($write_access) {
							?><td class="sq-backend-table-header">Delete ?</td><?php
						}
						?>
					</tr>
				<?php

				foreach ($current as $type) {
					?>
					<tr>
						<td class="sq-backend-table-cell"><?php echo $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type, 'name'); ?></td>
						<?php
						if ($write_access) {
							?><td class="sq-backend-table-cell"><?php check_box($prefix.'_delete_type[]', $type); ?></td><?php
						}
						?>
					</tr>
					<?php
				}

				?></table><?php

		} else {
			echo 'No asset types are selected';
		}


		if ($write_access) {
			$asset_types = array_values(array_diff($GLOBALS['SQ_SYSTEM']->am->getTypeList(), $current));
			$asset_type_list = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset_types, 'name');
			if (!empty($asset_type_list)){
				asort($asset_type_list);
				echo '<br /><span class="sq-backend-field">Add New Type: </span>';
				combo_box($prefix.'_new_type', Array('' => '- Select asset type -') + $asset_type_list, false, 'none');
			}
		}

		return $write_access;

	}//end paintTypes()


	/**
	* Processes the interface for selecting searched asset types
	*
	* @param object Search_Folder		&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processTypes(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return false;

		$settings = $asset->attr('settings');
		$search_types = (isset($settings['types'])) ? $settings['types'] : Array();

		// deleting types
		if (isset($_POST[$prefix.'_delete_type'])) {
			$search_types = array_values(array_diff($search_types, $_POST[$prefix.'_delete_type']));
			$update = true;
		}

		// add type
		if (!empty($_POST[$prefix.'_new_type']) && $_POST[$prefix.'_new_type'] != '') {
			$new_type = trim($_POST[$prefix.'_new_type']);
			if (!in_array($new_type, $search_types)) {
				$search_types[] = $new_type;
				$update = true;
			}
		}

		$settings['types'] = $search_types;
		return $asset->setAttrValue('settings', $settings);

	}//end processTypes()


	/**
	* Paint the current asset types that have been added
	*
	* @param object Listing_Engine		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCurrentAssetTypes(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$settings = $asset->attr('settings');
		$types = (isset($settings['types'])) ? $settings['types'] : Array();
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
		return true;

	}//end paintCurrentAssetTypes()


	/**
	* Process the current asset types that have been added
	*
	* @param object Listing_Engine		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processCurrentAssetTypes(&$asset, &$o, $prefix)
	{
		$settings = $asset->attr('settings');
		$types = (isset($settings['types'])) ? $settings['types'] : Array();

		// we are looking to see if any types have been removed
		$removed = array_get_index($_POST, $prefix.'_remove', Array());
		foreach ($removed as $type) {
			if (isset($types[$type])) {
				unset($types[$type]);
			}
		}

		if (!isset($_POST[$prefix.'_inherit_types'])) $_POST[$prefix.'_inherit_types'] = Array();
		for (reset($types); null !== ($k = key($types)); next($types)) {
			$types[$k] = (in_array($k, $_POST[$prefix.'_inherit_types'])) ? true : false;
		}

		$settings['types'] = $types;
		return $asset->setAttrValue('settings', $settings);

	}//end processCurrentAssetTypes()


	/**
	* Paint section to add an asset type
	*
	* @param object Listing_Engine		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddAssetType(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return false;

		$settings = $asset->attr('settings');
		$current_types = (isset($settings['types'])) ? $settings['types'] : Array();

		$types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();
		$types = array_diff($types, $current_types);
		asort($types);

		$uc_types = Array('' => '');
		foreach ($types as $type) $uc_types[$type] = ucwords(str_replace('_',' ', $type));
		combo_box($prefix.'_new_type', $uc_types, false, '');

	}//end paintAddAssetType()


	/**
	* Process the section to add a new format for an asset
	*
	* @param object Listing_Engine		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddAssetType(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return false;

		if (isset($_POST[$prefix.'_new_type'])) {

			$settings = $asset->attr('settings');
			$types = (isset($settings['types'])) ? $settings['types'] : Array();

			$type = trim($_POST[$prefix.'_new_type']);
			if (empty($type)) return false;
			if (!isset($types[$type])) {
				$types[$type] = true;
				$settings['types'] = $types;
				return $asset->setAttrValue('settings', $settings);
			}
		}


		return false;

	}//end processAddAssetType()


}//end class

?>
