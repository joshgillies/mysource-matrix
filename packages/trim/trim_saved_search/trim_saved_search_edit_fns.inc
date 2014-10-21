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
* $Id: trim_saved_search_edit_fns.inc,v 1.16 2013/09/17 08:38:35 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source_edit_fns.inc';

/**
* TRIM_Saved_Search_Edit_Fns
*
* Purpose
*
*
* @author  Edison Wang <ewang@squiz.com.au>
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.16 $
* @package MySource_Matrix_Packages
* @subpackage __trim__
*/
class TRIM_Saved_Search_Edit_Fns extends Data_Source_Edit_Fns
{

	public $dsn = NULL;
	public $result = NULL;
	public $fields = Array();
	public $selected_fields = Array();
	public $group_by = Array();
	public $conn = NULL;


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
	* Paint the dynamic data interface
	*
	* @param object	&$asset	The asset that might use the files
	* @param string	&$o		Backend outputter
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function paintGeneralOptions(&$asset, &$o, $prefix)
	{

		require_once SQ_PACKAGES_PATH.'/trim/lib/trim_common.inc';
		$write_access = $asset->writeAccess('attributes');

		$var_list = $asset->attr('dynamic_vars');

		// MAYBE: detect keywords in SQL and inject them into variables

		$var_list_classified	= Array (
									'Search Parent Container'				=> Array (
																			   	'search_parent_container',
																			   ),
									'General Search Options'				=> Array (
																			   	'search_parent_container',
																			   	'fileTypes',
																			   	'limit',
																			   	'search_sort_1',
																			   	'search_sort_2',
																			   	'search_sort_3',
																			   	'sort_1_descending',
																			   	'sort_2_descending',
																			   	'sort_3_descending',
																			   ),
								  );

		foreach ($var_list_classified as $name => $options) {
			?>
			<table class="sq-backend-table">
			<tr>
			<th style="width:200px;"><?php echo $name; ?></th>
			<th>Default Value</th>
			</tr>
			<?php
			foreach ($options as $var) {
				if (!isset($var_list[$var])) continue;
				//these options moved out
				$value = $var_list[$var];
				$var_id = base64_encode($var);

				?> <tr>
					<td>
				<?php
				if ($write_access) {
					echo ucwords(str_replace('_', ' ', urlencode($var)));
				} else {
					echo ucwords(str_replace('_', ' ', urlencode($var)));
				}
				?> </td>
				<td valign="top" align="left">
				<?php

				$list = Array();

				$search_sort = FALSE;
				if (strpos(urlencode($var), 'search_sort') !== FALSE) {
					$search_sort = TRUE;
					$list['None']	= 'None';
					$list += TRIM_Common::getRecordSortProperties();
				} else if (urlencode($var) == 'search_date_range_type') {
					$list = TRIM_Common::getRecordDateRangeSearchClauseType();
				}//end if

				if ($write_access) {
					if ($search_sort) {
						combo_box($prefix.'_var_value['.$var_id.']', $list, FALSE, $value, 0, ' style="width:200px;"');
					} else if (is_array($value)) {
						$not_multiple = (bool) (strpos(urlencode($var), 'search_sort') === FALSE);
						combo_box($prefix.'_var_value['.$var_id.']', $list, $not_multiple, $value, 8, ' style="width:200px;"', 40);
					} else if (	strpos(urlencode($var), 'descending') !== FALSE || strpos(urlencode($var), 'use_expanded_number') !== FALSE) {
						combo_box($prefix.'_var_value['.$var_id.']', Array (TRUE => 'Yes', FALSE => 'No'), FALSE, $value);
					} else {
						text_box($prefix.'_var_value['.$var_id.']', $value, '','',FALSE, 'style="width:200px;"');
					}
				} else {
					if (is_array($value)) {
						?><ul style="position:relative;left:-41px;list-style:none;"><?php
						foreach ($value as $key => $val) {
							echo '<li>'.$val.'</li>';
						}//end foreach
						?></ul><?php
					} else if (	strpos(urlencode($var), 'descending') !== FALSE || strpos(urlencode($var), 'use_expanded_number') !== FALSE) {
						echo ($value) ? 'Yes' : 'No';
					} else {
						echo $value;
					}//end else
				}
				?> </td>
				</tr>
			<?php
			unset($var_list[$var]);

			}//end foreach

		}//end foreach

		?>	</table> <?php



		$o->sectionNote(translate('These options above set the default value for TRIM record searching. Spaces in variable names are not allowed. Variables are case sensitive, i.e. "name" is not the same as "NAME". Any variable added to this form becomes available in the "Data Mappings" section below. Default value is used if a data mapping has not been setup or produces no result. Variable names can only contain letters (A-Z,a-z), digits (0-9), underscore (_), dash (-) and dot (.)'));


		return $write_access;

	}//end paintGeneralOptions()


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

