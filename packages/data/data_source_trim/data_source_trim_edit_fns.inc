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
* $Id: data_source_trim_edit_fns.inc,v 1.1.2.2 2008/06/26 04:34:05 hnguyen Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source_edit_fns.inc';

/**
* Data_Source_DB_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.1.2.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Data_Source_TRIM_Edit_Fns extends Data_Source_Edit_Fns
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

		$var_list = $asset->attr('dynamic_vars');

		// MAYBE: detect keywords in SQL and inject them into variables

		$var_list_classified	= Array (
									'String Search Options'					=> Array (
																				'search_keyword', 'string_search_type'
																			   ),
									'Record Number Range Search Options'	=> Array (
																		   		'search_start_rec_num', 'search_end_rec_num', 'use_expanded_number',
																			   ),
									'Data Range Search Options'				=> Array (
																				'search_start_time', 'search_end_time', 'search_date_range_type',
																		   	   ),
									'General Search Options'				=> array_keys($var_list),
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
				$value = $var_list[$var];
				$var_id = base64_encode($var);

				?> <tr>
					<td>
				<?php
				if ($write_access) {
					echo urlencode($var);
					//text_box($prefix.'_var_name['.$var_id.']', urlencode($var));
				} else {
					echo urlencode($var);
				}
				?> </td>
				<td valign="top" align="left">
				<?php

				$list = Array();

				$search_sort = FALSE;
				if (urlencode($var) == 'fields') {
					$list = $asset->getRecordProperties();
				} else if (strpos(urlencode($var), 'search_sort') !== FALSE) {
					$search_sort = TRUE;
					$list['None']	= 'None';
					$list += $asset->getRecordProperties();
				} else if (urlencode($var) == 'string_search_type') {
					$list = $asset->getRecordStringSearchClauseType();
				} else if (urlencode($var) == 'search_date_range_type') {
					$list = $asset->getRecordDateRangeSearchClauseType();
				}//end if

				if ($write_access) {
					if ($search_sort) {
						combo_box($prefix.'_var_value['.$var_id.']', $list, FALSE, $value, 0, ' style="width:200px;"');
					} else if (is_array($value)) {
						$not_multiple = (bool) (strpos(urlencode($var), 'search_sort') === FALSE);
						combo_box($prefix.'_var_value['.$var_id.']', $list, $not_multiple, $value, 6, ' style="width:200px;"', 40);
					} else if (	strpos(urlencode($var), 'descending') !== FALSE || strpos(urlencode($var), 'use_expanded_number') !== FALSE) {
						combo_box($prefix.'_var_value['.$var_id.']', Array (TRUE => 'Yes', FALSE => 'No'), FALSE, $value);
					} else {
						text_box($prefix.'_var_value['.$var_id.']', urlencode($value), '','',FALSE, 'style="width:200px;"');
					}
				} else {
					if (is_array($value)) {
						?><ul style="margin: 1px 15px;"><?php
						foreach ($value as $key => $val) {
							echo '<li>'.$val.'</li>';
						}//end foreach
						?></ul><?php
					} else if (	strpos(urlencode($var), 'descending') !== FALSE || strpos(urlencode($var), 'use_expanded_number') !== FALSE) {
						echo ($value) ? 'Yes' : 'No';
					} else {
						echo urlencode($value);
					}//end else
				}
				?> </td>
				</tr>
			<?php
			unset($var_list[$var]);

			}//end foreach

		}//end foreach

		?>	</table> <?php



		$o->sectionNote(translate('trim_ds_search_default_options_note'));

		return $write_access;

	}//end paintDynamicData()


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

	}//end processDynamicData()


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


}//end class
?>
