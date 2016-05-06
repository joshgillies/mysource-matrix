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
*/

require_once SQ_ATTRIBUTES_PATH.'/parameter_map/parameter_map.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';


/**
* Dynamic File Asset Attribute
* Generates a file(s) dynamically in the temp data directory based on attribute current settings
*
* Asset_Attribute_Dynamic_File::generateFiles() generates the files based on attribute current value and returns file paths
* Its upto the function using this attribute to clean up the dynamic files once its done with generated files (Asset_Attribute_Dynamic_File::cleanUpGeneratedFiles())
*
* NOTE: Currently this this attribute is used for generating file attachments for email. However this can be used for any other generic purpose.
*
* <!-- Sample Edit Interface XML Node -->
*	<dynamic_file show_design_to_apply="1" show_dynamic_params="1" show_additional_get_params="1" />
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_Dynamic_File extends Asset_Attribute
{

	/**
	* Create location dir to keep dynamically generated files
	*
	* @param array
	*/
	var $_create_location;


	/**
	* File paths of dynamically generated files
	*
	* @param array
	*/
	var $_generated_files;


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Dynamic_File($attributeid=0, $value=NULL, $default_params=Array())
	{
		if (!isset($this->_edit_params)) {
			$this->_edit_params = Array();
		}

		if (is_null($value)) {
			$value = Array();
		}

		if (!empty($default_params)) {
			foreach($default_params as $param => $value) {
				$this->_edit_params[$param] = $value;
			}//end foreach
		} else {
			$this->_edit_params['show_design_to_apply'] = TRUE;
			$this->_edit_params['show_dynamic_params'] = TRUE;
			$this->_edit_params['show_additional_get_params'] = TRUE;
		}

		$this->_create_location = '';
		$this->_generated_files = Array();

		$this->Asset_Attribute($attributeid, $value);

	}//end constructor


    /**
    * Takes an XML_Tree_Node from an edit interface XML file and creates
    * the params array from it
    *
    * @param object &$node  XML_Tree_Node object.
    *
    * @return boolean
    * @access public
    */
    function setEditParams(&$node)
    {
        if (!parent::setEditParams($node)) return FALSE;
        foreach ($this->_edit_params as $field => $val) {
            if (isset($node->attributes()->$field)) {
                // Cannot convert directly to boolean, since (bool) of any non-empty
                // object is TRUE. Convert to int first, then to bool. (Bug #4095)
                $this->_edit_params[$field] = (bool)(int)$node->attributes()->$field;
            }
        }
        return TRUE;

    }//end setEditParams()


	/**
	* Adds a new parameter to the list of those available
	*
	* @param string $code   Parameter code
	* @param string $label  Friendly name describing the parameter
	*
	* @return void
	* @access public
	*/
	function setParameter($code, $label)
	{
		$this->_edit_params['parameters'][$code] = $label;

	}//end setParameter()


	/**
	* Print the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form elements
	* @param boolean	$read_only	are we just printing the value
	*
	* @return void
	* @access public
	*/
	function paint($prefix, $read_only=FALSE)
	{
		$main_prefix = str_replace(':', '_', $prefix);

		$current_values = @unserialize($this->value);
		if (empty($current_values)) {
			$current_values = Array();
		}

		foreach($current_values as $index => $current_value) {
			$prefix = $main_prefix.'_'.$index;
			if (!isset($current_value['asset_to_send'])) {
				$current_value['asset_to_send'] = '';
			}
			if (!isset($current_value['filename'])) {
				$current_value['filename'] = '';
			}
			if (!isset($current_value['design_to_apply'])) {
				$current_value['design_to_apply'] = '';
			}
			if (!isset($current_value['layout_to_apply'])) {
				$current_value['layout_to_apply'] = '';
			}
			if (!isset($current_value['dynamic_parameters'])) {
				$current_value['dynamic_parameters'] = serialize(Array());
			}
			if (!isset($current_value['additional_get_vars'])) {
				$current_value['additional_get_vars'] = Array();
			}
			if (count($current_values) > 1) {
				$padding_style = $index > 0 ? ' style="padding-top:10px;"' : '';
				echo '<div'.$padding_style.'><p><strong>File Attachment #'.($index+1).'</strong></p></div>';
			}
			?>
			<table border="0" class="sq-backend-table with-bottom-margin">
				<tr>
					<th><?php echo translate('Asset To Send'); ?></th>
					<td>
						<?php
							if (!$read_only) {
								asset_finder($prefix.'_asset_to_send', $current_value['asset_to_send']);
							} else {
								if (!empty($current_value['asset_to_send'])) {
									echo get_asset_tag_line($current_value['asset_to_send']);
								}
							}
						?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('File Name'); ?></th>
					<td>
						<?php
							if ($read_only) {
								echo htmlspecialchars($current_value['filename']);
							} else {
								text_box($prefix.'_filename', $current_value['filename'], 100, '', FALSE, 'class="sq-full-cell-width sq-form-field"');
							}
						?>
					</td>
				</tr>
			<?php
			// These are optional parameters
			if ($this->_edit_params['show_design_to_apply']) {
				?>
				<tr>
					<th><?php echo translate('Design To Apply'); ?></th>
					<td>
						<?php
							if (!$read_only) {
								asset_finder($prefix.'_design_to_apply', $current_value['design_to_apply'], Array('design' => 'D'));
							} else {
								if (!empty($current_value['design_to_apply'])) {
									echo get_asset_tag_line($current_value['design_to_apply']);
								}
							}
						?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('Layout To Apply'); ?></th>
					<td>
						<?php
							if (!$read_only) {
								asset_finder($prefix.'_layout_to_apply', $current_value['layout_to_apply'], Array('paint_layout_page' => 'D'));
							} else {
								if (!empty($current_value['layout_to_apply'])) {
									echo get_asset_tag_line($current_value['layout_to_apply']);
								}
							}
						?>
					</td>
				</tr>
				<?php
			}

			if ($this->_edit_params['show_dynamic_params']) {
				?>
				<tr>
					<th><?php echo translate('Dynamic Parameters'); ?></th>

					<td>
						<?php
							$dynamic_parameters_attr = new Asset_Attribute_Parameter_Map(0, $current_value['dynamic_parameters']);
							$dynamic_parameters_attr->setParameter('asset_to_send', 'Asset To Send');
							$dynamic_parameters_attr->paint($prefix, $read_only);
						?>
					</td>
				</tr>
				<?php
			}
			if ($this->_edit_params['show_additional_get_params']) {
				?>
				<tr id = "<?php echo $prefix; ?>_additional_get_param">
					<th><?php echo translate('Additional GET Parameters'); ?></th>
					<td>
						<?php
							$delete_icon = '';
							if (!$read_only) {
								// Delete icon with the new lines replaced
								ob_start();
									sq_print_icon(sq_web_path('data').'/asset_types/bodycopy/images/icons/delete.png', 16, 16, translate('Delete'), translate('Delete'), ' style="border:none"');
								$delete_icon = ob_get_clean();
								$delete_icon = str_replace("\n", ' ', $delete_icon);

								if (empty($current_value['additional_get_vars'])) {
									$last_key = -1;
								} else {
									$last_key = max(array_keys($current_value['additional_get_vars']));
								}
							?>
							<script>
								var <?php echo $prefix ?>_object = {
										nextIndex: [],

										/**
										 * Adds a new row.
										 */
										addGetRow: function(prefix) {
											node = document.getElementById(prefix + '_additional_get_body');
											index = this.nextIndex[prefix];

											// create a new row
											tr = document.createElement('tr');
											tr.id = prefix + '_additional_get_row_' + index;

											// build the cells
											td1 = document.createElement('td');
											td1.innerHTML = '<?php text_box("' + prefix + '_additional_get[' + index + '][name]", '', 20) ?>';
											td2 = document.createElement('td');
											td2.innerHTML = '<?php text_box("' + prefix + '_additional_get[' + index + '][value]", '', 30) ?>'
											td3 = document.createElement('td');
											td3.style.textAlign = 'center';
											td3.innerHTML = '<a href="" onclick="<?php echo $prefix ?>_object.removeGetRow(\'' + prefix +'\', \'' + index + '\'); return false;"><?php echo $delete_icon; ?></a>';

											// Now put them all together
											tr.appendChild(td1);
											tr.appendChild(td2);
											tr.appendChild(td3);
											node.appendChild(tr);

											// Go to next index
											this.nextIndex[prefix]++;

											// Update display of "no variable" status
											this.updateNoVarStatus(prefix);
										},

										/*
										 * Removes a row.
										 */
										removeGetRow: function(prefix, index) {
											node = document.getElementById(prefix + '_additional_get_row_' + index);
											node.parentNode.removeChild(node);
											this.updateNoVarStatus(prefix);
										},

										/*
										 * Updates display of the "no variable" body.
										 */
										updateNoVarStatus: function(prefix) {
											node = document.getElementById(prefix + '_additional_get_body');
											var noVarBody = document.getElementById(prefix + '_additional_get_none');
											noVarBody.style.display = (node.getElementsByTagName('tr').length > 0) ? 'none' : 'block';
										}
									};

								// Set the first new index for newly created rows
								<?php echo $prefix ?>_object.nextIndex['<?php echo $prefix; ?>'] = <?php echo ($last_key + 1); ?>;
						</script>
						<?php
							}//end not read only
						?>
						<table class="sq-backend-table" style="margin: 5px 0;">
							<thead>
								<tr>
									<th width="30%"><?php echo translate('Variable Name'); ?></th>
									<th width="65%"><?php echo translate('Variable Value'); ?></th>
									<?php
										if (!$read_only) {
									?>
										<th style="text-align:center; width:5%"><?php echo translate('Delete?') ?></th>
									<?php
										}
									?>
								</tr>
							</thead>
							<tbody id="<?php echo $prefix ?>_additional_get_none" style="display: <?php echo empty($current_value['additional_get_vars'])  ? 'block' : 'none'; ?>">
								<tr>
									<td colspan="3"><em><?php echo translate('No variables are set.'); ?></em></td>
								</tr>
							</tbody>
							<tbody id="<?php echo $prefix ?>_additional_get_body" class="sq-addtional-get-vars-list">
							<?php
							foreach ($current_value['additional_get_vars'] as $key => $var_info) {
								?>
								<tr id="<?php echo $prefix; ?>_additional_get_row_<?php echo $key; ?>">
									<td><?php
										if (!$read_only) {
											text_box($prefix.'_additional_get['.$key.'][name]', $var_info['name'], 20);
										} else {
											echo $var_info['name'];
										}
										?>
									</td>

									<td><?php
										if (!$read_only) {
											text_box($prefix.'_additional_get['.$key.'][value]', $var_info['value'], 30);
										} else {
											echo $var_info['value'];
										}
										?>
									</td>
									<?php
										if (!$read_only) {
									?>
										<td style="text-align:center"><a href="" onclick="<?php echo $prefix; ?>_object.removeGetRow('<?php echo $prefix; ?>','<?php echo $key; ?>'); return false;"><?php echo $delete_icon; ?></a></td>
									<?php
										}
									?>
								</tr>
								<?php
							}
							?>
							</tbody>
						</table>
						<?php
						if (!$read_only) {
							normal_button($prefix.'_additional_get_new_row', translate('Add Variable'), $prefix.'_object.addGetRow("'.$prefix.'"); return false;');
						}
						?>
					</td>
				</tr>
				<?php
			}//end if

			if (!$read_only) {
				?>
				<tr>
					<th><?php echo translate('Delete'); ?></th>

					<td>
						<?php
							check_box($prefix.'_delete', TRUE, FALSE);
							label(translate('Delete this file attachment configuration'), $prefix.'_delete');
						?>
					</td>
				</tr>
				<?php
			}
			?>
			</table>
		<?php

		}//end forach

		if (!empty($current_values)) {
			?>
			<div class="sq-backend-smallprint" style="margin-bottom: 9px"><?php echo translate('If selecting a non-file based asset to send, the extension used in the File Name will determine what type of file gets attached to the email. Example: If you select a Standard Page as the asset to send and enter "index.html" as the File Name, it will get attached as HTML file. If no File Name or extension is provided, the asset will get sent as a text file by default using the asset name as a File Name. File Name, Design To Apply and Paint Layout To Apply Settings are only applicable for non-file asset types. Variables set in the "Additional GET Parameters" setting will be passed to the asset specified in the "Asset To Send" field.'); ?></div>
			<?php
		}

		if (!$read_only) {
			echo '<div>';
			check_box($main_prefix.'_add_new', TRUE, FALSE);
			label(translate('Add new file attachment'), $main_prefix.'_add_new');

			hidden_field($main_prefix.'_current_values_count', count($current_values));

			echo '<div class="sq-backend-smallprint">';
			echo translate('Add a new custom file attachment to send as part of this email.');
			echo '</div></div>';
		}

	}//end paint()


	/**
	* Process the interface for filling in a value
	*
	* @param string	$prefix	prefix for the form elements
	*
	* @return void
	* @access public
	*/
	function process($prefix)
	{
		$main_prefix = str_replace(':', '_', $prefix);

		$current_values_count = isset($_REQUEST[$prefix.'_current_values_count']) ? $_REQUEST[$prefix.'_current_values_count'] : 0;

		$values = Array();
		for($index = 0; $index < $current_values_count; $index++) {
			$value = Array();
			$prefix = $main_prefix.'_'.$index;

			if (isset($_REQUEST[$prefix.'_delete'])) {
				continue;
			}

			// Process text and assetid fields
			$value['asset_to_send'] = (isset($_REQUEST[$prefix.'_asset_to_send']['assetid'])) ? $_REQUEST[$prefix.'_asset_to_send']['assetid'] : '0';
			$value['filename'] = (isset($_REQUEST[$prefix.'_filename'])) ? $_REQUEST[$prefix.'_filename'] : '';
			$value['design_to_apply'] = (isset($_REQUEST[$prefix.'_design_to_apply']['assetid'])) ? $_REQUEST[$prefix.'_design_to_apply']['assetid'] : '0';
			$value['layout_to_apply'] = (isset($_REQUEST[$prefix.'_layout_to_apply']['assetid'])) ? $_REQUEST[$prefix.'_layout_to_apply']['assetid'] : '0';

			// Process dynamic parameters
			$dynamic_parameters_attr = new Asset_Attribute_Parameter_Map();
			$dynamic_parameters_attr->process($prefix);
			$value['dynamic_parameters'] = $dynamic_parameters_attr->value;

			// Process additional get vars
			if (!empty($_REQUEST[$prefix.'_additional_get'])) {
				foreach($_REQUEST[$prefix.'_additional_get'] as $ag_key => $ag_value) {
					$empty_get_vars = TRUE;
					if (!empty($ag_value['name'])) {
						$value['additional_get_vars'][] = Array(
																'name' => $ag_value['name'],
																'value' => array_get_index($ag_value, 'value', ''),
															);
						$empty_get_vars = FALSE;
					}
					if ($empty_get_vars) {
						$value['additional_get_vars'] = Array();
					}
				}//end forach

			} else {
				 $value['additional_get_vars'] = Array();
			}

			$values[] = $value;
		}//end foreach

		if (isset($_REQUEST[$main_prefix.'_add_new'])) {
			// Add new empty file configuration option
			$values[] = Array();
		}

		$this->processed = $this->setValue($values);

	}//end process()


	/**
	* Generate the dynamic files in the data temp dir and return the generated files path
	*
	* @param array $additional_replacements	Additional keyword replacements to use in filename, dynamic parameters and additional GET vars
	*
	* @return array
	* @access public
	*/
	function generateFiles($additional_replacements=Array())
	{
		// Clear the previously generated files by this instance, if any
		$this->cleanUpGeneratedFiles();

		$create_location = SQ_TEMP_PATH.'/df_'.uniqid('', TRUE);
		if (!create_directory($create_location)) {
			trigger_error(sprintf(translate('Error occured when generating dynamic file(s). Cannot create the directory "%1$s".'), $create_location), E_USER_WARNING);
			return Array();
		}
		$this->_create_location = $create_location;

		$current_values = @unserialize($this->value);
		if (empty($current_values)) {
			return Array();
		}

		foreach($current_values as $current_value) {
			// Get the source asset
			$source_assetid = FALSE;
			if (isset($current_value['dynamic_parameters'])) {
				// Check dynamic parameter first for source asset to generate file from
				$dynamic_parameters_attr = new Asset_Attribute_Parameter_Map(0, $current_value['dynamic_parameters']);
				$source_assetid = $dynamic_parameters_attr->getParameterValue('asset_to_send', $additional_replacements);
				if (empty($source_assetid) || !$GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($source_assetid))) {
					// Fall back to the static 'Asset To Send'
					$source_assetid = $current_value['asset_to_send'];
					if (empty($source_assetid) || !$GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($source_assetid))) {
						$source_assetid = FALSE;
					}//end if
				}//end if
			}//end if
			// We dont have source assetid
			if (!$source_assetid) {
				continue;
			}
			$source_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($source_assetid);

			$filename = $current_value['filename'];
			// Replace any keywords in filename
			if (!empty($filename)) {
				replace_global_keywords($filename);
				$keywords = extract_keywords($filename, '', TRUE);
				if (!empty($keywords)) {
					$replacements = $additional_replacements;
					foreach($keywords as $keyword) {
						if (isset($replacements[$keyword])) {
							continue;
						}
						$replacement = $source_asset->getKeywordReplacement($keyword);
						if ($replacement == '%'.$keyword.'%') {
							// Blank out the unreplace keywords
							$replacements[$keyword] = '';
						} else {
							$replacements[$keyword] = $replacement;
						}
					}//end foreach
					replace_keywords($filename, $replacements);
				}//end if
			}

			// If we dont have filename, use the source asset name
			if (empty($filename)) {
				$filename = $source_asset->name;
			}

			// We will allow only these chars in the filename
			$filename = str_replace(' ', '_', $filename);
			$filename = preg_replace("/[^\w\d\.\-_~,;'\[\]\(\]]/", '', $filename);

			// If filename does not have extension, use the 'txt'
			if (strpos($filename, '.') === FALSE) {
				$filename .= '.txt';
			}

			// Make sure the filepath is not outside the data dir
			$file_path = $create_location.'/'.$filename;
			if (strpos($file_path, SQ_TEMP_PATH.'/') === FALSE) {
				// Though this should never happen after above filename chars whitelist filter
				trigger_error(sprintf(translate('Cannot create the dynamic file "%1$s" outside the data temp directory.'), $file_path), E_USER_WARNING);
				continue;
			}

			// If the asset type is file, we have already the file content ready
			$file_types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('file', TRUE);
			$asset_type = $source_asset->type();
			if (in_array($asset_type, $file_types)) {
				$file_info = $source_asset->getExistingFile();
				if (empty($file_info['path']) || !is_file($file_info['path'])) {
					trigger_error(sprintf(translate('The source file asset "%1$s" does not have its file in the filesystem.'), $source_asset->id), E_USER_WARNING);
					continue;
				}
				if (!copy($file_info['path'], $file_path)) {
					trigger_error(sprintf(translate('Cannot copy the source file "%1$s" to the temp dir.'), $file_info['path']), E_USER_WARNING);
					continue;
				}
			} else {
				// The output of the asset is our file content
				$design_id = array_get_index($current_value, 'design_to_apply', '');
				$layout_id = array_get_index($current_value, 'layout_to_apply', '');
				$get_vars = array_get_index($current_value, 'additional_get_vars', Array());
				$asset_content = $this->_getAssetContent($source_asset->id, $design_id, $layout_id, $get_vars, $additional_replacements);

				if (file_put_contents($file_path, $asset_content) === FALSE) {
					trigger_error(translate('Cannot create dynamic file in the temp dir.'), E_USER_WARNING);
					continue;
				}
			}

			// Add the file just created to the list
			$this->_generated_files[] = $file_path;

		}//end foreach

		return $this->_generated_files;

	}//end generateFiles()


	/**
	* Returns the file path of the generated files
	*
	* @return array
	* @access public
	*/
	function getGeneratedFiles()
	{
		return $this->_generated_files;

	}//end getGeneratedFiles()


	/**
	* Remove the generated files from the filesystem
	* This function should be called once the generated files are not required anymore
	*
	* @return void
	* @access public
	*/
	function cleanUpGeneratedFiles()
	{
		if (!empty($this->_create_location) && is_dir($this->_create_location)) {
			if (strpos($this->_create_location, SQ_TEMP_PATH.'/df_') !== FALSE) {
				delete_directory($this->_create_location);
			} else {
				trigger_error(sprintf(translate('Cannot clean the dyamic files dir. The directory "%1$s" is outside the data temp directory.'), $this->_create_location), E_USER_WARNING);
			}
		}

		$this->_create_location = '';
		$this->_generated_files = Array();

	}//end cleanUpGeneratedFiles()


	/**
	* Get the keywords in the attribute settings "Filename", "Dynamic Parameter" and "Additional GET var" settings
	*
	* @param array $additional_replacements	Additional keyword replacements to use in filename, dynamic parameters and additional GET vars
	*
	* @return array
	* @access public
	*/
	function getKeywords($additional_replacements=Array())
	{
		return extract_keywords($this->value);

	}//end getKeywords()


	/**
	* Ensure the attribute being set is an array
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	function validateValue(&$value)
	{
		// cant serilise scalar variables
		if (is_scalar($value)) {
			// see if it is already serilised
			$unser = @unserialize(trim((string) $value));
			if (is_scalar($unser)) {
				return FALSE;
			} else {
				$value = $unser;
			}
		}

		if (!is_array($value)) return FALSE;

		if (empty($value)) {
			$value = Array();
		}

		$value = serialize($value);

		return TRUE;

	}//end validateValue()


	/**
	* Get the asset output
	*
	* @param string $assetid
	* @param string $design_id
	* @param string $layout_id
	* @param array  $get_vars
	* @param array  $additional_replacements
	*
	* @return void
	* @access public
	*/
	function _getAssetContent($asset_id, $design_id, $layout_id, $get_vars, $additional_replacements=Array())
	{
		$details = Array(
					'content_id'        => $asset_id,
					'content_design'    => $design_id,
					'content_layout'    => $layout_id,
				);

		// Change the current global asset
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset_id);
		$old_current_asset = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET', NULL);
		$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $asset);

		// Include additional vars if any
		if (!empty($get_vars)) {
			$old_get = $_GET;
			$old_request = $_REQUEST;
			foreach($get_vars as $get_value) {
				// Replace global keywords in get var value
				replace_global_keywords($get_value['value']);
				$_GET[$get_value['name']] = $get_value['value'];
				$_REQUEST[$get_value['name']] = $get_value['value'];
			}
		}

		$output = $GLOBALS['SQ_SYSTEM']->am->generateRawContent($details);
		if (!empty($additional_replacements)) {
			replace_keywords($output, $additional_replacements);
		}

		// Perform the highest level keyword replacement on the asset output
		$GLOBALS['SQ_SYSTEM']->replaceKeyword($output);

		// Rstore global current asset and frontend asset
		if (is_null($old_current_asset)) {
			$GLOBALS['SQ_SYSTEM']->unsetGlobalDefine('CURRENT_ASSET');
		} else {
			$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $old_current_asset);
		}

		// Restore global _GET/_REQUEST vars
		if (!empty($get_vars)) {
			$_GET = $old_get;
			$_REQUEST = $old_request;
		}

		return $output;

	}//end _getAssetContent()


}//end class
?>