		$o->sectionNote(translate('These options above set the default value for TRIM record searching. Spaces in variable names are not allowed. Variables are case sensitive, i.e. "name" is not the same as "NAME". Any variable added to this form becomes available in the "Data Mappings" section below. Default value is used if a data mapping has not been setup or produces no result. Variable names can only contain letters (A-Z,a-z), digits (0-9), underscore (_), dash (-) and dot (.)'));


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
	* Process the dynamic data interface
	*
	* @param object	&$asset	The asset that might use the files
	* @param string	&$o		Backend outputter
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function processGeneralOptions(&$asset, &$o, $prefix)
	{
		$var_name_pattern = '/^[a-zA-Z_\-0-9\.]+$/';

		$write_access = $asset->writeAccess('attributes');
		if (!$write_access) return FALSE;

		$to_delete = array_get_index($_REQUEST, $prefix.'_delete_var', Array());
		$var_value = array_get_index($_REQUEST, $prefix.'_var_value', Array());
		$var_name = array_get_index($_REQUEST, $prefix.'_var_name', Array());

		$var_list = $asset->attr('dynamic_vars');

		$map = $asset->getAttribute('parameter_map');

		$parameter_names = $map->getParameters();
		foreach ($var_list as $name => $default) {
			$map->setParameter($name, 'Variable: '.$name);
		}

		foreach ($var_value as $hash => $value) {
			$name = base64_decode($hash);
			if (isset($to_delete[$hash])) {
				unset($var_list[$name]);

				continue;
			}

			$var_list[$name] = $value;
		}

		$new_var_list = Array();
		$rename_success = FALSE;

		// now we rename variables
		foreach ($var_name as $hash => $new_name) {
			$new_name = trim($new_name);

			if (empty($new_name)) {
				$rename_success = FALSE;
				trigger_error('Variable name cannot be empty. No variables were renamed.', E_USER_NOTICE);
				break;
			}

			// check new name against regex
			$new_name_is_valid = preg_match($var_name_pattern, $new_name);
			if (!$new_name_is_valid) {
				$rename_success = FALSE;
				trigger_error('Variable name ['.htmlentities($new_name).'] contains characters that are not allowed. Can only use letters (A-Za-z), digits(0-9), underscore (_), dash (-) and dot (.)', E_USER_NOTICE);
				break;
			}

			$name = base64_decode($hash);

			// skip the deleted vars
			if (!isset($var_list[$name])) continue;

			if (isset($new_var_list[$new_name])) {
				$rename_success = FALSE;
				trigger_error('Could not rename variables because variable name "'.htmlentities($new_name).'" already exists', E_USER_NOTICE);
				break;
			}

			if ($new_name != $name) {
				if (in_array($name, $parameter_names)) {
					$map->moveParameter($name, $new_name);
				}
				$rename_success = TRUE;
			}

			$new_var_list[$new_name] = $var_list[$name];

		}//end foreach


		if ($rename_success) $var_list = $new_var_list;

		$new_names = array_get_index($_REQUEST, $prefix.'_new_dynamic_data_name', Array());
		if (!empty($new_names)) {

			foreach ($new_names as $seq_number => $new_var_name) {
				$new_var_name = trim($new_var_name);

				$new_var_value = array_get_index($_REQUEST[$prefix.'_new_dynamic_data_value'], $seq_number, '');
				if (empty($new_var_name)) {
					if (!empty($new_var_value)) {
						trigger_error('New variable name cannot be empty', E_USER_WARNING);
					}
					continue;
				}

				// check new name against regex
				$new_name_is_valid = preg_match($var_name_pattern, $new_var_name);
				if (!$new_name_is_valid) {
					trigger_error('Variable name "'.htmlentities($new_var_name).'" contains characters that are not allowed. Can only use letters (A-Za-z), digits(0-9), underscore (_), dash (-) and dot (.)', E_USER_NOTICE);
					continue;
				}

				if (isset($var_list[$new_var_name])) {
					trigger_error('Variable Name "'.htmlentities($new_var_name).'" is already in use', E_USER_WARNING);
					continue;
				}

				$var_list[$new_var_name] = $new_var_value;
				$map->setParameter($new_var_name, 'Variable: '.$new_var_name);
			}
		}

		$asset->setAttrValue('dynamic_vars', $var_list);
		$asset->setAttrValue('parameter_map', unserialize($map->value));

		return TRUE;

	}//end processGeneralOptions()



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
		$wsdl	= $asset->attr('wsdl');
		if ($write_access) {
			require_once SQ_LIB_PATH.'/html_form/html_form.inc';
			text_box($prefix.'_wsdl_url', $wsdl, '35');
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
        $old_wsdl = $asset->attr('wsdl');
        if (!empty($_POST[$prefix.'_wsdl_url'])) {
            $new_wsdl = trim($_POST[$prefix.'_wsdl_url']);
	        if ($old_wsdl != $new_wsdl) {
	            $res = $asset->setAttrValue('wsdl', $new_wsdl);
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
		$connector_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'trim_connector', FALSE, 'trim_connector');
		if (empty($connector_link)) {
			$connector_asset_id = NULL;
		} else {
			$connector_asset_id = $connector_link['minorid'];
		}

		if ($asset->writeAccess('links')) {
			asset_finder($prefix.'_connector_node', $connector_asset_id, Array('trim_connector'=>'D'));
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

		$current_connector_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'trim_connector', FALSE, 'trim_connector');
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
			$result_id = $GLOBALS['SQ_SYSTEM']->am->createAssetLink($asset, $new_connector_asset, SQ_LINK_NOTICE, 'trim_connector');

			if (empty($result_id)) {
			trigger_error('Could not link the connector to this asset', E_USER_WARNING);
			return FALSE;
		}

		}

		return TRUE;

	}//end processConnectorLink()


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
		//
		?>
		<script type="text/javascript" src="<?php echo sq_web_path('data').'/asset_types/trim_saved_search/js/ajax.js' ?>">
		</script>
		<div>
			<table style="width:auto;border-spacing:0px;" border="0">
				<tr>
					<td width="90"><span style="font-weight:bold;">Check Update</span></td>
					<td width="150"><span id="update_text">Not Checked</span></td>
					<td><?php if ($wa) normal_button($prefix.'_update', 'Check for Updates', 'sendRequest(\''.$asset->getURL().'?check_update=1\', \'updateCheckInterface\');'); ?></td>
					<td><?php if ($wa) normal_button($prefix.'_synch', 'Synchronize Now', 'sendRequest(\''.$asset->getURL().'?synch_now=1\', \'updateSynchInterface\');'); ?></td>
					<td><?php if ($wa) normal_button($prefix.'_force', 'Force Update', 'sendRequest(\''.$asset->getURL().'?force_update=1\', \'updateSynchInterface\');'); ?></td>
					<td><img id="trim_saved_search_loader" src="<?php echo sq_web_path('data').'/asset_types/trim_saved_search/images/loader.gif'?>" style="display:none;" height="20" width"20" /></td>
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
	* paint function for retrieving fields from TRIM server
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintRetrieveFields(&$asset, &$o, $prefix)
	{
		$wa	= $asset->writeAccess('attributes');
		$fields = $asset->attr('retrieved_fields');
		$standard_fields = isset ($fields['standard_fields']) ? count($fields['standard_fields']) : 0;
		$additional_fields = isset ($fields['additional_fields']) ? count($fields['additional_fields']) : 0;
		$record_types = isset ($fields['record_types']) ? count($fields['record_types']) : 0;
		?>
		<table>
		<tr>
		<td>
		<?php
		if (!empty($_POST[$prefix.'_retrieve'])) echo '<span style="color:green;">';
		echo (empty($fields) ? 'Empty' : $standard_fields.' standard fields, '.$additional_fields.' additional fields and '.$record_types.' record types retrieved');
		if (!empty($_POST[$prefix.'_retrieve'])) echo '</span>';
		?>
		</td>
		<td>
		<?php
		if ($wa) submit_button($prefix.'_retrieve', 'Retrieve Now', '');
		?>
		</td>
		</tr>
		</table>
		<?php
		return $wa;

	}//end paintSynchCheck()


