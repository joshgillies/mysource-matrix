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
* $Id: search_folder_edit_fns.inc,v 1.16 2005/08/10 04:12:29 tbarrett Exp $
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
* @version $Revision: 1.16 $
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
									'details'		=> Array(
														'name'			=> 'Details',
														'force_unlock'	=> false,
														'lock_type'		=> 'attributes',
													   ),
									'permissions'	=> Array(
														'name'			=> 'Permissions',
														'force_unlock'	=> true,
														'lock_type'		=> 'permissions',
													   ),
									'linking'		=> Array(
														'name'			=> 'Linking',
														'force_unlock'	=> true,
														'lock_type'		=> 'links',
													   ),
									'history'		=> Array(
														'name'			=> 'History',
														'force_unlock'	=> true,
														'lock_type'		=> 'none',
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
			if (!empty($current)) {
				$root_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current);
			}

			if (!is_null($root_asset)) {
				echo $root_asset->name.' (Id: #'.$root_asset->id.')';
			} else {
				echo translate('sch_folder_no_search_root');
			}
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
				echo translate('sch_folder_no_search_phrase');
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
			combo_box($prefix.'_logic', Array('and' => translate('sch_folder_logic_and'), 'or' => translate('sch_folder_logic_or')), false, $current);
		} else {
			echo translate('sch_folder_logic_'.$current);
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
			foreach ($statuses as $constant => $value) {
				$options[$value] = get_status_description($value);
			}

			combo_box($prefix.'_statuses', $options, true, $current);
		} else {
			if (empty($current)) {
				echo translate('sch_folder_no_status_selected');
			} else {
				echo '<ul>';
				foreach ($current as $status) {
					echo '<li>'.get_status_description($status).'</li>';
				}
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
						<td class="sq-backend-table-header"><?php echo translate('sch_searched_asset_types') ?></td>
						<?php
						if ($write_access) {
							?><td class="sq-backend-table-header"><?php echo translate('delete_question') ?></td><?php
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
			echo translate('sch_no_asset_types_selected');
		}


		if ($write_access) {
			$asset_types = array_values(array_diff($GLOBALS['SQ_SYSTEM']->am->getTypeList(), $current));
			$asset_type_list = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset_types, 'name');
			if (!empty($asset_type_list)){
				asort($asset_type_list);
				echo '<br /><span class="sq-backend-field">'.translate('sch_add_new_type').': </span>';
				combo_box($prefix.'_new_type', Array('' => '- '.translate('sch_select_asset_type').' -') + $asset_type_list, false, 'none');
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
	function paintAssetTypes(&$asset, &$o, $prefix)
	{
		$settings = $asset->attr('settings');
		$types = array_get_index($settings, 'types', Array());
		if ($asset->writeAccess('attributes')) {
			// print the form fields
			$display_values = Array('type_code' => Array(), 'inherit' => Array());
			foreach ($types as $type => $inherit) {
				$display_values['type_code'][] = $type;
				$display_values['inherit'][] = $inherit;
			}
			asset_type_chooser($prefix.'_types', true, $display_values, null, false, true);
			return true;
		} else {
			// print the read-only version
			if (empty($types)) {
				echo '<p class="sq-backend-warning">'.translate('sch_no_asset_types_selected').'</p>';
			} else {
				$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(array_keys($types), 'name');
				?>
					<table class="sq-backend-table">
						<tr>
							<th><?php echo translate('type'); ?></th>
							<th><?php echo translate('inherit_question'); ?></th>
						</tr>
				<?php
				$inherit_image_path = sq_web_path('lib').'/web/images/';
				foreach ($types as $type => $inherit) {
					$inherit_image = $inherit_image_path.($inherit ? 'tick' : 'cross').'.gif';
					$inherit_alt = $inherit ? translate('yes') : translate('no');
					?>
						<tr>
							<td><?php echo $type_names[$type]; ?></td>
							<td><img src="<?php echo $inherit_image; ?>" alt="<?php echo $inherit_alt; ?>" /></td>
						</tr>
					<?php
				}
				?>
					</table>
				<?php
			}
			return false;
		}

	}//end paintAssetTypes()


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
	function processAssetTypes(&$asset, &$o, $prefix)
	{
		$settings = $asset->attr('settings');

		$submitted_types = Array();
		foreach ($_POST[$prefix.'_types']['type_code'] as $i => $type_code) {
			if (!empty($type_code)) {
				$submitted_types[$type_code] = $_POST[$prefix.'_types']['inherit'][$i];
			}
		}

		$settings['types'] = $submitted_types;
		return $asset->setAttrValue('settings', $settings);

	}//end processAssetTypes()


}//end class

?>
