<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: sharepoint_saved_search_edit_fns.inc,v 1.4 2009/01/05 22:55:57 hnguyen Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source_edit_fns.inc';
require_once SQ_PACKAGES_PATH.'/sharepoint/lib/sharepoint_common.inc';

/**
* Sharepoint_Saved_Search_Edit_Fns
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __sharepoint__
*/
class Sharepoint_Saved_Search_Edit_Fns extends Data_Source_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	//-- Dynamic Data --//


	/**
	* Paint the dynamic data interface
	*
	* @param object	&$asset	The asset that might use the files
	* @param string	&$o		Backend outputter
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function paintDynamicData(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$current_search_options	= $asset->attr('search_options');

		$available_operators	= Array (
									'>'	=> '>',
									'<'	=> '<',
									'='	=> '=',
								  );


		$o->openRaw();
		$connection	= $asset->getConnection();
		$data_list	= $asset->attr('data_list');
		$available_fields	= Array();
		if (!empty($data_list) && $data_list != 'None') {
			$available_fields	= Sharepoint_Common::getListAvailableFields($connection, $data_list, TRUE);

		}//end if

		$options	= array_get_index($current_search_options, $data_list, Array());
			if (!empty($options)) {
		?>

			<span class="sq-backend-section-subheading" style="margin-left:60px;">Search Logic</span><br />
			<?php combo_box($prefix.'_search_logic', Array('And' => 'AND', 'Or' => 'OR'), FALSE, 'And', 0, ' style="margin:7 0 20 120px;"'); ?>
			<br />


			<p class="sq-backend-section-subheading" style="margin-left:60px;">Criterion</p>
			<table class="sq-backend-table" style="width:auto; margin-left:120px;">
				<th>Field Name</th>
				<th>Operator</th>
				<th>Value</th>
				<?php if ($write_access) { ?>
				<th>Delete?</th>
				<?php } ?>
			<?php



						foreach ($options as $field_name => $crit) {
							$field_name_info	= isset($available_fields[$field_name]) ? $available_fields[$field_name] : '';
							if ($write_access) {
							?>
								<tr>
									<td style="width:250px;"><p><?php echo $field_name_info.' ( '.$field_name.' )'; ?></p></td>
									<td><?php combo_box($prefix.'_new_search_operator', $available_operators, FALSE, $crit['operator']); ?></td>
									<td><p><?php text_box($prefix.'_url', $crit['value'], 30); ?></p></td>
									<td><?php check_box($prefix.'_delete_crit['.$data_list.']['.$field_name.']'); ?></td>
								</tr>

							<?php
							} else {
							?>
								<tr>
									<td style="width:250px;"><p><?php echo $available_fields[$field_name].' ( '.$field_name.' )'; ?></p></td>
									<td align="center"><?php echo $available_operators[$crit['operator']]; ?></td>
									<td><p><?php echo $crit['value']; ?></p></td>
								</tr>

							<?php
							}// end else
						}//end foreach

			?>
			</table>
			<br /><br />
			<?php
			}//end if empty current_search_options
					if ($write_access) {
						?>
						<p class="sq-backend-section-subheading" style="margin-left:60px;">Add New Criteria</p>
						<table class="sq-backend-table" style="width:auto; margin-left:120px;">
							<th>Field Name</th>
							<th>Operator</th>
							<th>Value</th>
							<tr>
								<td style="width:250px;"><?php combo_box($prefix.'_new_search_criteria_field', $available_fields, FALSE); ?></td>
								<td><?php combo_box($prefix.'_new_search_criteria_operator', $available_operators, FALSE); ?></td>
								<td><p><?php text_box($prefix.'_new_search_criteria_value', '', 30); ?></p></td>
							</tr>
						</table>
						<?php

					}//end if
					?>

		<?php
		$o->closeRaw();



		return $write_access;

	}//end paintDynamicData()


	/**
	* Process the dynamic data interface
	*
	* @param object	&$asset	The asset that might use the files
	* @param string	&$o		Backend outputter
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function processDynamicData(&$asset, &$o, $prefix)
	{
		$criteria_field		= array_get_index($_POST, $prefix.'_new_search_criteria_field', NULL);
		$criteria_operator	= array_get_index($_POST, $prefix.'_new_search_criteria_operator', NULL);
		$criteria_value		= array_get_index($_POST, $prefix.'_new_search_criteria_value', NULL);

		$current_search_options	= $asset->attr('search_options');
		$current_data_list		= $asset->attr('data_list');

		$new_data_list	= array_get_index($_POST, $prefix.'_data_list', '');
		if ($new_data_list != $current_data_list) {
			$asset->setAttrValue('data_list', $new_data_list);
		}//end if
		$current_data_list		= $asset->attr('data_list');

		if ($criteria_field != 'None' && !empty($criteria_value)) {
			$new_criteria	= Array (
								'operator'	=> $criteria_operator,
								'value'		=> $criteria_value,
							  );

			$current_search_options[$current_data_list][$criteria_field]	= $new_criteria;
		}//end if


		$delete_array	= array_get_index($_POST, $prefix.'_delete_crit', Array());

		if (!empty($delete_array) && isset($delete_array[$current_data_list])) {
			foreach ($delete_array[$current_data_list] as $remove_field_name => $value) {
				if (isset($current_search_options[$current_data_list][$remove_field_name])) {
					unset($current_search_options[$current_data_list][$remove_field_name]);
				}//end if
			}//end foreach
		}//end if

		$asset->setAttrValue('search_options', $current_search_options);

		return TRUE;

	}//end processDynamicData()


	/**
	* paint data mapping
 	*
	* @param object $asset
	* @param object $o
	* @param string $prefix
	*/
	function paintDataMapping(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$var_list = $asset->attr('dynamic_vars');

		$o->openSection('Data Mappings');
		$o->openRaw();
		$map = $asset->getAttribute('parameter_map');

		$parameter_names = $map->getParameters();

		foreach ($var_list as $var_name => $default) {
			$map->setParameter($var_name, 'Variable: '.$var_name);
		}

		$map->paint($prefix.'_parameter_map', !$write_access);
		$o->closeRaw();

		$o->sectionNote(translate('trim_ds_search_default_options_note'));

		return $write_access;

	}//end paintDataMapping()


	/**
	* process data mapping
 	*
	* @param object $asset
	* @param object $o
	* @param string $prefix
	*/
	function processDataMapping(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		if (!$write_access) return FALSE;
		$var_name_pattern = '/^[a-zA-Z_\-0-9\.]+$/';
		$var_list = $asset->attr('dynamic_vars');

		$map = $asset->getAttribute('parameter_map');

		$map->process($prefix.'_parameter_map');

		$asset->setAttrValue('parameter_map', unserialize($map->value));

		return $write_access;

	}//end processDataMapping()


	/**
	* paint record search type
 	*
	* @param object $asset
	* @param object $o
	* @param string $prefix
	 */
	function paintRecordSearchType(&$asset, &$o, $prefix)
	{
		$selected_type = $asset->attr('record_search_type');
		$write_access	= $asset->writeAccess('attributes');
		//echo 'Painting Record Search Type';
		$record_search_types	= Array();
		$record_search_types = $asset->getRecordSearchTypes();
		if ($write_access) {
			require_once SQ_LIB_PATH.'/html_form/html_form.inc';
			combo_box($prefix.'_search_types', $record_search_types, FALSE, $selected_type);
		} else {
			echo $record_search_types[array_pop($selected_type)];
		}//end else if

		return $write_access;

	}//end paintRecordSearchType()


	/**
	* process record search type
 	*
	* @param object $asset
	* @param object $o
	* @param string $prefix
	 */
	function processRecordSearchType(&$asset, &$o, $prefix)
	{
        if (!$asset->writeAccess('attributes')) return FALSE;

        $new_name = NULL;
        $search_type = $asset->attr('record_search_type');
        if (!empty($_POST[$prefix.'_search_types'])) {
            $new_search_type = $_POST[$prefix.'_search_types'];
			if ($search_type != $new_search_type) {
	            $res = $asset->setAttrValue('record_search_type', Array($new_search_type));
				$current_dyn_vars	= $asset->attr('dynamic_vars');
				if (!is_array($current_dyn_vars)) {
					return FALSE;
				}

				// Lets clean up the dynamic vars so we can put the new options in.
				$asset->cleanupDynamicVars($current_dyn_vars);

				$new_search_dyn_vars = Array();
				if ($new_search_type == 'StringSearch') {
					$new_search_dyn_vars	= Array (
												'search_keyword'		=> 'doc',
												'string_search_type'	=> Array (
																		'TitleWord'	=> 'TitleWord',
																	   ),
											  );

				} else if ($new_search_type == 'DateRangeSearch') {
					$new_search_dyn_vars	= Array (
												'StartTime'				=> '',
												'EndTime'				=> '',
												'dateRangeSearchType'	=> Array (
																			'DateCreated'	=> 'DateCreated',
																		   ),
											  );
				} else if ($new_search_type == 'NumberRangeSearch') {
					$new_search_dyn_vars	= Array (
												'StartNum'			=> '',
												'EndNum'			=> '',
												'UseExpandedNumber'	=> TRUE,
											  );
				}//end else if
				$new_search_dyn_vars += $current_dyn_vars;
				$asset->setAttrValue('dynamic_vars', $new_search_dyn_vars);

	            return TRUE;
	        } else {
	            return FALSE;
	        }
        }
        return TRUE;


	}//end processRecordSearchType()


	/**
	* paint the interface for editing wsdl url
 	*
	* @param object $asset
	* @param object $o
	* @param string $prefix
	* @return boolean
	*/
	function paintWsdlUrl(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$wsdl	= $asset->attr('url');
		if ($write_access) {
			require_once SQ_LIB_PATH.'/html_form/html_form.inc';
			text_box($prefix.'_sys_url', $wsdl, '35');
		} else {
			echo $wsdl;
		}//end else if

		return $write_access;

	}//end paintWsdlUrl()


	/**
	* prcoess the interface for editing wsdl url
	*
	* @param object $asset
	* @param object $o
	* @param string $prefix
	* @return boolean
	 */
	function processWsdlUrl(&$asset, &$o, $prefix)
	{
        if (!$asset->writeAccess('attributes')) return FALSE;

        $new_name = NULL;
        $old_wsdl = $asset->attr('url');
        if (!empty($_POST[$prefix.'_sys_url'])) {
            $new_wsdl = trim($_POST[$prefix.'_sys_url']);
	        if ($old_wsdl != $new_wsdl) {
	            $res = $asset->setAttrValue('url', $new_wsdl);
	            return TRUE;
	        } else {
	            return FALSE;
	        }
        }
        return TRUE;

	}//end processWsdlUrl()


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
	function paintConnectorLink(&$asset, &$o, $prefix)
	{
		// get root links
		$connector_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'sharepoint_connector', FALSE, 'sharepoint_connector');
		if (empty($connector_link)) {
			$connector_asset_id = NULL;
		} else {
			$connector_asset_id = $connector_link['minorid'];
		}

		if ($asset->writeAccess('links')) {
			asset_finder($prefix.'_connector_node', $connector_asset_id, Array('sharepoint_connector'=>'D'));
		} else {
			if (empty($connector_asset_id)) {
				echo 'No connector selected';
			} else {
				echo get_asset_tag_line($connector_asset_id);
			}
		}
		return TRUE;

	}//end paintConnectorLink()


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
	function processConnectorLink(&$asset, &$o, $prefix)
	{
		$new_connector_id = @$_REQUEST[$prefix.'_connector_node']['assetid'];

		if (!$asset->writeAccess('links')) {
			trigger_error('You do not have write access to change the connector link', E_USER_WARNING);
			return FALSE;
		}

		$current_connector_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'sharepoint_connector', FALSE, 'sharepoint_connector');
		$current_connector_asset_id = @$current_connector_link['minorid'];

		if (!empty($current_connector_link)) {
			// if the new connector id is not null and is equal to the current
			// connector asset id then return
			if ((!empty($new_connector_id)) && ($current_connector_asset_id === $new_connector_id)) {
				return TRUE;
			} else {
				// else means that we want to create a new link or we want to
				// clear the link
				$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($current_connector_link['linkid']);

				// if the new connector id is empty means we want to clear the
				// link we got nothing else to do so return TRUE
				if (empty($new_connector_id)) return TRUE;
			}
		}

		if (!empty($new_connector_id)) {
			$new_connector_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($new_connector_id);
			$result_id = $GLOBALS['SQ_SYSTEM']->am->createAssetLink($asset, $new_connector_asset, SQ_LINK_NOTICE, 'sharepoint_connector');

			if (empty($result_id)) {
			trigger_error('Could not link the connector to this asset', E_USER_WARNING);
			return FALSE;
		}

		}

		return TRUE;

	}//end processConnectorLink()


	/**
	* paint function for selecting list
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintListSelection(&$asset, $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$current_data_list	= $asset->attr('data_list');
		if ($current_data_list == '') $current_data_list = 'None';
		$connection	= $asset->getConnection();
		$list_names	= Array (
						'None'	=> 'None'	// We want to keep this option on top
					  );

		$valid = FALSE;
		if (isset($connection['url']) && !empty($connection['url'])) {
			$valid = Sharepoint_Common::test_sharepoint_connection($connection);
		}//end if
		if ($valid) {
			$all_lists	= Sharepoint_Common::getListCollections($connection);
			$sp_lists	= Sharepoint_Common::getListNames($connection, $all_lists);
			$list_names	+= $sp_lists;
		}//end if

		if (!$write_access) {
			if (isset($list_names[$current_data_list])) {
				echo $list_names[$current_data_list];
			}//end if
		} else {
			combo_box($prefix.'_data_list', $list_names, FALSE, $current_data_list);
		}//end else

	}//end paintListSelection()


	/**
	* process function for selecting list
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function processListSelection(&$asset, $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		if (!$write_access) return FALSE;

		$new_data_list	= array_get_index($_POST, $prefix.'_data_list', '');
		$current_data_list	= $asset->attr('data_list');
		if ($new_data_list != $current_data_list) {
			$asset->setAttrValue('data_list', $new_data_list);
		}//end if

		return TRUE;

	}//end processListSeletion()


	/**
	* paint function for synching
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintSynchCheck(&$asset, &$o, $prefix)
	{
		$wa	= $asset->writeAccess('attributes');
		$o->openRaw();
		?>
		<script type="text/javascript" src="<?php echo sq_web_path('data').'/asset_types/sharepoint_saved_search/js/ajax.js' ?>"></script>

		<div style="position:relative;top:0px;">
			<table style="width:auto;border-spacing:0px;" border="0">
				<tr>
					<td width="90"><span style="font-weight:bold;">Check Update</span><br /><br /></td>
					<td width="150"><span id="update_text">Not Checked</span><br /><br /></td>
					<td><?php //if ($wa) normal_button($prefix.'_update', 'Check for Updates', 'checkUpdate()'); ?><br /><br /></td>
					<td><?php if ($wa) normal_button($prefix.'_synch', 'Synchronize Now', 'sendRequest(\''.$asset->getURL().'?synch_now=1\', \'updateSynchInterface\');'); ?><br /><br /></td>
				</tr>
			</table>
		</div>

		<?php
		$o->closeRaw();

	}//end paintSynchCheck()


	/**
	* process function for synching
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function processSynchCheck(&$asset, &$o, $prefix)
	{

	}//end processSynchCheck()


}//end class
?>