	/**
	* process function for retrieving fields from TRIM server
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function processRetrieveFields(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		$connection = $asset->getConnection();

        if (!empty($_POST[$prefix.'_retrieve']) && !empty($connection)) {
			// get all fields form TRIM server
			$st_fields = Array();
			$udf_fields = Array();
			$fields_type = Array();
			try {
				$client = new SoapClient($connection['wsdl'], $connection['authentication']);
				$data = $client->ListProperties(Array('ObjectName'	=> 'Record'));
				foreach ($data->ListPropertiesResult->PropertyDescription as $field) {
					if (preg_match('/^udf:/', (String) $field->Name)) {
						$udf_fields[(String) $field->Name] = ((String) $field->Caption);
					}
					else {
						$st_fields[(String) $field->Name] = ((String) $field->Caption);
					}
					// record fields type
					$fields_type[(String) $field->Name] = (String) $field->Type;
					// add custom matrix fields in correct order
					if((String) $field->Name == 'recContents') $st_fields['recContainerTitle'] = 'Container Title';
				}


			} catch (SoapFault $e) {
				trigger_error($e->getMessage(), E_USER_WARNING);
				return FALSE;
			}//end if


			//retrieve record types info
			$record_types = TRIM_Common::getAllRecordTypes($connection);
			$asset->setAttrValue('retrieved_fields', Array('standard_fields' => $st_fields, 'additional_fields' => $udf_fields, 'field_types' => $fields_type, 'record_types' => $record_types));

		}
        return TRUE;
	}//end processRetrieveFields()


	/**
	* Paints the interface for adding and configuring search fields.
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintSearchFields(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$var_list = $asset->attr('dynamic_vars');
		$search_groups = isset($var_list['search_clauses']['search_groups']) ? $var_list['search_clauses']['search_groups'] : Array();
		$global_logic = isset($var_list['search_clauses']['global_logic']) ? $var_list['search_clauses']['global_logic'] : 'AND';

		//get possible search fields
		$available_fields = Array('none'	=>	'-- '.ucfirst(translate('Add search field').' --'));

		$list = $asset->getRetrievedFields('additional');
		$available_fields += TRIM_Common::getRecordStringSearchClauseType() + $list;

		//work out total rows
		$total_rows = 0;
		foreach($search_groups as $group_data) {
				$total_rows += count($group_data['content']);
				if ($write_access) $total_rows ++;
		}

		?>
		<table class ="sq-backend-table" border="1" style="margin-bottom: 7px; ">
			<tr>
				<th rowspan="2"><?php echo translate('Groups'); ?></th>

				<th colspan="<?php echo $write_access ? 3 : 2; ?>"><?php echo translate('Search Fields'); ?></th>

				<th rowspan="2"><?php echo translate('Group Logic'); ?></th>

				<?php
				if ($write_access) {
					?>
					<th rowspan="2"><?php echo translate('Delete'); ?></th>

					<?php
				}
				?>
				<th rowspan="2"><?php echo translate('Global Logic'); ?></th>

			</tr>
			<tr>
				<th><?php echo translate('Name');?></th>

				<th><?php echo translate('Keyword');?> </th>

				<?php
				if ($write_access) {
					?>
					<th><?php echo translate('Delete'); ?></th>

					<?php
				}
				?>
			</tr>
			<?php
			foreach ($search_groups as $group_index => $group_data) {
				if ($write_access) $group_data['content'][] = Array('caption' => 'new');

				foreach ($group_data['content'] as $field_index => $field_data) {
					?>
					<tr>
						<?php
						if ($field_index == 0) {
						?>
						<td rowspan="<?php echo (count($group_data['content'])); ?>">Group <?php echo ($group_index+1); ?></td>
						<?php
						}
						?>
						<td class="sq-backend-data" style="padding: 4px;">
							<?php
							if ($field_data['caption'] == 'new') {
								if ($write_access) {
									$count  = count($group_data['content']);
									combo_box($prefix.'_fields['.$group_index.']['.$count.'][new_search_field]', $available_fields, FALSE,'none');
								}
							} else {
								echo $field_data['caption'];
								//display field type
								$field_type = $asset->getRetrievedFieldType($field_data['name']);
								echo (empty($field_type) ? '' : ' ('.$field_type.')');
							}
							?>
						</td>
						<td>
							<?php
							if ($field_data['caption'] != 'new') {
								if($write_access)
									text_box($prefix.'_fields['.$group_index.']['.$field_index.'][keyword]', $field_data['keyword'], '35');
								else
									echo $field_data['keyword'];
							}//end if
							?>
						</td>
						<?php
						if ($write_access) {
							// Delete field Cell
							?>
							<td class="sq-backend-data">
								<?php
								if ($field_data['caption'] != 'new') {
									check_box($prefix.'_fields['.$group_index.']['.$field_index.'][delete]');
								}
								?>
							</td>
							<?php
						}
						if ($field_index == 0) {
							// Logic Cell
							?>
							<td rowspan="<?php echo count($group_data['content']);?>" style="vertical-align:middle; text-align:center;">
								<?php
								$group_logic_options = Array('AND' => translate('AND'), 'OR' => translate('OR'));


								if ($write_access) {
									combo_box($prefix.'_fields['.$group_index.'][group_logic]', $group_logic_options, FALSE, array_get_index($group_data, 'group_logic', 'AND'), 0, 'title="'.translate('Use this logic operator to connect search fields').'"');

								} else {
									echo array_get_index($group_data, 'group_logic', 'AND');
								}
								?>
							</td>
							<?php
						}

						if (($field_index == 0) && $write_access) {
							// Delete field overall cell
							?>
							<td rowspan="<?php echo count($group_data['content']);?>">
								<?php
								check_box($prefix.'_fields['.$group_index.'][delete]');
								?>
							</td>
							<?php
						}

						if ($group_index == 0 && $field_index == 0) {
							// Global Logic Cell
							?>
							<td rowspan="<?php echo $total_rows; ?>" style="vertical-align:middle; text-align:center;">
								<?php
								$global_logic_options = Array('AND' => translate('AND'), 'OR' => translate('OR'));


								if ($write_access) {
									combo_box($prefix.'_fields[global_logic]', $global_logic_options, FALSE, $global_logic, 0, 'title="'.translate('Use this logic operator to connect search groups').'"');

								} else {
									echo $global_logic;
								}
								?>
							</td>
							<?php
						}

						?>
						</tr>

						<?php
					}//end foreach
				}//end foreach */
				?>
			</tr>

