<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: sharepoint_saved_search_edit_fns.inc,v 1.8 2012/08/30 00:59:19 ewang Exp $
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
* @version $Revision: 1.8 $
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
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	//-- Dynamic Data --//


	/**
	* Paint the search configuration interface
	*
	* @param object	&$asset	The asset that might use the files
	* @param string	&$o		Backend outputter
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function paintSearchConfig(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$current_search_options	= $asset->attr('search_options');

		$available_operators	= Array (
									'>'				=> '>',
									'>='			=> '>=',
									'<'				=> '<',
									'<='			=> '<=',
									'='				=> '=',
									'!='			=> '!=',
									'Contains'		=> 'Contains',
									'BeginsWith'	=> 'BeginsWith',
									'IsNotNull'		=> 'IsNotNull',
									'IsNull'		=> 'IsNull',
								  );


		$o->openRaw();

		$connection			= $asset->getConnection();
		$data_list			= $asset->attr('data_list');
		$search_logic		= $asset->attr('search_logic');
		$row_limit			= $asset->attr('row_limit');
		$available_fields	= Array( 'None' => '');

		if (!empty($data_list) && $data_list != 'None') {
			$sp_available_fields	= Sharepoint_Common::getListAvailableFields($connection, $data_list, TRUE);

			//(refer to http://www.sharepointblogs.com/ssa/archive/2007/10/03/invalid-column-name-c2.aspx )
			if (isset($sp_available_fields['LinkFilename'])) {
				unset($sp_available_fields['LinkFilename']);
			}//end if
			if (isset($sp_available_fields['LinkFilenameNoMenu'])) {
				unset($sp_available_fields['LinkFilenameNoMenu']);
			}//end if
			if (isset($sp_available_fields['BaseName'])) {
				unset($sp_available_fields['BaseName']);
			}//end if

			foreach ($sp_available_fields as $raw_name => $display_name) {
				$sp_available_fields[$raw_name]	= $display_name.' ('.$raw_name.')';
			}//end foreach

			$available_fields  += $sp_available_fields;
		}//end if

		$options	= array_get_index($current_search_options, $data_list, Array());

		?>
			<span class="sq-backend-section-subheading" style="margin-left:60px;">Row Limit</span><br />
		<?php
			if (!$write_access) {
				echo '<div style="margin:15px 0px 20px 120px;font-weight:bold;">'.$row_limit.'</div>';
			} else {
				text_box($prefix.'_row_limit', $row_limit, 4, '', FALSE, 'style="margin:15px 0px 20px 120px;"');
			}//end else

		if (!empty($options)) {
		?>
			<br />
			<span class="sq-backend-section-subheading" style="margin-left:60px;">Search Logic</span><br />
			<?php
			if (!$write_access) {
				echo '<div style="margin:15px 0px 20px 120px;font-weight:bold;">'.strtoupper($search_logic).'</div>';
			} else {
				combo_box($prefix.'_search_logic', Array('And' => 'AND', 'Or' => 'OR'), FALSE, $search_logic, 0, ' style="margin:15px 0px 20px 120px;"');
			}//end else

			?>
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



						foreach ($options as $index => $field_info) {
							$field_name	= key($field_info);
							$crit	= $field_info[$field_name];
							$field_name_info	= isset($available_fields[$field_name]) ? $available_fields[$field_name] : '';

							if ($write_access) {
							?>
								<tr>
									<td style="width:330px;"><p><?php combo_box($prefix.'_edit_field_name['.$index.']', $available_fields, FALSE, $field_name); ?></p></td>
									<td><?php combo_box($prefix.'_edit_op['.$index.']', $available_operators, FALSE, $crit['operator']); ?></td>
									<td><p><?php text_box($prefix.'_edit_val['.$index.']', $crit['value'], 50); ?></p></td>
									<td><?php check_box($prefix.'_delete_crit['.$data_list.']['.$field_name.']'); ?></td>
								</tr>

							<?php
							} else {
							?>
								<tr>
									<td style="width:330px;"><p><?php echo $available_fields[$field_name]; ?></p></td>
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
								<td style="width:330px;"><?php combo_box($prefix.'_new_search_criteria_field', $available_fields, FALSE); ?></td>
								<td><?php combo_box($prefix.'_new_search_criteria_operator', $available_operators, FALSE); ?></td>
								<td><p><?php text_box($prefix.'_new_search_criteria_value', '', 50); ?></p></td>
							</tr>
						</table>
						<?php

					}//end if
					?>

		<?php
		$o->closeRaw();



		return $write_access;

	}//end paintSearchConfig()


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
	function processSearchConfig(&$asset, &$o, $prefix)
	{
		$criteria_field		= array_get_index($_POST, $prefix.'_new_search_criteria_field', NULL);
		$criteria_operator	= array_get_index($_POST, $prefix.'_new_search_criteria_operator', NULL);
		$criteria_value		= array_get_index($_POST, $prefix.'_new_search_criteria_value', NULL);
		$search_logic		= array_get_index($_POST, $prefix.'_search_logic', 'And');
		$row_limit			= array_get_index($_POST, $prefix.'_row_limit', '');


		if (!empty($row_limit) && $row_limit > 0) {
			$asset->setAttrValue('row_limit', $row_limit);
		}//end if

		$editing_criterion	= array_get_index($_POST, $prefix.'_edit_val', Array());
		$editing_operator	= array_get_index($_POST, $prefix.'_edit_op', Array());
		$editing_field_name	= array_get_index($_POST, $prefix.'_edit_field_name', Array());

		$current_search_options	= $asset->attr('search_options');
		$current_data_list		= $asset->attr('data_list');

		if (isset($current_search_options[$current_data_list])) {
			foreach ($current_search_options[$current_data_list] as $index	=> $field_info) {
				foreach ($editing_criterion as $edit_val_index => $new_value) {
					if (isset($current_search_options[$current_data_list][$edit_val_index])) {
						$field_crit_info_1	= $current_search_options[$current_data_list][$edit_val_index];
						$field_crit_value	= $field_crit_info_1[key($field_crit_info_1)]['value'];
						if ($field_crit_value != $new_value) {
							$current_search_options[$current_data_list][$edit_val_index][key($field_crit_info_1)]['value']	= $new_value;
						}//end if
					}//end if
				}//end foreach

				foreach ($editing_operator as $edit_op_index => $new_operator) {
					if (isset($current_search_options[$current_data_list][$edit_op_index])) {
						$field_crit_info_2	= $current_search_options[$current_data_list][$edit_op_index];
						$field_crit_op	= $field_crit_info_2[key($field_crit_info_2)]['operator'];
						if ($field_crit_op != $new_operator) {
							$current_search_options[$current_data_list][$edit_op_index][key($field_crit_info_2)]['operator']	= $new_operator;
						}//end if
					}//end if
				}//end foreach

				foreach ($editing_field_name as $edit_fn_index => $new_field_name) {
					if (isset($current_search_options[$current_data_list][$edit_fn_index])) {
						$field_crit_info_3	= $current_search_options[$current_data_list][$edit_fn_index];
						$field_crit_fn	= key($field_crit_info_3);
						if ($field_crit_fn != $new_field_name) {
							$current_search_options[$current_data_list][$edit_fn_index][$new_field_name]	= $field_crit_info_3[$field_crit_fn];
							unset($current_search_options[$current_data_list][$edit_fn_index][$field_crit_fn]);
						}//end if
					}//end if
				}//end foreach
			}//end foreach
		}//end if

		$new_data_list	= array_get_index($_POST, $prefix.'_data_list', '');
		if ($new_data_list != $current_data_list && !empty($new_data_list)) {
			$asset->setAttrValue('data_list', $new_data_list);
		}//end if
		$current_data_list		= $asset->attr('data_list');

		if (!empty($criteria_field) && $criteria_field != 'None' && !is_null($criteria_value)) {
			$new_criteria	= Array (
								'operator'	=> $criteria_operator,
								'value'		=> $criteria_value,
							  );

			$current_search_options[$current_data_list][]	= Array (
																$criteria_field	=> $new_criteria,
															  );
		}//end if

		$delete_array	= array_get_index($_POST, $prefix.'_delete_crit', Array());

		if (!empty($delete_array) && isset($delete_array[$current_data_list])) {
			foreach ($current_search_options[$current_data_list] as $index => $field_info) {
				foreach ($delete_array[$current_data_list] as $remove_field_name => $value) {
					if (key($field_info) == $remove_field_name) {
						unset($current_search_options[$current_data_list][$index]);
					}//end if
				}//end foreach
			}//end foreach
		}//end if

		$asset->setAttrValue('search_logic', $search_logic);
		$asset->setAttrValue('search_options', $current_search_options);

		return TRUE;

	}//end processSearchConfig()


    /**
     * Test connection
     *
     * @param object $asset
     * @param object $o
     * @param object $prefix
     *
     * @return boolean
     * @access public
     */
    function paintTestConnection(&$asset, &$o, $prefix)
    {
		$connection	= $asset->getConnection();

    	$wa = $asset->writeAccess('attributes');
			require_once SQ_PACKAGES_PATH.'/sharepoint/lib/sharepoint_common.inc';

			$valid = FALSE;
			if (!empty($connection['url'])) {
				$valid = Sharepoint_Common::test_sharepoint_connection($connection, TRUE);
			}//end if

			if ($valid) {
				echo '<span style="color:green;font-weight:bold">Connected</span>';
			} else {
				echo '<span style="color:red;font-weight:bold">Not Connected</span>';
			}//end if

		return $wa;

    }//end paintTestConnection()


    /**
     * Test connection
     *
     * @param object $asset
     * @param object $o
     * @param object $prefix
     *
     * @return boolean
     * @access public
     */
    function processTestConnection(&$asset, &$o, $prefix)
    {
		return FALSE;
    }//end processTestConnection()


	/**
	* paint the interface for editing wsdl url
 	*
	* @param object $asset
	* @param object $o
	* @param string $prefix
	* @return boolean
	*/
	function paintSystemUrl(&$asset, &$o, $prefix)
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

	}//end paintSystemUrl()


	/**
	* prcoess the interface for editing wsdl url
	*
	* @param object $asset
	* @param object $o
	* @param string $prefix
	* @return boolean
	 */
	function processSystemUrl(&$asset, &$o, $prefix)
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

	}//end processSystemUrl()


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

	
	/**
	* Paint function for Download File
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintDownloadFile(&$asset, $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		
		$download_file_attr = $asset->attr('download_file');
		$options = Array (
					0 => "No",
					1 => "Yes",
				);
		if (!$write_access) {			
			echo $download_file_attr ? "Yes" : "No";			
		} else {
			combo_box($prefix.'_download_file', $options, FALSE, $download_file_attr);
		}//end else

	}//end paintDownloadFile


	/**
	* Process function for Download File
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function processDownloadFile(&$asset, $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		if (!$write_access) return FALSE;
		$download_file_attr	= array_get_index($_POST, $prefix.'_download_file', 1);
		$current_download_file_attr = $asset->attr('download_file');

		if ($current_download_file_attr != $download_file_attr) {
			$asset->setAttrValue('download_file', $download_file_attr);
		}//end if

		return TRUE;

	}//end processDownloadFile()


}//end class
?>
