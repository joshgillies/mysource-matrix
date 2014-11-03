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
* $Id: data_source_edit_fns.inc,v 1.12 2013/01/25 04:53:40 cupreti Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Data_Source_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Data_Source_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paints the interface for entering the name of the shadow asset
	*
	* @param Data_Source		$asset	the asset we are handling
	* @param Backend_Outputter	$o		the backend outputter
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintShadow(Data_Source $asset, Backend_Outputter $o, $prefix)
	{
		$name = $asset->attr('shadow_name');
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			text_box('shadow_name', !empty($name) ? $name : '', 80);
		} else {
			if (!empty($name)) echo $name;
		}

		return $wa;

	}//end paintShadow()


	/**
	* Processes the interface for entering and changin the where clause.
	*
	* @param Data_Source		$asset	the asset we are handling
	* @param Backend_Outputter	$o		the backend outputter
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function processShadow(Data_Source $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		$new_name = NULL;
		$name = $asset->attr('shadow_name');
		if (!empty($_POST['shadow_name'])) {
			$new_name = $_POST['shadow_name'];
		}
		if ($name != $new_name) {
			$res = $asset->setAttrValue('shadow_name', $new_name);
			return TRUE;
		} else {
			return FALSE;
		}

	}//end processShadow()


	/**
	* Paints the interface for entering the name of the shadow asset
	*
	* @param Data_Source		$asset	the asset we are handling
	* @param Backend_Outputter	$o		the backend outputter
	* @param string				$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	public function paintKeywords(Data_Source $asset, Backend_Outputter $o, $prefix)
	{
		$shadow_asset = $asset->getAsset($asset->id.':0');
		if (is_null($shadow_asset)) {
			echo '<strong>'.translate('No keywords available - this data source, as currently configured, does not have any record sets.').'</strong>';

			return;
		}
		$keywords = $shadow_asset->getAvailableKeywords();
		$write_access = $asset->writeAccess('attributes');
		?>
		<table class="sq-backend-table">
			<tr>
				<th><?php echo translate('Keyword');?></th>

				<th><?php echo translate('Description');?></th>

			</tr>
			<?php
			foreach ($keywords as $key => $desc) {
				// all the keywords other than the shadow asset name are to be displayed for the user to choose the
				// keywords or a combination of keywords and strings to set the shadow asset name
				if ($key == 'data_source_record_set_shadow_asset_name' || $key == 'ds__shadow_asset_name') {
					unset($keywords[$key]);
					continue;
				} else {
					if (strpos($key, 'data_source_record_set') === FALSE && strpos($key, 'ds__') === FALSE) {
						unset($keywords[$key]);
						continue;
					}
				}
				?>
				<tr>
					<td><?php echo '%'.$key.'%';?> </td>
					<td><?php echo $desc; ?> </td>
				</tr>
			<?php
			}
		?>
		</table>
		<?php

		return $write_access;

	}//end paintKeywords()





	/**
	* Processes the interface for entering and changin the where clause.
	*
	* @param Data_Source		$asset	the asset we are handling
	* @param Backend_Outputter	$o		the backend outputter
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function processKeywords(Data_Source $asset, Backend_Outputter $o, $prefix)
	{
		return FALSE;

	}//end processKeywords()


	/**
	* Paints the interface for filtering the records from the datasource result
	*
	* @param Data_Source		$asset	the asset we are handling
	* @param Backend_Outputter	$o		the backend outputter
	* @param string				$prefix	the prefix for this asset
	*
	* @return boolean
	* @access public
	*/
	public function paintRecordFilter(&$asset, &$o, $prefix)
	{
		$filters = $asset->attr('record_filters');
		$wa = $asset->writeAccess('attributes');
		$o->openField('','wide_col');
			?>
			<table class ="sq-backend-table" border="1" style="margin-bottom: 7px">
				<tr>
					<th><?php echo translate('Record Set Field Name'); ?></th>

					<th><?php echo translate('Value to Match'); ?></th>

					<th><?php echo translate('Match Type'); ?></th>

					<th><?php echo translate('Match Case'); ?></th>

					<th><?php echo translate('Allow Empty'); ?></th>

					<?php
					if ($wa) {
						?>
						<th style="text-align:center;"><?php echo translate('Delete'); ?>?</th>

						<?php
					}
					?>
				</tr>
				<?php

				$match_options = Array(
									'exact' => 'String match - Exact',
									'contains' => 'String match - Contains',
									'regex' => 'Regex match',
									);
				$case_options = Array(
									'ignore_case' => 'No',
									'match_case' => 'Yes',
								);
				$allow_empty_options = Array(
									0 => 'No',
									1 => 'Yes',
								);


				foreach ($filters as $index => $field_data) {
					$field_name = $field_data['name'];
					$field_value = $field_data['value'];
					$match_type = $field_data['match_type'];
					$match_case = $field_data['match_case'];
					$allow_empty = array_get_index($field_data, 'allow_empty', 0);

					?>
					<tr>
						<td>
							<?php
								if ($wa) {
									text_box($prefix.'_filter['.$index.'][name]', $field_name, 20);
								} else {
									echo $field_name;
								}
							?>
						</td>
						<td>
							<?php
								if ($wa) {
									text_box($prefix.'_filter['.$index.'][value]', $field_value, 60);
								} else {
									echo $field_value;
								}
							?>
						</td>
						<td>
							<?php
								if ($wa) {
									combo_box($prefix.'_filter['.$index.'][match_type]', $match_options, FALSE, $match_type);
								} else {
									echo $match_options[$match_type];
								}
							?>
						</td>
						<td>
							<?php
								if ($wa) {
									combo_box($prefix.'_filter['.$index.'][match_case]', $case_options, FALSE, $match_case);
								} else {
									echo $case_options[$match_case];
								}
							?>
						</td>
						<td>
							<?php
								if ($wa) {
									combo_box($prefix.'_filter['.$index.'][allow_empty]', $allow_empty_options, FALSE, $allow_empty);
								} else {
									echo $allow_empty_options[$allow_empty];
								}
							?>
						</td>
						<?php
							if ($wa) {
								echo '<td style="text-align:center;">';
								check_box($prefix.'_filter['.$index.'][delete]');
								echo '</td>';
							}
						?>


					</tr>
				<?php
				}
				?>
			</table>
			<div class="sq-backend-field-note">
				<strong><?php echo translate('Note'); ?>:</strong> <?php echo translate('Dynamic variables added in "Dynamic Inputs" screen can be used here in the "Value to Match" field in the form <strong>%%VARIABLENAME%%.</strong>'); ?>


			</div>
			<?php
		$o->closeField();
		if ($wa) {
			$o->openField(translate('Add New Condition'));

				label(translate('Record Set Field Name'), $prefix.'_new_filter_name');

				echo '&nbsp;';
				text_box($prefix.'_new_filter_name', '', 25);
			$o->closeField();
		}


		return $wa;

	}//end paintRecordFilter()


	/**
	* Process the interface for filtering the records from the datasource result
	*
	* @param Data_Source		$asset	the asset we are handling
	* @param Backend_Outputter	$o		the backend outputter
	* @param string				$prefix	the prefix for this asset
	*
	* @return boolean
	* @access public
	*/
	public function processRecordFilter(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		$updated_filters = Array();
		if (isset($_POST[$prefix.'_filter'])) {
			foreach($_POST[$prefix.'_filter'] as $index => $filter_data) {
				if (isset($filter_data['delete'])) {
					continue;
				}
				$filter_name = trim($filter_data['name']);
				$filter_value = $filter_data['value'];
				$match_type = $filter_data['match_type'];
				$match_case = $filter_data['match_case'];
				$allow_empty = $filter_data['allow_empty'];

				// Validate the regex string
				if ($match_type == 'regex' && !empty($filter_value)) {
					$valid_regex = @preg_match('/'.str_replace('/', '\/',$filter_value).'/', 'test');
					if ($valid_regex === FALSE) {
						trigger_localised_error('DATA0007', sprintf(translate('Specified string "%1$s" is not a valid regular expression'), $filter_value), E_USER_WARNING);
						$filter_value = '';
					}
				}

				$updated_filters[] = Array(
									'name' => $filter_name,
									'value' => $filter_value,
									'match_type' => $match_type,
									'match_case' => $match_case,
									'allow_empty' => $allow_empty,
								);
			}//end foreach
		}//end if

		if (!empty($_POST[$prefix.'_new_filter_name'])) {
			$updated_filters[] = Array(
								'name' => trim($_POST[$prefix.'_new_filter_name']),
								'value' => '',
								'match_type' => 'contains',
								'match_case' => 'ignore_case',
								'allow_empty' => 0,
							);
		}

		return $asset->setAttrValue('record_filters', $updated_filters);

	}//end processRecordFilter()


	/**
	* Find out if there are any tables selected or not
	*
	* @param Data_Source	$asset	The asset that might use the files
	* @param string			$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	public function resultSetExists(&$asset, $prefix)
	{
		$result = $asset->getResultSet();
		return !empty($result);

	}//end resultSetExists()


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

		$var_list = $asset->attr('dynamic_vars');

		$o->openRaw();

		echo '<table class="sq-backend-table">';
		echo '<tr>';
		echo '<th>Name</th>';
		echo '<th>Default Value</th>';
		if ($write_access) {
			echo '<th width="10%" align="right" style="border-left: dotted 1px grey;">Delete</th>';
		}
		echo '</tr>';
		foreach ($var_list as $var => $value) {
			$var_id = base64_encode($var);

			echo '<tr>';
			echo '<td>';
			if ($write_access) {
				text_box($prefix.'_var_name['.$var_id.']', $var);
			} else {
				echo $var;
			}
			echo '</td>';
			echo '<td valign="top" align="left">';
			$value = urldecode($value);
			if ($write_access) {
				text_box($prefix.'_var_value['.$var_id.']', urlencode($value));
			} else {
				echo urlencode($value);
			}
			echo '</td>';
			if ($write_access) {
				echo '<td style="border-left: dotted 1px grey;">';
				check_box($prefix.'_delete_var['.$var_id.']');
				echo '</td>';
			}
			echo '</tr>';
		}

		if ($write_access) {
			echo '<tr class="new-data-row">';
			echo '<th colspan="3" align="left">';
			echo '<em style="color:#666">Add New Variables Below</em>';
			echo '</th>';
			echo '</tr>';

			echo '<tr class="new-data-row">';
			echo '<td>';
			text_box($prefix.'_new_dynamic_data_name[]', NULL);
			echo '</td>';
			echo '<td>';
			text_box($prefix.'_new_dynamic_data_value[]', NULL);
			echo '</td>';
			echo '<td style="border-left: dotted 1px grey;">';
			echo '</td>';
			echo '</tr>';
		}
		echo '</table>';

		$o->closeRaw();

		$o->sectionNote(translate('Variables added in this section can be used in Record Filters and in other applicable fields in the form <strong>%%VARIABLENAME%%</strong>. Spaces in variable names are not allowed. Variables are case sensitive, i.e. "name" is not the same as "NAME". Any variable added to this form becomes available in the "Data Mappings" section below. Default value is used if a data mapping has not been setup or produces no result. Variable names can only contain letters (A-Z,a-z), digits (0-9), underscore (_), dash (-) and dot (.)'));


		$o->closeSection();

		$o->openSection('Data Mappings');
		$o->openRaw();
		$map = $asset->getAttribute('parameter_map');

		$parameter_names = $map->getParameters();

		foreach ($var_list as $var_name => $default) {
			$map->setParameter($var_name, 'Variable: '.$var_name);
		}

		$map->paint($prefix.'_parameter_map', !$write_access);
		$o->closeRaw();

		$o->sectionNote(translate('Use this section to setup dynamic assignment of values to variables.'));


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

		$map->process($prefix.'_parameter_map');

		foreach ($var_value as $hash => $value) {
			$name = base64_decode($hash);
			if (isset($to_delete[$hash])) {
				unset($var_list[$name]);
				// also need to delete it from the PARAMETER MAP
				if (in_array($name, $parameter_names)) {
					$map->unsetParameter($name);
				}

				continue;
			}
			$value = urldecode($value);
			$var_list[$name] = urlencode($value);
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
				$new_var_value = urldecode($new_var_value);
				$var_list[$new_var_name] = urlencode($new_var_value);
				$map->setParameter($new_var_name, 'Variable: '.$new_var_name);
			}
		}

		$asset->setAttrValue('dynamic_vars', $var_list);
		$asset->setAttrValue('parameter_map', unserialize($map->value));

		return TRUE;

	}//end processDynamicData()


}//end class

?>
