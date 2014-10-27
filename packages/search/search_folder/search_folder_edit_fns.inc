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
* $Id: search_folder_edit_fns.inc,v 1.27 2012/08/30 00:59:15 ewang Exp $
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
* @version $Revision: 1.27 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Folder_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

		$this->static_screens = Array(
									'details'		=> Array(
														'name'			=> 'Details',
														'force_unlock'	=> FALSE,
														'lock_type'		=> 'menu',
													   ),
									'permissions'	=> Array(
														'name'			=> 'Permissions',
														'force_unlock'	=> TRUE,
														'lock_type'		=> 'permissions',
													   ),
									'linking'		=> Array(
														'name'			=> 'Linking',
														'force_unlock'	=> TRUE,
														'lock_type'		=> 'links',
													   ),
									'history'		=> Array(
														'name'			=> 'History',
														'force_unlock'	=> TRUE,
														'lock_type'		=> 'none',
													   ),
								);

	}//end constructor


	/**
	* Paints the root node selection box
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRoot(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$settings = $asset->attr('settings');
		$current = (isset($settings['root'])) ? $settings['root'] : Array();

		if ($write_access) {
			multiple_asset_finder($prefix.'_roots', $current);
		} else {
			?><ul style="margin: 1px 15px"><?php
			foreach ($current as $assetid) {
				if (!empty($assetid)) {
					?><li><?php echo get_asset_tag_line($assetid) ?></li><?php
				} else {
					echo translate('No search root has been selected. The whole system will be searched.');

				}
			}
			?></ul><?php
		}

		return $write_access;

	}//end paintRoot()


	/**
	* Processes the value input from root node selection box
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	function processRoot(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;

		$settings = $asset->attr('settings');
		$settings['root'] = Array();

		if (is_array($_POST[$prefix.'_roots'])) {
			foreach ($_POST[$prefix.'_roots'] as $root_node) {
				if (!empty($root_node['assetid'])) {
					$settings['root'][] = $root_node['assetid'];
				}
			}
		}

		return $asset->setAttrValue('settings', $settings);

	}//end processRoot()


	/**
	* Paint the interface for entering a search phrase
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
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
				echo translate('No search terms have been entered. All assets will be searched.');

			} else {
				echo $current;
			}
		}

		return $write_access;

	}//end paintPhrase()


	/**
	* Processes the interface for entering a search phrase
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processPhrase(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;

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
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintLogic(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$options = Array(
			'and' => translate('Results include all terms'),
			'or'  => translate('Results include any terms'),
		);
		$settings = $asset->attr('settings');
		$current = (isset($settings['logic'])) ? $settings['logic'] : 'and';

		if ($write_access) {
			combo_box($prefix.'_logic', $options, FALSE, $current);
		} else {
			echo $options[$current];
		}

		return $write_access;

	}//end paintLogic()


	/**
	* Processes the interface for selecting search logic
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processLogic(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;

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
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
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

			combo_box($prefix.'_statuses', $options, TRUE, $current);
		} else {
			if (empty($current)) {
				echo translate('No statuses are selected');

			} else {
				?><ul style="margin: 1px 15px"><?php
				foreach ($current as $status) {
					echo '<li>';
					echo get_asset_status_icon($status);
					echo get_status_description($status);
					echo '</li>';
				}
				?></ul><?php
			}
		}

		return $write_access;

	}//end paintStatus()


	/**
	* Processes the interface for selecting searched statuses
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processStatus(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;

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
	* Paint the current asset types that have been added
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
		$settings = $asset->attr('settings');
		$types = array_get_index($settings, 'types', Array());
		if ($asset->writeAccess('attributes')) {
			// print the form fields
			$display_values = Array('type_code' => Array(), 'inherit' => Array());
			foreach ($types as $type => $inherit) {
				$display_values['type_code'][] = $type;
				$display_values['inherit'][] = $inherit;
			}
			asset_type_chooser($prefix.'_types', TRUE, $display_values, NULL, FALSE, TRUE);
			return TRUE;
		} else {
			// print the read-only version
			if (empty($types)) {
				echo '<p class="sq-backend-warning">'.translate('No asset types are selected').'</p>';

			} else {
				$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(array_keys($types), 'name');
				?>
					<table class="sq-backend-table">
						<tr>
							<th><?php echo translate('Type'); ?></th>

							<th><?php echo translate('Inherit?'); ?></th>

						</tr>
				<?php
				$inherit_image_path = sq_web_path('lib').'/web/images/';
				foreach ($types as $type => $inherit) {
					$inherit_image = $inherit_image_path.($inherit ? 'tick' : 'cross').'.gif';
					$inherit_alt = $inherit ? translate('Yes') : translate('No');


					?>
						<tr>
							<td><?php echo get_asset_type_icon($type).$type_names[$type]; ?></td>
							<td><img src="<?php echo $inherit_image; ?>" alt="<?php echo $inherit_alt; ?>" /></td>
						</tr>
					<?php
				}
				?>
					</table>
				<?php
			}
			return FALSE;
		}

	}//end paintAssetTypes()


	/**
	* Process the current asset types that have been added
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
