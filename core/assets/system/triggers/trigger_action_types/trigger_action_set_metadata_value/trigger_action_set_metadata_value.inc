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
* $Id: trigger_action_set_metadata_value.inc,v 1.43 2013/01/29 07:29:49 cupreti Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/parameter_map/parameter_map.inc';

/**
* Trigger_Action_Set_Metadata_Value
*
* A trigger action that applies or denies a metadata value
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.43 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Metadata_Value extends Trigger_Action
{


	/**
	* Execute this action
	*
	* Returns an array of data about what it did, or false on error
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*       'fieldid'							=> int    // metadata_field id
	*       'field_name'						=> string // name of the field
	*       'value'								=> string // the value to set
	*       'replace_keywords_in_set_value'		=> boolean (0|1), // the flag to indicate if this trigger action will replace the keywords in the set value with the broadcasting asset is the current asset in the replacement context
	*       );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		// check required settings
		if (empty($settings['fieldid'])) return FALSE;
		// do not return false already from here incase we are using parameter map for gettign the value

		// If no context ID is set at ALL, then presume it's an old trigger, and
		// that we use the current context instead.
		if (array_key_exists('contextid', $settings) === FALSE) {
			$settings['contextid'] = NULL;
		}

		if ($settings['contextid'] === NULL) {
			$settings['contextid'] = $GLOBALS['SQ_SYSTEM']->getContextId();
		}

		$all_contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
		if (array_key_exists($settings['contextid'], $all_contexts) === FALSE) {
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->changeContext($settings['contextid']);

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				$GLOBALS['SQ_SYSTEM']->restoreContext();
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		// get the asset
		$asset = $state['asset'];

		// verify the fieldid
		$success = $mm->isMetadataFieldAssetid($settings['fieldid']);
		if (!$success) {
			$GLOBALS['SQ_SYSTEM']->restoreContext();
			return FALSE;
		}

		$old_current_asset = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET', NULL);
		$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $asset);

		$parameter_map_value = array_get_index($settings, 'value_map', serialize(Array()));
		$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
		$param_map_value = $atr_parameter_map->getParameterValue('metadata_value');

		// settings contains the 'fallback' static value.
		if (!empty($param_map_value)) {
			$value = $param_map_value;
			// Fix for #3803 set metadata trigger not escaping "=" correctly. Moved following
			// code from outside the IF block into. We will not need to escaps the "=" if we
			// fetching value from DB. "=" are already escapped in there.
			// use this only for param map values.
			//
			// Sanitise the value for equal signs (=)
			// As per Bug #2951 HTML Submitted becomes malformed due to a simple equals sign
			require_once SQ_CORE_PACKAGE_PATH.'/metadata/metadata_field/metadata_field.inc';
			$value = Metadata_Field::encodeValueString($value, Array());
		} else {
			if (!array_key_exists('value', $settings)) {
				return FALSE;
			}
			$value = $settings['value'];
		}

		// get the current metadata values
		$metadata = $mm->getMetadata($asset->id);

		// get the field name
		$field_name = '';
		if (isset($metadata[$settings['fieldid']])) {
			$field_name = $metadata[$settings['fieldid']][0]['name'];
		}

		//if the replace keywords checkbox is checked, do keyword replacement here before setting the value to metadata field
		if (array_get_index($settings, 'replace_keywords_in_set_value', 0)) {
			// replace global keywords
			replace_global_keywords($value);

			// replace broadcasting asset keywords
			$keywords = retrieve_keywords_replacements($value);
			$replacements = Array();
			foreach ($keywords as $keyword) {
				$replacements[$keyword] = $asset->getKeywordReplacement($keyword);
			}
			replace_keywords($value, $replacements);
		}

		// set the new value
		$metadata = Array();
		$metadata[$settings['fieldid']] = Array(
											0	=> Array (
													'name'	=> $field_name,
													'value'	=> $value,
												   ),
										  );
		$success = $mm->setMetadata($asset->id, $metadata);

		// Regenerate Metadata File
		if ($success) {
			$field = $GLOBALS['SQ_SYSTEM']->am->getAsset($settings['fieldid']);
			// If the set field is contextable and is not a select field, regenerate the metadata file in the current context only;
			// otherwise, regenerate all metadata files
			if ($field->attr('is_contextable') && !($field instanceof Metadata_Field_Select)) {
				$mm->generateContentFile($asset->id);
			} else {
				$all_contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
				foreach ($all_contexts as $contextid => $context_data) {
					$mm->generateContentFile($asset->id, FALSE, $contextid);
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->restoreContext();

		if (is_null($old_current_asset)) {
			$GLOBALS['SQ_SYSTEM']->unsetGlobalDefine('CURRENT_ASSET');
		} else {
			$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $old_current_asset);
		}

		// all done very smoothly like an ice cream
		if ($success) {
			return Array(
					'fieldid'		=> $settings['fieldid'],
					'field_name'	=> $field_name,
					'value'			=> $value,
					'contextid'		=> $settings['contextid'],
				   );
		} else {
			return FALSE;
		}

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	*
	* @return boolean
	* @access public
	*/
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		$output					= '';
		$metadata_fieldid		= array_get_index($settings, 'fieldid', 0);
		$metadata_field_value	= array_get_index($settings, 'value', NULL);
		$metadata_field         = NULL;

		if ($metadata_fieldid) {
			$metadata_field = $GLOBALS['SQ_SYSTEM']->am->getAsset($metadata_fieldid);
		}
		// asset finder for Metadata Field
		ob_start();
			echo '<div class="sq-break-line-inputs">';
			if ($write_access) {
				echo 'Choose metadata field: ';
				asset_finder($prefix.'[fieldid]', $metadata_fieldid, Array('metadata_field' => 'D'));
			} else {
				echo 'Metadata field: ';
				if ($metadata_fieldid  && $metadata_field) {
					echo get_asset_tag_line($metadata_fieldid);
				} else if ($metadata_fieldid  && !$metadata_field) {
					echo '<span class="sq-backend-warning">Unknown asset (Id: #'.$metadata_fieldid.')</span>';
				} else {
					echo '<em>No metadata field specified</em>';
				}
			}
			echo '</div>';
			$output_part1 = ob_get_contents();
		ob_end_clean();
		$output .= $output_part1;

		// Input field for the value
		ob_start();
			if ($write_access && $metadata_field) {
				echo '<div class="sq-break-line-inputs">';
				// attribute friendly prefix
				$new_prefix = str_replace(array('[',']'), '_', $prefix);
				echo 'Set the value for this field: ';
				// set the field name
				$edit_fns = $metadata_field->getEditFns();
				$edit_fns->paintValueInterface($metadata_field, $GLOBALS['SQ_SYSTEM']->backend->out, $metadata_field_value, TRUE, TRUE, FALSE, NULL, $new_prefix);
				hidden_field($prefix.'[form_printed]', 1);

				hidden_field($prefix.'[new_prefix]', $new_prefix);

				$parameter_map_value = array_get_index($settings, 'value_map', serialize(Array()));
				$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);

				$atr_parameter_map->setParameter('metadata_value', 'Metadata Field Value');
				echo $atr_parameter_map->paint($new_prefix.'_parameter_map');
				echo '</div>';
				echo '<br/>';
			}
			$output_part2 = ob_get_contents();
		ob_end_clean();
		$output .= $output_part2;

		// Replace keywords checkbox
		ob_start();
			echo '<div class="sq-break-line-inputs">';
			check_box($prefix.'[replace_keywords_in_set_value]', '1', array_get_index($settings, 'replace_keywords_in_set_value', 0), NULL, ($write_access? '' : 'disabled="disabled"'));
			label('Replace keywords in the set value', $prefix.'[replace_keywords_in_set_value]');
			echo '</div>';
		$output_part3 = ob_get_clean();

		$output .= $output_part3;

		ob_start();
			echo '<div class="sq-break-line-inputs">';
			$current_context = array_get_index($settings, 'contextid', NULL);

			$contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
			$context_list = Array(
								'-default-' => translate('Use currently active context at time of firing'),

								'-sep-'		=> '----------------------',
							);
			foreach ($contexts as $context_item_id => $context_item) {
				if ((int)$current_context === (int)$context_item_id) {
					$context_name = $context_item['name'];
				}

				$context_list[$context_item_id] = $context_item['name'];
			}

			$default = Array('-default-');
			if (array_key_exists($current_context, $contexts) === TRUE) {
				$default = Array($current_context);
			}

			if ($metadata_field) {
				label(translate('Set value in this context:'), $prefix.'[context_to_select]');

				echo ' ';
				if ($write_access) {
					combo_box($prefix.'[context_to_select]', $context_list, FALSE, $default);
				} else {
					if ($current_context === NULL) {
						echo '<strong>';
						echo translate('Use currently active context at time of firing');

						echo '</strong>';
					} else if (array_key_exists($current_context, $contexts) === TRUE) {
						echo '<strong>';
						echo $contexts[$current_context]['name'];
						echo '</strong>';
					} else {
						echo '<span class="sq-backend-warning">'.translate('Set context has been deleted. Trigger will fail until the context is changed to an existing one.').'</span>';

					}
				}
			}
			echo '</div>';
		$output_context_select = ob_get_clean();
		$output .= $output_context_select;

		return $output;

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as
	*								specified by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		// process fieldid
		$fieldid_data = array_get_index($request_data, 'fieldid', 0);
		$form_printed = array_get_index($request_data, 'form_printed', 0);
		$prefix		  = array_get_index($request_data, 'new_prefix', '');

		$fieldid = $fieldid_data['assetid'];

		if (!$GLOBALS['SQ_SYSTEM']->am->assetExists($fieldid)) {
			return translate('Selected asset does not exist');

		}

		if ($fieldid) {
			$metadata_field = $GLOBALS['SQ_SYSTEM']->am->getAsset($fieldid);

			// Bug fix #4014 Set Metadata Value trigger action saves asset ID that is not Metadata Field ID
			// Once we have the field check if the asset is a metadata field
			if (!($metadata_field instanceof Metadata_Field)) {
				return 'The input ID is not a Metadata Field ID';
			}

			// Can we edit the destination field? If we can't then what's the point;
			// we can't even change it through a trigger
			if (!$metadata_field->attr('editable')) {
				trigger_localised_error('CORE0246', sprintf(translate('Metadata field "%1$s" (Id: #%2$s) is marked as not editable; it cannot be set as the field to set'), $metadata_field->name, $metadata_field->id), E_USER_WARNING);
				return FALSE;
			}

			// got this far?? the field seems to be good for us
			$settings['fieldid'] = $fieldid;

			$edit_fns = $metadata_field->getEditFns();

			$result = '';
			if ($form_printed) {
				$field_cascade_value=FALSE;
				$edit_fns->processInlineValueInterface($metadata_field, $result, $field_cascade_value, NULL, $prefix);
			}

			$settings['field_name']	= $metadata_field->name;
			$settings['value']		= $result;

			$new_prefix = str_replace(array('[',']'), '_', $prefix);
			if ($new_prefix) {
				$atr_parameter_map = new Asset_Attribute_Parameter_Map();
				$atr_parameter_map->process($new_prefix.'_parameter_map');
				$settings['value_map'] = $atr_parameter_map->value;
			}

			$new_context = array_get_index($request_data, 'context_to_select', 0);
			if (is_numeric($new_context) === TRUE) {
				$new_context = (int)$new_context;
				$contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
				if (array_key_exists($new_context, $contexts) === TRUE) {
					$settings['contextid'] = $new_context;
				} else {
					trigger_localised_error('TRIG0005', sprintf(translate('Unable to select context #%1$s for use in trigger, the context no longer exists'), $new_context), E_USER_WARNING);
				}
			} else if ($new_context === '-default-') {
				$settings['contextid'] = NULL;
			}
		}

		$settings['replace_keywords_in_set_value']	= array_get_index($request_data, 'replace_keywords_in_set_value', 0);

		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs acquire before executing
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	public static function getLocks($settings, &$state)
	{
		return Array($state['assetid'] => Array('all'));

	}//end getLocks()


}//end class

?>