		</table>
		<?php
		if ($write_access) {
			echo ('<div class="sq-backend-smallprint">'.translate('Add search field in a new group').'</div>');

			$count = count($search_groups);
			combo_box($prefix.'_fields['.$count.'][new_search_group]', $available_fields, FALSE,'none');
		}
		return TRUE;

	}//end paintSearchFields()


	/**
	* Processes the interface for adding and configuring search fields.
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processSearchFields(&$asset, &$o, $prefix)
	{

		if (!$asset->writeAccess('attributes')) return FALSE;
		$var_list = $asset->attr('dynamic_vars');
		$search_groups = Array();
		$global_logic = '';

		$available_fields = Array();
		$list = $asset->getRetrievedFields('additional');
		$available_fields = TRIM_Common::getRecordStringSearchClauseType() + $list;

		if (isset($_POST[$prefix.'_fields'])) {
			// set global logic
			if(isset($_POST[$prefix.'_fields']['global_logic'])) {
				$global_logic = $_POST[$prefix.'_fields']['global_logic'];
				unset($_POST[$prefix.'_fields']['global_logic']);
			}
			else {
				$global_logic = 'AND';
			}

			foreach ($_POST[$prefix.'_fields'] as $group_index => $group) {
				// set group logic
				if(isset($group['group_logic'])) {
					$group_logic = $group['group_logic'];
					unset($group['group_logic']);
				}
				else {
					$group_logic = 'AND';
				}


				if(isset($group['delete'])) continue;
				if(isset($group['new_search_group']) && $group['new_search_group'] === 'none')	continue;
				if(isset($group['new_search_group']) && $group['new_search_group'] != 'none') {
					// build a search field with a new group
					$new_group = Array();
					$field_name = $group['new_search_group'];
					$field_caption = isset($available_fields[$field_name]) ? $available_fields[$field_name] : '';
					$new_group['group_logic'] = 'AND';
					$new_group['content'][] = Array('name' => $field_name, 'caption' => $field_caption, 'keyword' => '');

				}
				else {
					$new_group = Array();
					$new_group['group_logic'] = $group_logic;
					$new_group['content'] = Array();

					foreach ($group as $field_index => $field) {
						if(isset($field['delete'])) continue;
						if(isset($field['new_search_field']) && $field['new_search_field'] === 'none')	continue;
						if(isset($field['new_search_field']) && $field['new_search_field'] != 'none') {
							//build new field
							$field_name = $field['new_search_field'];
							$field_caption = isset($available_fields[$field_name]) ? $available_fields[$field_name] : '';
							$keyword = '';
						}
						else {
							// process normal fields
							$old_field = $var_list['search_clauses']['search_groups'][$group_index]['content'][$field_index];
							$field_name = $old_field['name'];
							$field_caption = $old_field['caption'];
							$keyword = $field['keyword'];
						}
						$new_group['content'][] = Array('name' => $field_name, 'caption' => $field_caption, 'keyword' => $keyword);
					}
				}
				if(!empty($new_group['content'])) $search_groups[] = $new_group;
			}
		}
		$var_list['search_clauses']['search_groups'] = $search_groups;
		$var_list['search_clauses']['global_logic'] = $global_logic;

		return ($asset->setAttrValue('dynamic_vars', $var_list));

	}//end processSearchFields()



	/**
	* Paints the interface for adding and deleting result fields.
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintResultFields(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$var_list = $asset->attr('dynamic_vars');
		$result_fields = isset($var_list['result_fields']) ? $var_list['result_fields'] : Array();

		//get possible search fields
		$available_fields = Array('none'	=>	'-- '.ucfirst(translate('Add result field').' --'));

		$list = $asset->getRetrievedFields('all');
		$available_fields += !empty($list) ? $list : TRIM_Common::getRecordProperties();


		?>
		<table class ="sq-backend-table" border="1" style="margin-bottom: 7px; ">

			<tr>
				<th><?php echo translate('Name');?></th>

				<th><?php echo translate('Type');?> </th>

				<th><?php echo translate('Category');?> </th>

				<?php
				if ($write_access) {
					?>
					<th><?php echo translate('Delete'); ?></th>

					<?php
				}
				?>
			</tr>
			<?php
				foreach ($result_fields as $name => $caption) {
					?>
					<tr>
						<td class="sq-backend-data" style="padding: 4px;">
							<?php
								echo $caption;
							?>
						</td>
						<td>
							<?php
								$field_type = $asset->getRetrievedFieldType($name);
								echo (empty($field_type) ? '' : $field_type);
							?>
						</td>
						<td>
							<?php
								echo(preg_match('/^udf:/', $name) ? translate('Additional Field') : translate('Standard Field'));


							?>
						</td>
						<?php
						if ($write_access) {
							// Delete field Cell
							?>
							<td class="sq-backend-data">
								<?php
									check_box($prefix.'_result_fields['.$name.'][delete]');
								?>
							</td>
							<?php
						}


				}//end foreach
				?>
			</tr>
		</table>
		<?php
		if ($write_access) {
			echo ('<div class="sq-backend-smallprint">'.translate('Tell TRIM server which infomation to be returned').'</div>');

			combo_box($prefix.'_result_fields[new]', $available_fields, FALSE,'none');
		}

		return TRUE;

	}//end paintResultFields()


	/**
	* Processes the interface for adding and deleting result fields
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processResultFields(&$asset, &$o, $prefix)
	{

		if (!$asset->writeAccess('attributes')) return FALSE;
		$var_list = $asset->attr('dynamic_vars');
		$result_fields = isset($var_list['result_fields']) ? $var_list['result_fields'] : Array();


		//get possible search fields
		$list = $asset->getRetrievedFields('all');
		$available_fields = !empty($list) ? $list : TRIM_Common::getRecordProperties();

		if (isset($_POST[$prefix.'_result_fields'])) {
			foreach ($_POST[$prefix.'_result_fields'] as $field => $value) {
				if (isset($field['delete']) && isset($result_fields[$field])) {
					unset($result_fields[$field]);
				}
			}
			if(isset($_POST[$prefix.'_result_fields']['new']) && $_POST[$prefix.'_result_fields']['new'] !== 'none') {
				$new_field = $_POST[$prefix.'_result_fields']['new'];
				$field_caption = isset($available_fields[$new_field]) ? $available_fields[$new_field] : '';
				$result_fields[$new_field] = $field_caption;
			}
		}

		$var_list['result_fields'] = $result_fields;
		return ($asset->setAttrValue('dynamic_vars', $var_list));

	}//end processResultFields()



	/**
	* Paints the interface for record  type filter
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintRecordTypeFilter(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$retrieved = $asset->attr('retrieved_fields');
		$available_record_types = Array('none'	=>	'-- '.ucfirst(translate('Add record type').' --'));

		$available_record_types += isset($retrieved['record_types']) ? $retrieved['record_types'] : Array();

		$var_list = $asset->attr('dynamic_vars');
		$record_types = isset($var_list['record_type_filter']) ? $var_list['record_type_filter'] : Array();


		?>
		<table class ="sq-backend-table" border="1" style="margin-bottom: 7px; width:300px;">

			<tr>
				<th><?php echo translate('Name');?></th>

				<?php
				if ($write_access) {
					?>
					<th><?php echo translate('Delete'); ?></th>

					<?php
				}
				?>
			</tr>
			<?php
				foreach ($record_types as $uri => $name) {
					?>
					<tr>
						<td class="sq-backend-data" style="padding: 4px;">
							<?php
								echo $name;
							?>
						</td>

						<?php
						if ($write_access) {
							// Delete field Cell
							?>
							<td class="sq-backend-data">
								<?php
									check_box($prefix.'_record_type_filter['.$uri.'][delete]');
								?>
							</td>
							<?php
						}


				}//end foreach
				?>
			</tr>
		</table>
		<?php
		if ($write_access) {
			echo ('<div class="sq-backend-smallprint">'.translate('Only include above record types.').'</div>');

			combo_box($prefix.'_record_type_filter[new]', $available_record_types, FALSE,'none');
			if(count($available_record_types) ==1 ) {
				echo ('<div class="sq-backend-smallprint">'.translate('No record types found, please retrieve from TRIM server.').'</div>');

			}
		}

		return TRUE;

	}//end paintRecordTypeFilter()


	/**
	* Processes the interface for adding and deleting record types filter
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processRecordTypeFilter(&$asset, &$o, $prefix)
	{

		if (!$asset->writeAccess('attributes')) return FALSE;
		$var_list = $asset->attr('dynamic_vars');
		$record_types = isset($var_list['record_type_filter']) ? $var_list['record_type_filter'] : Array();


		//get possible record types
		$retrieved = $asset->attr('retrieved_fields');
		$available_record_types = isset($retrieved['record_types']) ? $retrieved['record_types'] : Array();

		if (isset($_POST[$prefix.'_record_type_filter'])) {
			foreach ($_POST[$prefix.'_record_type_filter'] as $field => $value) {
				if (isset($value['delete']) && isset($record_types[$field])) {
					unset($record_types[$field]);
				}
			}
			if(isset($_POST[$prefix.'_record_type_filter']['new']) && $_POST[$prefix.'_record_type_filter']['new'] !== 'none') {
				$new_uri = $_POST[$prefix.'_record_type_filter']['new'];
				$record_name = isset($available_record_types[$new_uri]) ? $available_record_types[$new_uri] : '';
				$record_types[$new_uri] = $record_name;
			}
		}

		$var_list['record_type_filter'] = $record_types;
		return ($asset->setAttrValue('dynamic_vars', $var_list));

	}//end processResultFields()


	/**
	* Paints the interface for setting up the download file name
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDownloadedFileName(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$current_value = $asset->attr('filename_field');

		$var_list = $asset->attr('dynamic_vars');
		$result_fields = isset($var_list['result_fields']) ? $var_list['result_fields'] : Array();
		// Append an empty option
		array_unshift($result_fields, '');

		if ($write_access) {
			combo_box($prefix.'_filename_field', $result_fields, FALSE, $current_value);
		} else {
			echo array_get_index($result_fields, $current_value, '');
		}

		return TRUE;

	}//end paintDownloadedFileName()


	/**
	* Process the interface for setting up the download file name
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processDownloadedFileName(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			return FALSE;
		}

		$new_value = isset($_POST[$prefix.'_filename_field']) ? $_POST[$prefix.'_filename_field'] : FALSE;
		if ($new_value === FALSE) {
			return FALSE;
		}

		$current_value = $asset->attr('filename_field');
		if ($current_value == $new_value) {
			return TRUE;
		}

		return $asset->setAttrValue('filename_field', $new_value);

	}//end processDownloadedFileName()

}//end class
?>
